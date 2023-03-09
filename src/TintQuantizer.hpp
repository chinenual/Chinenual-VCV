#pragma once

#include "PitchNote.hpp"

#define NUM_INPUT_ROWS 6
#define NUM_INPUT_COLS 2
#define NUM_INPUTS (NUM_INPUT_ROWS * NUM_INPUT_COLS)

namespace Chinenual {
namespace Tint {

    struct TintQuantizer {
        enum Mode {
            MODE_UP,
            MODE_DOWN,
            MODE_UP_DOWN,
            MODE_UP2,
            MODE_DOWN2,
            MODE_UP2_DOWN2,
            MODE_QUANTIZE
        };

        Mode mode;
        int octave;
        // current note is playing up or down when bidirectional
        bool upDown;
        // MAX_FLOAT for not in chord; otherwise the quantized v/oct value for the note
        float quantizedNoteVoltages[PITCH_NOTE_MAX - PITCH_NOTE_MIN];
        // actual voltage specified for the given entry in the reference chord
        float chordInputVoltageState[rack::PORT_MAX_CHANNELS];

        void reset()
        {
            mode = TintQuantizer::MODE_UP;
            octave = 0;
            upDown = false;
            for (int n = PITCH_NOTE_MIN; n <= PITCH_NOTE_MAX; n++) {
                quantizedNoteVoltages[n - PITCH_NOTE_MIN] = std::numeric_limits<float>::max();
            }
            // INFO("RESET!");
            for (int i = 0; i < rack::PORT_MAX_CHANNELS; i++) {
                chordInputVoltageState[i] = 0.f;
            }
        }

        /* note is MIDI note value */
        bool inChord(int note)
        {
            return chordFreq(note) < std::numeric_limits<float>::max();
        }
        /* note is MIDI note value, return value it's quantized frequency voltage if < maxfloat (maxfloat means not in a chord)
        Don't use the quantizedNoteVoltages array directly since it is not "zero based" based on MIDI note value */
        float chordFreq(int note)
        {
            return quantizedNoteVoltages[note - PITCH_NOTE_MIN];
        }

        float tintinnabulate(float v)
        {
            int note = voltageToPitch(v);

            int delta = 0; // up or down
            int count = 0; // (1 = first available note in the chord, 2 = second)
            switch (mode) {
            case MODE_UP:
                count = 1;
                delta = 1;
                break;
            case MODE_UP2:
                count = 2;
                delta = 1;
                break;
            case MODE_DOWN:
                count = 1;
                delta = -1;
                break;
            case MODE_DOWN2:
                count = 2;
                delta = -1;
                break;
            case MODE_UP_DOWN:
                count = 1;
                delta = upDown ? 1 : -1;
                break;
            case MODE_UP2_DOWN2:
                count = 2;
                delta = upDown ? 1 : -1;
                break;
            case MODE_QUANTIZE:
                // not really tintinnabulation - we just snap to the nearest note in the chord (nearest by frequency)
                float down_v = 0.f, up_v = 0.f; // voltages of the nearest note up or down
                for (int n = note; n <= PITCH_NOTE_MAX; n++) {
                    if (inChord(n)) {
                        up_v = chordFreq(n);
                        break;
                    }
                }
                for (int n = note - 1; n >= PITCH_NOTE_MIN; n--) {
                    if (inChord(n)) {
                        down_v = chordFreq(n);
                        break;
                    }
                }
                // choose the nearest one
                if ((up_v - v) < (v - down_v)) {
                    return up_v + octave; // octave can be used directly since V/oct is 1 volt per octave
                } else {
                    return down_v + octave; // octave can be used directly since V/oct is 1 volt per octave
                }
                break;
            }
            if (delta > 0) {
                // UP
                int c = 0;
                for (int n = note + 1; n <= PITCH_NOTE_MAX; n++) {
                    if (inChord(n)) {
                        c++;
                    }
                    if (c == count) {
                        return chordFreq(n) + octave; // octave can be used directly since V/oct is 1 volt per octave
                    }
                }
            } else {
                // DOWN
                int c = 0;
                for (int n = note - 1; n >= PITCH_NOTE_MIN; n--) {
                    if (inChord(n)) {
                        c++;
                    }
                    if (c == count) {
                        return chordFreq(n) + octave; // octave can be used directly since V/oct is 1 volt per octave
                    }
                }
            }
            // shouldnt happen, but return something reasonable
            return pitchToVoltage(note) + octave; // octave can be used directly since V/oct is 1 volt per octave
        }

        /* Given chordInputVoltageState[], setup the quantizedNoteVoltages[] array */
        void prepare(int noteCount)
        {

            for (int n = PITCH_NOTE_MIN; n <= PITCH_NOTE_MIN; n++) {
                quantizedNoteVoltages[n - PITCH_NOTE_MIN] = std::numeric_limits<float>::max();
            }
            for (int c = 0; c < noteCount; c++) {
                // we assume inputs are in +/-10V
                float v = chordInputVoltageState[c];
                float deviate = voltageToPitchDeviation(v);
                int chord_n = voltageToPitch(v);
                for (int n = PITCH_NOTE_MIN; n <= PITCH_NOTE_MAX; n++) {
                    if ((n % 12) == (chord_n % 12)) {
                        quantizedNoteVoltages[n - PITCH_NOTE_MIN] = microPitchToVoltage(deviate + n);
                    }
                }
            }
        }
    };

} // namespace Tint
} // namespace Chinenual