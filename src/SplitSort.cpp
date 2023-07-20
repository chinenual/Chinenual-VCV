#include <osdialog.h>

#include "plugin.hpp"
#include <array>

namespace Chinenual {
namespace SplitSort {

    struct SplitSort : Module {
        enum ParamId {
            SORT_PARAM,
            PARAMS_LEN
        };
        enum InputId {
            POLY_INPUT,
            LINK_INPUT,
            INPUTS_LEN
        };
        enum OutputId {
            ENUMS(SPLIT_OUTPUT, 16),
            LINK_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            SORT_LIGHT,
            LIGHTS_LEN
        };

        SplitSort()
        {
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort");
            configInput(LINK_INPUT, "Sorting Link");
            configOutput(LINK_OUTPUT, "Sorting Link");
            configInput(POLY_INPUT, "");
            for (int i = 0; i < 16; i++) {
                configOutput(SPLIT_OUTPUT + i, string::f("Channel %d", i + 1));
            }
            onReset();
        }

        void
        onReset() override
        {
            params[SORT_PARAM].setValue(0.f);
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_object_set_new(rootJ, "sort", json_boolean(params[SORT_PARAM].getValue()));
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;
            json_t* sortJ = json_object_get(rootJ, "sort");
            if (sortJ)
                params[SORT_PARAM].setValue(json_boolean_value(sortJ) ? 1.f : 0.f);
        }

        void process(const ProcessArgs& args) override
        {
            bool useLink = inputs[LINK_INPUT].isConnected();
            int numChannels = inputs[POLY_INPUT].getChannels();
            outputs[LINK_OUTPUT].setChannels(numChannels);

            if (params[SORT_PARAM].getValue()) {
                lights[SORT_LIGHT].setBrightness(1.0f);
                // sort on our own and populate the LINK array.  Put the voltages in a vector with the LINK
                // values and sort both together using the first element as the comparison value if sorting without
                // a link, or the second element if using the link as the sort element.
                std::array<std::array<float, 2>, 16> sorted;
                for (int ch = 0; ch < 16; ch++) {
                    sorted[ch][0] = inputs[POLY_INPUT].getVoltage(ch);
                    if (useLink) {
                        // will be 0.0f for unused channels
                        sorted[ch][1] = inputs[LINK_INPUT].getVoltage(ch);
                    } else {
                        sorted[ch][1] = sorted[ch][0];
                    }
                }
                std::sort(sorted.begin(), sorted.begin() + numChannels,
                    [useLink](const std::array<float, 2>& a, const std::array<float, 2>& b) {
                        if (useLink) {
                            if (a[1] == 0.0f) {
                                // 0.0f might have meant an unused channel on the link input.
                                // Treat it as "leave this position in place"
                                return false;
                            } else {
                                return a[1] < b[1];
                            }
                        } else {
                            return a[0] < b[0];
                        }
                    });
                for (int ch = 0; ch < 16; ch++) {
                    outputs[SPLIT_OUTPUT + ch].setVoltage(sorted[ch][0]);
                    if (useLink) {
                        outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(ch), ch);
                    } else {
                        outputs[LINK_OUTPUT].setVoltage(inputs[POLY_INPUT].getVoltage(ch), ch);
                    }
                }
            } else {
                // unsorted:
                lights[SORT_LIGHT].setBrightness(0.0f);
                for (int ch = 0; ch < 16; ch++) {
                    outputs[SPLIT_OUTPUT + ch].setVoltage(inputs[POLY_INPUT].getVoltage(ch));
                    outputs[LINK_OUTPUT].setVoltage(inputs[POLY_INPUT].getVoltage(ch), ch);
                }
            }
        };
    };

#define FIRST_X 10.0
#define FIRST_Y_A 17.0
#define FIRST_Y_B (FIRST_Y_A + SPACING_Y + (SPACING_Y / 3))
#define FIRST_Y_C (FIRST_Y_A + (9 * SPACING_Y) + (2 * SPACING_Y / 3))
#define SPACING_X 10.0
#define SPACING_Y 10.0
#define LED_OFFSET_X 3.0
#define LED_OFFSET_Y -9.5
#define BUTTON_OFFSET_X -1.0
#define BUTTON_OFFSET_Y -5.0

    struct SplitSortWidget : ModuleWidget {

        struct SortLight : GrayModuleLightWidget {
            SortLight()
            {
                addBaseColor(nvgRGB(0xff, 0x33, 0x33));
            }
        };
        SplitSortWidget(SplitSort* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/SplitSort.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X, FIRST_Y_A + 0 * SPACING_Y)), module,
                SplitSort::POLY_INPUT));

            addChild(createParamCentered<VCVBezelLatch>(mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_A + 0 * SPACING_Y)), module, SplitSort::SORT_PARAM));
            addChild(createLightCentered<LEDBezelLight<SortLight>>(mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_A + 0 * SPACING_Y)), module, SplitSort::SORT_LIGHT));

            for (int i = 0; i < 8; i++) {
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X, FIRST_Y_B + i * SPACING_Y)), module,
                    SplitSort::SPLIT_OUTPUT + i));
            }
            for (int i = 0; i < 8; i++) {
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_B + i * SPACING_Y)), module,
                    SplitSort::SPLIT_OUTPUT + 8 + i));
            }
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X, FIRST_Y_C)), module,
                SplitSort::LINK_INPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_C)), module,
                SplitSort::LINK_OUTPUT));
        }
    };

} // namespace SplitSort
} // namespace Chinenual

Model* modelSplitSort = createModel<Chinenual::SplitSort::SplitSort,
    Chinenual::SplitSort::SplitSortWidget>("SplitSort");
