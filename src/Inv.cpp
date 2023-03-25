#include <osdialog.h>

#include "PitchInverter.hpp"
#include "PitchNote.hpp"
#include "plugin.hpp"

namespace Chinenual {
namespace Inv {

    struct Inv : Module {
        enum ParamId {
            MODE_PARAM,
            PIVOT_PITCH_PARAM,
            PARAMS_LEN
        };
        enum InputId {
            SCALE_INPUT,
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
#if 0
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configInput(CHORD_INPUT, "Pitches to use for harmonization");
            configInput(PITCH_INPUT, "Melody pitch(es)");
            configInput(GATE_INPUT, "When to change direction of harmony (if bidirectional)");
            configOutput(Inv_OUTPUT, "Harmonized pitch(es)");
            configOutput(MIX_OUTPUT, "Original plus harmonized pitches");
            configSwitch(MODE_PARAM, 0, 6, 0, "Mode", { "Up", "Down", "Up/Down", "Up+1", "Down-1", "Up+1/Down-1", "Quantize" });
            configSwitch(OCTAVE_PARAM, -3, 3, 0, "Octave Offset", { "-3", "-2", "-1", "0", "+1", "+2", "+3" });
#endif
        }

        void onReset() override
        {
            inv.reset();
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_object_set_new(rootJ, "mode", json_integer(inv.mode));
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;

            json_t* modeJ = json_object_get(rootJ, "mode");
            if (modeJ) {
                inv.mode = (PitchInverter::Mode)json_integer_value(modeJ);
            }
        }

        void process(const ProcessArgs& args) override
        {
#if 0   
            // if (args.frame % 1) {
            inv.mode = (PitchInverter::Mode)(int)params[MODE_PARAM].getValue();
            inv.octave = (int)params[OCTAVE_PARAM].getValue();
            if (gateTrigger.process(inputs[GATE_INPUT].getVoltage(), 1.f, 2.f)) {
                // toggle the direction
                inv.upDown = !inv.upDown;
            }

            bool chordChange = false;
            for (int c = 0; c < inputs[CHORD_INPUT].getChannels(); c++) {
                // we assume inputs are in +/-10V
                float v = clamp(inputs[CHORD_INPUT].getPolyVoltage(c), PITCH_VOCT_MIN, PITCH_VOCT_MAX);
                if (v != inv.chordInputVoltageState[c]) {
                    chordChange = true;
                }
                inv.chordInputVoltageState[c] = v;
            }
            if (chordChange) {
                inv.prepare(inputs[CHORD_INPUT].getChannels());
            }
            int Inv_c = 0;
            int mix_c = 0;
            for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                // we assume inputs are in +/-10V
                float in_v = clamp(inputs[PITCH_INPUT].getPolyVoltage(c), PITCH_VOCT_MIN, PITCH_VOCT_MAX);
                float Inv_v = inv.Invinnabulate(in_v);
                outputs[Inv_OUTPUT].setVoltage(Inv_v, Inv_c);
                outputs[MIX_OUTPUT].setVoltage(in_v, mix_c);
                outputs[MIX_OUTPUT].setVoltage(Inv_v, mix_c + 1);
                Inv_c += 1;
                mix_c += 2;
            }
            outputs[Inv_OUTPUT].setChannels(Inv_c);
            outputs[MIX_OUTPUT].setChannels(mix_c);
            //}
#endif
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
#if 0
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 1)), module, Inv::MODE_PARAM));
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 2)), module, Inv::OCTAVE_PARAM));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 3)), module, Inv::CHORD_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 4)), module, Inv::PITCH_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 5)), module, Inv::GATE_INPUT));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 6)), module, Inv::Inv_OUTPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 7)), module, Inv::MIX_OUTPUT));
#endif
        }
    };

} // namespace Inv
} // namespace Chinenual

Model* modelInv = createModel<Chinenual::Inv::Inv,
    Chinenual::Inv::InvWidget>("Inv");
