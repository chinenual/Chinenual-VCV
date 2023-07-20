#include <osdialog.h>

#include "plugin.hpp"
#include <array>

namespace Chinenual {
namespace MergeSort {

    struct MergeSort : Module {
        enum ParamId {
            SORT_PARAM,
            PARAMS_LEN
        };
        enum InputId {
            ENUMS(SPLIT_INPUT, 16),
            LINK_INPUT,
            INPUTS_LEN
        };
        enum OutputId {
            POLY_OUTPUT,
            LINK_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            SORT_LIGHT,
            LIGHTS_LEN
        };

        MergeSort()
        {
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configParam(SORT_PARAM, 0.f, 1.f, 0.f, "Sort");
            configInput(LINK_INPUT, "Sorting Link");
            configOutput(LINK_OUTPUT, "Sorting Link");
            configOutput(POLY_OUTPUT, "");
            for (int ch = 0; ch < 16; ch++) {
                configInput(SPLIT_INPUT + ch, string::f("Channel %d", ch + 1));
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
            int numChannels = 0;
            for (int ch = 15; ch >= 0; ch--) {
                if (inputs[SPLIT_INPUT + ch].isConnected()) {
                    numChannels = ch + 1;
                    break;
                }
            }
            outputs[LINK_OUTPUT].setChannels(numChannels);
            outputs[POLY_OUTPUT].setChannels(numChannels);

            if (params[SORT_PARAM].getValue()) {
                lights[SORT_LIGHT].setBrightness(1.0f);
                // sort on our own and populate the LINK array.  Put the voltages in a vector with the LINK
                // values and sort both together using the first element as the comparison value if sorting without
                // a link, or the second element if using the link as the sort element.
                std::array<std::array<float, 2>, 16> sorted;
                for (int ch = 0; ch < 16; ch++) {
                    sorted[ch][0] = inputs[SPLIT_INPUT + ch].getVoltage();
                    if (useLink) {
                        // will be 0.0f for unused channels
                        sorted[ch][1] = inputs[LINK_INPUT].getVoltage(ch);
                    } else {
                        sorted[ch][1] = (ch + 1) * 0.1f;
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
                    outputs[POLY_OUTPUT].setVoltage(sorted[ch][0], ch);
                    outputs[LINK_OUTPUT].setVoltage(sorted[ch][1], ch);
                }
            } else {
                // unsorted:
                lights[SORT_LIGHT].setBrightness(0.0f);
                for (int ch = 0; ch < 16; ch++) {
                    outputs[POLY_OUTPUT].setVoltage(inputs[SPLIT_INPUT + ch].getVoltage(), ch);
                    outputs[LINK_OUTPUT].setVoltage(ch < numChannels ? 0.1f * ch : 0.f, ch);
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

    struct MergeSortWidget : ModuleWidget {

        struct SortLight : GrayModuleLightWidget {
            SortLight()
            {
                addBaseColor(nvgRGB(0xff, 0x33, 0x33));
            }
        };
        MergeSortWidget(MergeSort* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/MergeSort.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X, FIRST_Y_A + 0 * SPACING_Y)), module,
                MergeSort::POLY_OUTPUT));

            addChild(createParamCentered<VCVBezelLatch>(mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_A + 0 * SPACING_Y)), module, MergeSort::SORT_PARAM));
            addChild(createLightCentered<LEDBezelLight<SortLight>>(mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_A + 0 * SPACING_Y)), module, MergeSort::SORT_LIGHT));

            for (int ch = 0; ch < 8; ch++) {
                addInput(createInputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X, FIRST_Y_B + ch * SPACING_Y)), module,
                    MergeSort::SPLIT_INPUT + ch));
            }
            for (int ch = 0; ch < 8; ch++) {
                addInput(createInputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_B + ch * SPACING_Y)), module,
                    MergeSort::SPLIT_INPUT + 8 + ch));
            }
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X, FIRST_Y_C)), module,
                MergeSort::LINK_INPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_C)), module,
                MergeSort::LINK_OUTPUT));
        }
    };

} // namespace MergeSort
} // namespace Chinenual

Model* modelMergeSort = createModel<Chinenual::MergeSort::MergeSort,
    Chinenual::MergeSort::MergeSortWidget>("MergeSort");
