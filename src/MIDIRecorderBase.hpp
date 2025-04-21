#pragma once

#include "MidiMessage.h"
#include "plugin.hpp"

namespace Chinenual {
namespace MIDIRecorder {

    // the Chinenual logo color:
    static const NVGcolor textColor_logoyellow = nvgRGB(0xff, 0xd5, 0x56);

#define NUM_TRACKS 10

    struct MasterToExpanderMessage {
        bool isRecording;
    };

    struct ExpanderToMasterMessage {
        // current status of the inputs for each track (are any inputs connected?)
        bool active[NUM_TRACKS];

        // a vector of new midi messages since last flip per track
        std::vector<smf::MidiMessage> msgs[NUM_TRACKS];

        ExpanderToMasterMessage()
        {
            // preallocate the vectors so we're not triggering an alloc in the audio thread.
            // Technically, we only need room for 1 message per track since the CC's are rate limited to no
            // more than one message per frame (and in practice many fewer than that due to the
            // rateLimiter timer)  - and the master consumes the messages every frame even though the
            // expanders don't produce them every frame.
            //
            // But preallocate extras "just in case" the master doesn't consume the vectors as expected.
            // It's only few bytes of overhead.
            for (int t = 0; t < NUM_TRACKS; t++) {
                msgs[t].reserve(3);
            }
        }
    };

    // the grid of track inputs must be in one continuous sequence, starting with
    // FIRST_INPUT_ID, incrementing across columns, and then down tracks.
    template <int cols_per_track>
    struct MIDIRecorderBase : Module {
        int FIRST_INPUT_ID;
        static const int COLS_PER_TRACK = cols_per_track;

        dsp::Timer rateLimiterTimer;
        bool rateLimiterTriggered = false;

        void processRateLimiter(const ProcessArgs& args)
        {
            // Adapted from Rack's CV-MIDI module:
            //
            // MIDI baud rate is 31250 b/s, or 3125 B/s.
            // CC messages are 3 bytes, so we can send a maximum of 1041 CC messages
            // per second. Since multiple CCs can be generated, play it safe and limit
            // the CC rate to 200 Hz.
            const double rateLimiterPeriod = 1 / 200.f;
            rateLimiterTriggered = (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
            if (rateLimiterTriggered)
                rateLimiterTimer.time -= rateLimiterPeriod;
        }

        bool activeTrackCacheDirty;
        bool activeTrackCache[NUM_TRACKS];

        virtual bool trackIsActive(const int track)
        {
            if (activeTrackCacheDirty) {
                for (int t = 0; t < NUM_TRACKS; t++) {
                    const auto first = FIRST_INPUT_ID + t * COLS_PER_TRACK;
                    const auto last = first + COLS_PER_TRACK;
                    activeTrackCache[t] = false;
                    for (int i = first; i < last; i++) {
                        if (inputs[i].isConnected()) {
                            activeTrackCache[t] = true;
                            break;
                        }
                    }
                }
                activeTrackCacheDirty = false;
            }
            return activeTrackCache[track];
        }

        void onReset() override
        {
            Module::onReset();
            activeTrackCacheDirty = true;
        }

        void process(const ProcessArgs& args) override { processRateLimiter(args); }

        void onPortChange(const Module::PortChangeEvent& e) override
        {
            // flush the active track cache
            activeTrackCacheDirty = true;
        }

        void onExpanderChange(const Module::ExpanderChangeEvent& e) override
        {
            bool connected = false;
            if ((e.side && rightExpander.module) || ((!e.side) && leftExpander.module)) {
                connected = true;
            }
            INFO("[%s] %s Expander %sconnected", model->slug.c_str(), e.side ? "Right" : "Left", connected ? "" : "dis");
        }

        MIDIRecorderBase(const int first_input_id)
        {
            FIRST_INPUT_ID = first_input_id;
        }
    };

} // namespace MIDIRecorder
} // namespace Chinenual
