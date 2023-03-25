#include <osdialog.h>

#include "PitchInverter.hpp"
#include "PitchNote.hpp"
#include "plugin.hpp"

namespace Chinenual {
namespace Inv {

    struct Inv : Module {
        enum ParamId {
            PARAMS_LEN
        };
        enum InputId {
            PIVOT_PITCH_INPUT,
            PITCH_INPUT,
            INPUTS_LEN
        };
        enum OutputId {
            MIX_OUTPUT,
            INV_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            LIGHTS_LEN
        };

        PitchInverter inv;

        Inv()
        {
            onReset();
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configInput(PIVOT_PITCH_INPUT, "Inversion pivot pitch");
            configInput(PITCH_INPUT, "Melody pitch(es)");
            configOutput(INV_OUTPUT, "Inverted pitches");
            configOutput(MIX_OUTPUT, "Original plus harmonized pitches");
        }

        void onReset() override
        {
            inv.reset();
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;
        }

        void process(const ProcessArgs& args) override
        {
            float pivot_v = clamp(inputs[PIVOT_PITCH_INPUT].getVoltage(), PITCH_VOCT_MIN, PITCH_VOCT_MAX);
            int inv_c = 0;
            int mix_c = 0;
            for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                // we assume inputs are in +/-10V
                float in_v = clamp(inputs[PITCH_INPUT].getPolyVoltage(c), PITCH_VOCT_MIN, PITCH_VOCT_MAX);
                float inv_v = clamp(inv.invert(pivot_v, in_v), PITCH_VOCT_MIN, PITCH_VOCT_MAX);
                outputs[INV_OUTPUT].setVoltage(inv_v, inv_c);
                outputs[MIX_OUTPUT].setVoltage(in_v, mix_c);
                outputs[MIX_OUTPUT].setVoltage(inv_v, mix_c + 1);
                inv_c += 1;
                mix_c += 2;
            }
            outputs[INV_OUTPUT].setChannels(inv_c);
            outputs[MIX_OUTPUT].setChannels(mix_c);
        }
    };

#define FIRST_X -5.0
#define FIRST_Y 5.0
#define SPACING_X 20.0
#define SPACING_Y 15.0
#define FIRST_X_OUT -6.3
#define PAIR_SPACING 0.42
#define LABEL_OFFSET_X -19.0
#define LABEL_OFFSET_Y -12.0
#define LABEL_HEIGHT 22
#define LABEL_WIDTH 55

#define SPACING_X_OUT 14.1
#define LABEL_OFFSET_X_OUT (LABEL_OFFSET_X + 2.0)
#define LABEL_OFFSET_Y_OUT (LABEL_OFFSET_Y - 0.5) // leave space for the shading under the output jacks

    struct InvWidget : ModuleWidget {
        InvWidget(Inv* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/Inv.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 3)), module, Inv::PIVOT_PITCH_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 4)), module, Inv::PITCH_INPUT));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 6)), module, Inv::INV_OUTPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 7)), module, Inv::MIX_OUTPUT));
        }
    };

} // namespace Inv
} // namespace Chinenual

Model* modelInv = createModel<Chinenual::Inv::Inv,
    Chinenual::Inv::InvWidget>("Inv");
