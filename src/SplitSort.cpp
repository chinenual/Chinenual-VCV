#include <osdialog.h>

#include "plugin.hpp"

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
            int numChannels = inputs[POLY_INPUT].getChannels();
            outputs[LINK_OUTPUT].setChannels(numChannels);

            if (params[SORT_PARAM].getValue()) {
                lights[SORT_LIGHT].setBrightness(1.0f);
                if (inputs[LINK_INPUT].isConnected()) {
                    //  just use the order recorded in the LINK
                    for (int i = 0; i < 16; i++) {
                        // daisy chain the link input to the output:
                        outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(i), i);
                        // the link element represents this element's "original unsorted position" encoded as 0.1v, 0.2v, ...
                        int j = ((int)(inputs[LINK_INPUT].getVoltage(i) * 10.0f)) - 1;
                        if (j < 0) {
                            // link value was 0.f: i.e., unconnected.
                            outputs[SPLIT_OUTPUT + i].setVoltage(0.f);
                        } else {
                            if (j >= 0 && j < 16) {
                                // nop if out of bounds
                                outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT].getVoltage(j));
                            }
                        }
                    }
                } else {
                    // sort on our own and populate the LINK array.  Put the voltages in a vector with the LINK
                    // values and sort both together using the first element as the comparison value:
                    std::array<std::array<float, 2>, 16> sorted;
                    for (int i = 0; i < 16; i++) {
                        sorted[i][0] = inputs[POLY_INPUT].getVoltage(i);
                        sorted[i][1] = (i + 1) * 0.1f;
                    }
                    std::sort(sorted.begin(), sorted.begin() + numChannels,
                        [](const std::array<float, 2>& a, const std::array<float, 2>& b) {
                            return a[0] < b[0];
                        });
                    for (int i = 0; i < 16; i++) {
                        outputs[SPLIT_OUTPUT + i].setVoltage(sorted[i][0]);
                        outputs[LINK_OUTPUT].setVoltage(sorted[i][1], i);
                    }
                }
            } else {
                // unsorted:
                lights[SORT_LIGHT].setBrightness(0.0f);
                for (int i = 0; i < 16; i++) {
                    outputs[SPLIT_OUTPUT + i].setVoltage(inputs[POLY_INPUT].getVoltage(i));
                    outputs[LINK_OUTPUT].setVoltage(i < numChannels ? 0.1f * i : 0.f, i);
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
