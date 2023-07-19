#include <osdialog.h>

#include "plugin.hpp"
#include <array>

namespace Chinenual {
namespace PolySort {

    static const int NUM_INPUTS = 10;

    struct PolySort : Module {
        enum ParamId {
            ENUMS(LINK_PARAM, NUM_INPUTS),
            PARAMS_LEN
        };
        enum InputId {
            ENUMS(IN_INPUT, NUM_INPUTS),
            INPUTS_LEN
        };
        enum OutputId {
            ENUMS(OUT_OUTPUT, NUM_INPUTS),
            OUTPUTS_LEN
        };
        enum LightId {
            ENUMS(LINK_LIGHT, NUM_INPUTS),
            LIGHTS_LEN
        };

        PolySort()
        {
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            for (int i = 0; i < NUM_INPUTS; i++) {
                configParam(LINK_PARAM + i, 0.f, 1.f, 0.f, string::f("Use input %d's sort order", i));
                configInput(IN_INPUT + i, string::f("%d", i + 1));
                configOutput(OUT_OUTPUT + i, string::f("%d", i + 1));
            }
            onReset();
        }

        void
        onReset() override
        {
            for (int i = 0; i < NUM_INPUTS; i++) {
                params[LINK_PARAM + i].setValue(0.f);
            }
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_t* arrayJ = json_array();
            for (int i = 0; i < 8; i++) {
                json_array_append(arrayJ, json_boolean(0.f != params[LINK_PARAM + i].getValue()));
            }
            json_object_set_new(rootJ, "links", arrayJ);
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;
            json_t* arrayJ = json_object_get(rootJ, "links");
            if (arrayJ) {
                for (int i = 0; i < 8; i++) {
                    json_t* eleJ = json_array_get(arrayJ, i);
                    if (eleJ) {
                        bool val = json_boolean_value(eleJ);
                        params[LINK_PARAM + i].setValue(val);
                    }
                }
            }
        }

        void process(const ProcessArgs& args) override
        {
            typedef struct {
                float voltage;
                int order;
            } SortData;
            std::array<SortData, 16> sorted;
            for (int i = 0; i < NUM_INPUTS; i++) {
                // the first input has no "link" (the link[0] param is ignored)
                bool useLink = (i > 0 && params[LINK_PARAM + i].getValue());
                if (i > 0) {
                    lights[LINK_LIGHT + i].setBrightness(params[LINK_PARAM + i].getValue() ? 1.0f : 0.0f);
                }

                for (int ch = 0; ch < 16; ch++) {
                    sorted[ch].voltage = inputs[IN_INPUT + i].getVoltage(ch);
                    if (useLink) {
                        // reuse the sort order from the previous input
                    } else {
                        sorted[ch].order = ch;
                    }
                }
                outputs[OUT_OUTPUT + i].setChannels(inputs[IN_INPUT + i].getChannels());
                if (inputs[IN_INPUT + i].isConnected()) {
                    if (useLink) {
                        for (int ch = 0; ch < 16; ch++) {
                            int idx = sorted[ch].order;
                            outputs[OUT_OUTPUT + i].setVoltage(sorted[idx].voltage, ch);
                        }
                    } else {
                        std::sort(sorted.begin(), sorted.begin() + inputs[IN_INPUT + i].getChannels(),
                            [](const SortData& a, const SortData& b) {
                                return a.voltage < b.voltage;
                            });
                        for (int ch = 0; ch < 16; ch++) {
                            outputs[OUT_OUTPUT + i].setVoltage(sorted[ch].voltage, ch);
                        }
                    }
                }
            }
        }
    };

#define FIRST_X 5.0
#define FIRST_Y 17.0
#define SPACING_X 10.0
#define SPACING_Y 10.5
#define LED_OFFSET_X 3.0
#define LED_OFFSET_Y -9.5
#define BUTTON_OFFSET_X -1.0
#define BUTTON_OFFSET_Y -5.0

    struct PolySortWidget : ModuleWidget {

        struct SortLight : GrayModuleLightWidget {
            SortLight()
            {
                addBaseColor(nvgRGB(0xff, 0x33, 0x33));
            }
        };
        PolySortWidget(PolySort* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/PolySort.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            for (int i = 0; i < NUM_INPUTS; i++) {
                if (i > 0) {
                    addChild(createParamCentered<VCVBezelLatch>(mm2px(Vec(FIRST_X, FIRST_Y + i * SPACING_Y)), module, PolySort::LINK_PARAM + i));
                    addChild(createLightCentered<LEDBezelLight<SortLight>>(mm2px(Vec(FIRST_X, FIRST_Y + i * SPACING_Y)), module, PolySort::LINK_LIGHT + i));
                }
                addInput(createInputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X, FIRST_Y + i * SPACING_Y)), module,
                    PolySort::IN_INPUT + i));
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + 2 * SPACING_X, FIRST_Y + i * SPACING_Y)), module,
                    PolySort::OUT_OUTPUT + i));
            }
        }
    };

} // namespace PolySort
} // namespace Chinenual

Model* modelPolySort = createModel<Chinenual::PolySort::PolySort,
    Chinenual::PolySort::PolySortWidget>("PolySort");
