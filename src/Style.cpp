#include "Style.hpp"

namespace Chinenual {
namespace Style {

    void Style::initialize()
    {
        std::string defaultsDir = rack::asset::user("Chinenual/");
        if (!rack::system::isDirectory(defaultsDir)) {
            rack::system::createDirectory(defaultsDir);
        }
        std::string defaultsFile = rack::asset::user("Chinenual/default-style.json");
        json_error_t error;
        json_t* fd { nullptr };
        auto* fptr = std::fopen(defaultsFile.c_str(), "r");
        if (fptr) {
            fd = json_loadf(fptr, 0, &error);
            DEFER({ std::fclose(fptr); });
        }
        if (!fd) {
            setTextColor(RED);
        } else {
            {
                json_t* defj = json_object_get(fd, "textColor");
                int colId { -1 };
                if (defj) {
                    colId = json_integer_value(defj);
                }
                if (colId >= RED && colId <= WHITE) {
                    setTextColor((Color)colId);
                } else {
                    setTextColor(RED);
                }
            }
        }
        json_decref(fd);
    }

    void Style::updateJSON()
    {
        std::string defaultsDir = rack::asset::user("Chinenual/");
        if (!rack::system::isDirectory(defaultsDir)) {
            rack::system::createDirectory(defaultsDir);
        }
        std::string defaultsFile = rack::asset::user("Chinenual/default-style.json");

        json_t* rootJ = json_object();
        json_t* cJ = json_integer(getTextColor());
        json_object_set_new(rootJ, "textColor", cJ);
        FILE* f = std::fopen(defaultsFile.c_str(), "w");
        if (f) {
            json_dumpf(rootJ, f, JSON_INDENT(2));
            std::fclose(f);
        }
        json_decref(rootJ);
    }
    static Color textColor { RED };

    Color Style::getTextColor()
    {
        return textColor;
    }
    void Style::setTextColor(Color c)
    {
        if (c != textColor) {
            textColor = c;
            Style::updateJSON();
        }
    }

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