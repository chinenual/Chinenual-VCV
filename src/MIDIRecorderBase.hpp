#pragma once

#include "MidiMessage.h"
#include "plugin.hpp"

namespace Chinenual {
namespace MIDIRecorder {

    // the Chinenual logo color:
    static const NVGcolor textColor_logoyellow = nvgRGB(0xff, 0xd5, 0x56);
    // white:
    static const NVGcolor textColor_white = nvgRGB(0xff, 0xff, 0xff);
    // lighter red:
    static const NVGcolor textColor_red = nvgRGB(0xff, 0x33, 0x33);
    // light yellow (lighter variant of the logo color):
    static const NVGcolor textColor_yellow = nvgRGB(0xff, 0xdf, 0x80);

    static const NVGcolor ledTextColor = textColor_red;

#define NUM_TRACKS 10

    struct ExpanderToMasterMessage {
        // current status of the inputs for each track (are any inputs connected?)
        bool active[NUM_TRACKS];

        // a vector of new midi messages since last flip per track
        std::vector<smf::MidiMessage> msgs[NUM_TRACKS];
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
            // Adapted from Rack's Recorder module:
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

        bool active_track_dirty;
        bool active_track_cache[NUM_TRACKS];

        virtual bool trackIsActive(const int track)
        {
            return true;
            const auto first = FIRST_INPUT_ID + track * COLS_PER_TRACK;
            const auto last = first + COLS_PER_TRACK;
            for (int i = first; i <= last; i++) {
                if (inputs[i].isConnected()) {
                    return true;
                }
            }
            return false;
        }

        void process(const ProcessArgs& args) override { processRateLimiter(args); }

        void onPortChange(const Module::PortChangeEvent& e) override
        {
            // flush the active track cache
            active_track_dirty = true;
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
