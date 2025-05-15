#pragma once
#include <rack.hpp>

namespace Chinenual {
namespace Style {

    // Based loosely on the SurgeXT "XTStyle" components
    // Since we only need to support a simple set of properties much of the complexity of the
    // Surge approach is not needed.

    enum Color {
        RED,
        YELLOW,
        GREEN,
        AQUA,
        WHITE
    };
    const Color DEFAULT_COLOR = RED;
    const static std::vector<std::string> colorNames = {
        "Red",
        "Yellow",
        "Green",
        "Aqua",
        "White",
    };
    std::string colorName(Color c);
    NVGcolor getNVGColor(Color c);

#define CONFIG_STYLE(PARAM_NAME) \
    configParam(PARAM_NAME, 0.f, (float)Style::colorNames.size(), 0.0f, "Text Style")

#define STYLE_MENUS(PARAM_NAME)                                      \
    {                                                                \
        menu->addChild(new MenuSeparator);                           \
        menu->addChild(createIndexSubmenuItem(                       \
            "Text Color", Chinenual::Style::colorNames,              \
            [=]() { return module->params[PARAM_NAME].getValue(); }, \
            [=](int val) {                                           \
                module->params[PARAM_NAME].setValue(val);            \
            }));                                                     \
    }

}
}
