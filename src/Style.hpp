#pragma once
#include <rack.hpp>

namespace Chinenual {
namespace Style {

    // Based loosely on the SurgeXT "XTStyle" components
    // Since we only need to support a simple set of properties and only globally across all the
    // modules in the plugin, much of the complexity of the Surge approach is not needed.

    enum Color {
        RED,
        YELLOW,
        GREEN,
        AQUA,
        WHITE
    };
    const static std::vector<std::string> colorNames = {
        "Red",
        "Yellow",
        "Green",
        "Aqua",
        "White",
    };
    std::string colorName(Color c);
    NVGcolor getNVGColor(Color c);

    struct Style {
        static void initialize();

        static Color getTextColor();
        static void setTextColor(Color c);

    private:
        static void updateJSON();
    };

#define STYLE_MENUS()                                                                \
    {                                                                                \
        menu->addChild(new MenuSeparator);                                           \
        menu->addChild(createIndexSubmenuItem(                                       \
            "Text Color", Chinenual::Style::colorNames,                              \
            [=]() { return Chinenual::Style::Style::getTextColor(); },               \
            [=](int val) {                                                           \
                Chinenual::Style::Style::setTextColor((Chinenual::Style::Color)val); \
            }));                                                                     \
    }

}
}
