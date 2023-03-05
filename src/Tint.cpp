#include <osdialog.h>

#include "PitchNote.hpp"
#include "plugin.hpp"

#define NUM_INPUT_ROWS 6
#define NUM_INPUT_COLS 2
#define NUM_INPUTS (NUM_INPUT_ROWS * NUM_INPUT_COLS)

namespace Chinenual {
namespace Tint {

    struct Tint : Module {
        enum ParamId {
            MODE_PARAM,
            OCTAVE_PARAM,
            PARAMS_LEN
        };
        enum InputId {
            CHORD_INPUT,
            PITCH_INPUT,
            GATE_INPUT,
            INPUTS_LEN
        };
        enum OutputId {
            MIX_OUTPUT,
            TINT_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            LIGHTS_LEN
        };

        enum Mode {
            MODE_UP,
            MODE_DOWN,
            MODE_UP_DOWN,
            MODE_UP2,
            MODE_DOWN2,
            MODE_UP2_DOWN2
        };

        Mode mode;
        int octave;
        bool upDown; // current note is playing up or down when bidirectional
        int inChord[128]; // -1 not in chord;  >= 0 index of the reference chord note
        float chordState[rack::PORT_MAX_CHANNELS];
        float chordDeviation[rack::PORT_MAX_CHANNELS];
        dsp::SchmittTrigger gateTrigger;

        Tint()
        {
            onReset();

            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configInput(CHORD_INPUT, "Pitches to use for harmonization");
            configInput(PITCH_INPUT, "Melody pitch(es)");
            configInput(GATE_INPUT, "When to change direction of harmony (if bidirectional)");
            configOutput(TINT_OUTPUT, "Harmonized pitch(es)");
            configOutput(MIX_OUTPUT, "Original plus harmonized pitches");
            configSwitch(MODE_PARAM, 0, 5, 0, "Mode", { "Up", "Down", "Up/Down", "Up+1", "Down-1", "Up+1/Down-1" });
            configSwitch(OCTAVE_PARAM, -3, 3, 0, "Octave Offset", { "-3", "-2", "-1", "0", "+1", "+2", "+3" });
        }

        void onReset() override
        {
            mode = MODE_UP;
            octave = 0;
            upDown = false;
            gateTrigger.reset();
            for (int n = 0; n < 128; n++) {
                inChord[n] = -1;
            }
            INFO("RESET!");
            for (int i = 0; i < rack::PORT_MAX_CHANNELS; i++) {
                chordState[i] = 0.f;
                chordDeviation[i] = 0.f;
            }
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_object_set_new(rootJ, "octave", json_integer(octave));
            json_object_set_new(rootJ, "mode", json_integer(mode));
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;

            json_t* modeJ = json_object_get(rootJ, "mode");
            if (modeJ) {
                mode = (Mode)json_integer_value(modeJ);
            }
            json_t* octaveJ = json_object_get(rootJ, "octave");
            if (octaveJ) {
                octave = json_integer_value(octaveJ);
            }
        }

        float tintinnabulate(int note)
        {
            int delta = 0; // up or down
            int count = 0; // (1 = first available note in the chord, 2 = second)
            switch (mode) {
            case MODE_UP:
                count = 1;
                delta = 1;
                break;
            case MODE_UP2:
                count = 2;
                delta = 1;
                break;
            case MODE_DOWN:
                count = 1;
                delta = -1;
                break;
            case MODE_DOWN2:
                count = 2;
                delta = -1;
                break;
            case MODE_UP_DOWN:
                count = 1;
                delta = upDown ? 1 : -1;
                break;
            case MODE_UP2_DOWN2:
                count = 2;
                delta = upDown ? 1 : -1;
                break;
            }
            if (delta > 0) {
                // UP
                int c = 0;
                for (int n = note + 1; n < 128; n++) {
                    if (inChord[n] >= 0) {
                        c++;
                    }
                    if (c == count) {
                        INFO("n:%d inc:%d  dev:%f -> %f\n,", n, inChord[n], chordDeviation[inChord[n] - 1],
                            microPitchToVoltage(n + (octave * 12) + chordDeviation[inChord[n] - 1]));
                        return microPitchToVoltage(n + (octave * 12) + chordDeviation[inChord[n] - 1]);
                    }
                }
            } else {
                // DOWN
                int c = 0;
                for (int n = note - 1; n > 0; n--) {
                    if (inChord[n] >= 0) {
                        c++;
                    }
                    if (c == count) {
                        return microPitchToVoltage(n + (octave * 12) + chordDeviation[inChord[n] - 1]);
                    }
                }
            }
            // shouldnt happen, but return something reasonable
            return pitchToVoltage(note + (octave * 12));
        }

        void process(const ProcessArgs& args) override
        {
            // if (args.frame % 1) {
            mode = (Mode)(int)params[MODE_PARAM].getValue();
            octave = (int)params[OCTAVE_PARAM].getValue();
            if (gateTrigger.process(inputs[GATE_INPUT].getVoltage(), 1.f, 2.f)) {
                // toggle the direction
                upDown = !upDown;
            }

            bool chordChange = false;
            for (int c = 0; c < inputs[CHORD_INPUT].getChannels(); c++) {
                float v = inputs[CHORD_INPUT].getPolyVoltage(c);
                float deviate = voltageToPitchDeviation(v);
                if (v != chordState[c] || deviate != chordDeviation[c]) {
                    chordChange = true;
                }
                chordState[c] = v;
                chordDeviation[c] = deviate;
            }
            if (chordChange) {
                for (int n = 0; n < 128; n++) {
                    inChord[n] = -1;
                }
                for (int c = 0; c < inputs[CHORD_INPUT].getChannels(); c++) {
                    INFO("CHORD CHANGE [%d] %f -> %f\n", c, chordState[c], chordDeviation[c]);
                    int chord_n = voltageToPitch(inputs[CHORD_INPUT].getPolyVoltage(c));
                    for (int n = 0; n < 128; n++) {
                        if ((n % 12) == (chord_n % 12)) {
                            inChord[n] = c + 1;
                        }
                    }
                }
            }
            int tint_c = 0;
            int mix_c = 0;
            for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                float in_v = inputs[PITCH_INPUT].getPolyVoltage(c);
                int in = voltageToPitch(in_v);
                float tint_v = tintinnabulate(in);
                outputs[TINT_OUTPUT].setVoltage(tint_v, tint_c);
                outputs[MIX_OUTPUT].setVoltage(in_v, mix_c);
                outputs[MIX_OUTPUT].setVoltage(tint_v, mix_c + 1);
                tint_c += 1;
                mix_c += 2;
            }
            outputs[TINT_OUTPUT].setChannels(tint_c);
            outputs[MIX_OUTPUT].setChannels(mix_c);
            //}
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

    struct TintWidget : ModuleWidget {
        TintWidget(Tint* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/Tint.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 1)), module, Tint::MODE_PARAM));
            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 2)), module, Tint::OCTAVE_PARAM));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 3)), module, Tint::CHORD_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 4)), module, Tint::PITCH_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 5)), module, Tint::GATE_INPUT));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 6)), module, Tint::TINT_OUTPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 7)), module, Tint::MIX_OUTPUT));
        }
    };

} // namespace Tint
} // namespace Chinenual

Model* modelTint = createModel<Chinenual::Tint::Tint,
    Chinenual::Tint::TintWidget>("Tint");
