#include "Style.hpp"

namespace Chinenual {
namespace Style {

    std::string colorName(Color c)
    {
        return colorNames[(int)c];
    }

    NVGcolor getNVGColor(Color c)
    {
        switch (c) {
        case RED:
            return nvgRGB(0xff, 0x33, 0x33);
        case YELLOW:
            return nvgRGB(0xff, 0xd4, 0x56);
        case GREEN:
            return nvgRGB(114, 234, 101);
        case AQUA:
            return nvgRGB(19, 236, 196);
        case WHITE:
            return nvgRGB(235, 235, 235);
        }
        return nvgRGB(255, 0, 1);
    }

}
}