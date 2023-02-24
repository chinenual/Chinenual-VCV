#include <osdialog.h>

#include "plugin.hpp"

#define NUM_INPUT_ROWS 6
#define NUM_INPUT_COLS 2
#define NUM_INPUTS (NUM_INPUT_ROWS * NUM_INPUT_COLS)

namespace Chinenual {
namespace Tint {

    struct Tint : Module {
        enum ParamId {
            MODE_PARAM,
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
        bool upDown; // current note is playing up or down when bidirectional
        bool inChord[128];
        float chordState[rack::PORT_MAX_CHANNELS];
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
        }

        void onReset() override
        {
            mode = MODE_UP;
            upDown = false;
            gateTrigger.reset();
            for (int n = 0; n < 128; n++) {
                inChord[n] = false;
            }
            for (int i = 0; i < rack::PORT_MAX_CHANNELS; i++) {
                chordState[i] = 0.f;
            }
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

        float pitchToVoltage(int note)
        {
            // based on VCV CORE MIDI_CV:
            return (note - 60.f) / 12.f;
        }
        int voltageToPitch(float v)
        {
            // based on VCV CORE CV_MIDI:
            return (int)std::round(v * 12.f + 60.f);
        }

        int tintinabulate(int note)
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
                    if (inChord[n]) {
                        c++;
                    }
                    if (c == count) {
                        return n;
                    }
                }
            } else {
                // DOWN
                int c = 0;
                for (int n = note - 1; n > 0; n--) {
                    if (inChord[n]) {
                        c++;
                    }
                    if (c == count) {
                        return n;
                    }
                }
            }
            return note;
        }

        void process(const ProcessArgs& args) override
        {
            mode = (Mode)(int)params[MODE_PARAM].getValue();
            if (gateTrigger.process(inputs[GATE_INPUT].getVoltage())) {
                // toggle the direction
                upDown = !upDown;
            }
            bool chordChange = false;
            for (int c = 0; c < inputs[CHORD_INPUT].getChannels(); c++) {
                float v = inputs[CHORD_INPUT].getPolyVoltage(c);
                if (v != chordState[c]) {
                    chordChange = true;
                }
                chordState[c] = v;
            }
            if (chordChange) {
                for (int n = 0; n < 128; n++) {
                    inChord[n] = false;
                }
                for (int c = 0; c < inputs[CHORD_INPUT].getChannels(); c++) {
                    int chord_n = voltageToPitch(inputs[CHORD_INPUT].getPolyVoltage(c));
                    for (int n = 0; n < 128; n++) {
                        inChord[n] |= (n % 12) == (chord_n % 12);
                    }
                }
            }
            int tint_c = 0;
            int mix_c = 0;
            for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                int in = voltageToPitch(inputs[PITCH_INPUT].getPolyVoltage(c));
                int harm = tintinabulate(in);
                outputs[TINT_OUTPUT].setVoltage(pitchToVoltage(harm), tint_c);
                outputs[MIX_OUTPUT].setVoltage(pitchToVoltage(in), mix_c);
                outputs[MIX_OUTPUT].setVoltage(pitchToVoltage(harm), mix_c + 1);
                tint_c += 1;
                mix_c += 2;
            }
            outputs[TINT_OUTPUT].setChannels(tint_c);
            outputs[MIX_OUTPUT].setChannels(mix_c);
        }
    };

#define FIRST_X -14.0
#define FIRST_Y 20.0
#define SPACING_X 20.0
#define SPACING_Y 16.0
#define FIRST_X_OUT -8.0
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
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, SPACING_Y * 2)), module, Tint::CHORD_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, SPACING_Y * 3)), module, Tint::PITCH_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, SPACING_Y * 4)), module, Tint::GATE_INPUT));

            addParam(createParamCentered<RoundSmallBlackKnob>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, SPACING_Y * 5)), module, Tint::MODE_PARAM));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, SPACING_Y * 6)), module, Tint::TINT_OUTPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 1 * SPACING_X_OUT, SPACING_Y * 6)), module, Tint::MIX_OUTPUT));
        }
    };

} // namespace Tint
} // namespace Chinenual

Model* modelTint = createModel<Chinenual::Tint::Tint,
    Chinenual::Tint::TintWidget>("Tint");
