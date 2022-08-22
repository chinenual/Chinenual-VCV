#pragma once

#include "MIDIRecorderBase.hpp"
#include "MidiFile.h"
#include "MidiMessage.h"
#include "plugin.hpp"
#include <condition_variable>
#include <thread>

namespace Chinenual {
namespace MIDIRecorder {

    // a non-allocating double buffer to allow the audio thread to create new MIDI messages, but not trigger
    // allocations in the audio thread when pushing them onto a MIDIFile track's MidiEventList.
    //
    // Loosely based on the VCV Recorder module's worker thread design.

    struct MIDIBuffer {
        // BUFFER_LEN sized to be large enough that we don't encur thread sync too often, but not so
        // large that the worker is way out of sync with lastest events.
        static const int BUFFER_LEN = 1024;
        static const int NUM_BUFFERS = 3;

        // buffer indexes increment monotonically - we use modulo arithmetic to select one of the three
        // buffers.  This lets us easily determine if the worker is "behind" via simple subtraction
        int64_t bufferIndex = 0;
        int64_t workerBufferIndex = 0;

        bool running = false;
        std::thread workerThread;
        std::mutex workerMutex;
        std::condition_variable workerCv;
        std::mutex mainMutex;
        std::condition_variable mainCv;

        std::vector<smf::MidiEvent> buffers[NUM_BUFFERS][NUM_TRACKS];
        smf::MidiFile& midiFile;

        MIDIBuffer(smf::MidiFile& midiFile)
            : midiFile(midiFile)
        {
            // preallocate the buffers so the audio thread never triggers an allocation as it pushes
            // items to the buffer

            for (int i = 0; i < NUM_BUFFERS; i++) {
                for (int t = 0; t < NUM_TRACKS; t++) {
                    buffers[i][t].reserve(BUFFER_LEN);
                }
            }
        }

        ~MIDIBuffer()
        {
            stop();
        }

        // Called from the audio thread to record an event
        void appendEvent(const int track, smf::MidiEvent& event)
        {
            // wait for the worker to catch up if it has fallen behind:
            if (bufferIndex - workerBufferIndex >= NUM_BUFFERS) {
                std::unique_lock<std::mutex> lock(mainMutex);
                mainCv.wait(lock);
            }
            auto buffer = buffers[bufferIndex % NUM_BUFFERS];

            buffer[track].push_back(event);

            if (buffer[track].size() >= BUFFER_LEN) {
                // we advance to the next set of buffers when any track overflows its buffer

                // next buffer:
                bufferIndex++;
                // wake up the worker:
                workerCv.notify_one();
            }
        }

        void processEvents(std::vector<smf::MidiEvent> buffer[NUM_TRACKS])
        {
            for (int t = 0; t < NUM_TRACKS; t++) {
                // copy events out of the buffer into the midiFile eventLists:
#ifdef SDTDEBUG
                if (buffer[t].size() > 0) {
                    INFO("WORKER CONSUMING %lu events on track %d (buf %lld)", buffer[t].size(), t, workerBufferIndex);
                }
#endif
                for (size_t i = 0; i < buffer[t].size(); i++) {
#ifdef SDTDEBUG
                    // INFO("[%d] %d %d", t, buffer[t].at(i).track, buffer[t].at(i).tick);
#endif
                    midiFile.addEvent(t, buffer[t].at(i));
                }
                buffer[t].clear();
            }
            mainCv.notify_one();
        }

        void run()
        {
            std::unique_lock<std::mutex> lock(workerMutex);
            while (running) {
                // wait until the master thread tells us there's something to process:
                workerCv.wait(lock);
                for (; workerBufferIndex < bufferIndex; workerBufferIndex++) {
                    auto buffer = buffers[workerBufferIndex % NUM_BUFFERS];
                    processEvents(buffer);
                }
            }
            // we're not running any more, but there may be some pent up events we need to handle:
            for (int i = 0; i < NUM_BUFFERS; i++) {
                auto buffer = buffers[i];
                processEvents(buffer);
            }
            /// now we fall off the end of the thread
        }

        void start()
        {
            workerBufferIndex = 0;
            bufferIndex = 0;

            if (workerThread.joinable()) {
                return;
            }

            running = true;
            workerThread = std::thread([this] {
                run();
            });
        }

        void stop()
        {
            if (!workerThread.joinable()) {
                return;
            }

            running = false;
            workerCv.notify_all();
            if (workerThread.joinable()) {
                workerThread.join();
            }
        }
    };
};
}
