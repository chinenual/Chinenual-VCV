#pragma once

#include "PitchNote.hpp"

namespace Chinenual {
namespace Inv {

    struct PitchInverter {
        enum Mode {
            MODE_CHROMATIC,
            MODE_DIATONIC,
        };

        Mode mode;

        void reset()
        {
            mode = MODE_CHROMATIC;
        }

        float invert(float pivot, float v)
        {
            int note_pivot = voltageToPitch(pivot);
            int note_v = voltageToPitch(v);

            if (mode == MODE_CHROMATIC) {
                float offset = v - pivot;
                return pivot - offset;
            }
            // else return something "reasonable"
            return pitchToVoltage(note_v);
        }
    };

} // namespace Inv
} // namespace Chinenual