#include <osdialog.h>

#include "plugin.hpp"

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
            for (int i = 0; i < 16; i++) {
                configInput(SPLIT_INPUT + i, string::f("Channel %d", i + 1));
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
            int numChannels = 0;
            for (int i = 15; i >= 0; i--) {
                if (inputs[SPLIT_INPUT + i].isConnected()) {
                    numChannels = i + 1;
                    break;
                }
            }
            outputs[LINK_OUTPUT].setChannels(numChannels);

            if (params[SORT_PARAM].getValue()) {
                lights[SORT_LIGHT].setBrightness(1.0f);
                if (inputs[LINK_INPUT].isConnected()) {
                    // just use the order recorded in the LINK.  If there are less channels in the link than there are in this merge, then
                    // extra channels will just be appended unsorted
                    for (int i = 0; i < 16; i++) {
                        // daisy chain the link input to the output:
                        outputs[LINK_OUTPUT].setVoltage(inputs[LINK_INPUT].getVoltage(i), i);
                        // the link element represents this element's "original unsorted position" encoded as 0.1v, 0.2v, ...
                        int j = ((int)(inputs[LINK_INPUT].getVoltage(i) * 10.0f)) - 1;
                        if (j < 0) {
                            // link value was 0.f: i.e., unconnected.
                            outputs[POLY_OUTPUT].setVoltage(0.f, i);
                        } else {
                            if (j >= 0 && j < 16) {
                                // nop if out of bounds
                                outputs[POLY_OUTPUT].setVoltage(inputs[SPLIT_INPUT + i].getVoltage(j), i);
                            }
                        }
                    }
                } else {
                    // sort on our own and populate the LINK array.  Put the voltages in a vector with the LINK
                    // values and sort both together using the first element as the comparison value:
                    std::array<std::array<float, 2>, 16> sorted;
                    for (int i = 0; i < 16; i++) {
                        sorted[i][0] = inputs[SPLIT_INPUT + i].getVoltage();
                        sorted[i][1] = (i + 1) * 0.1f;
                    }
                    std::sort(sorted.begin(), sorted.begin() + numChannels,
                        [](const std::array<float, 2>& a, const std::array<float, 2>& b) {
                            return a[0] < b[0];
                        });
                    for (int i = 0; i < 16; i++) {
                        outputs[POLY_OUTPUT].setVoltage(sorted[i][0], i);
                        outputs[LINK_OUTPUT].setVoltage(sorted[i][1], i);
                    }
                }
            } else {
                // unsorted:
                lights[SORT_LIGHT].setBrightness(0.0f);
                for (int i = 0; i < 16; i++) {
                    outputs[POLY_OUTPUT].setVoltage(inputs[SPLIT_INPUT + i].getVoltage(i), i);
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

            for (int i = 0; i < 8; i++) {
                addInput(createInputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X, FIRST_Y_B + i * SPACING_Y)), module,
                    MergeSort::SPLIT_INPUT + i));
            }
            for (int i = 0; i < 8; i++) {
                addInput(createInputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y_B + i * SPACING_Y)), module,
                    MergeSort::SPLIT_INPUT + 8 + i));
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
