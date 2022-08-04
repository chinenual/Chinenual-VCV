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
  // current status of the inputs for each track (is anything connected?)
  bool active[NUM_TRACKS];
  // a vector of new midi messages per track
  std::vector<smf::MidiMessage> msgs[NUM_TRACKS];
};

struct MasterToExpanderMessage {};

struct MIDIRecorderBase : Module {
  dsp::Timer rateLimiterTimer;
  bool rateLimiterTriggered = false;

  void processRateLimiter(const ProcessArgs &args) {
    // Adapted from Rack's Recorder module:
    //
    // MIDI baud rate is 31250 b/s, or 3125 B/s.
    // CC messages are 3 bytes, so we can send a maximum of 1041 CC messages
    // per second. Since multiple CCs can be generated, play it safe and limit
    // the CC rate to 200 Hz.
    const double rateLimiterPeriod = 1 / 200.f;
    rateLimiterTriggered =
        (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
    if (rateLimiterTriggered) rateLimiterTimer.time -= rateLimiterPeriod;
  }

  void process(const ProcessArgs &args) override { processRateLimiter(args); }
};

}  // namespace MIDIRecorder
}  // namespace Chinenual