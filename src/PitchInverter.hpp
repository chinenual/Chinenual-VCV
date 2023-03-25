#pragma once

#include "PitchNote.hpp"

namespace Chinenual {
namespace Inv {

    struct PitchInverter {
        enum Mode {
            MODE_CHROMATIC,
            // no support for TONAL inversion (yet?).  Requires more thought on how to specify the pitch offset
            // to start the inversion
            // MODE_DIATONIC,
        };

        Mode mode;

        void reset()
        {
            mode = MODE_CHROMATIC;
        }

        /* All args in V/oct.  pivot: the note around which to reflect; v: the note to invert */
        float invert(float pivot_v, float v)
        {
            if (mode == MODE_CHROMATIC) {
                float offset = v - pivot_v;
                return pivot_v - offset;
            }
            // should never happen. return something "reasonable"
            return v;
        }
    };

} // namespace Inv
} // namespace Chinenual
