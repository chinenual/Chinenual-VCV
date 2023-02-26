#include <osdialog.h>

#include "plugin.hpp"

#define NUM_INPUT_ROWS 12
#define NUM_INPUT_COLS 1
#define NUM_INPUTS (NUM_INPUT_ROWS * NUM_INPUT_COLS)

namespace Chinenual {
namespace NoteMeter {

    struct NoteMeter : Module {
        enum ParamId {
            PARAMS_LEN
        };
        enum InputId {
            PITCH_INPUT_1,
            PITCH_INPUT_2,
            PITCH_INPUT_3,
            PITCH_INPUT_4,
            PITCH_INPUT_5,
            PITCH_INPUT_6,
            PITCH_INPUT_7,
            PITCH_INPUT_8,
            PITCH_INPUT_9,
            PITCH_INPUT_10,
            PITCH_INPUT_11,
            PITCH_INPUT_12,
            INPUTS_LEN
        };
        enum OutputId {
            OUTPUTS_LEN
        };
        enum LightId {
            LIGHTS_LEN
        };

        NoteMeter()
        {
            onReset();

            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            for (int i = PITCH_INPUT_1; i < PITCH_INPUT_1 + NUM_INPUTS; i++) {
                configInput(i, string::f("Pitch %d", i - PITCH_INPUT_1));
            }
        }

        std::string text[NUM_INPUTS];

        void onReset() override
        {
        }

        int voltageToPitch(float v)
        {
            // based on VCV CORE CV_MIDI:
            return (int)std::round(v * 12.f + 60.f);
        }

        float voltageToMicroPitch(float v)
        {
            // based on VCV CORE CV_MIDI:
            return (v * 12.f + 60.f);
        }

        void pitchToText(std::string& text, int note, float deviation)
        {
            const char* noteNames[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
            int nameIndex = note % 12;
            int octave = (note / 12) - 1; // simple division produces 60=C5; we want to display that as C4, so subtract 1
            auto absDeviation = std::abs(deviation);
            if (absDeviation >= 0.01f) {
                text = string::f("%s%d %s%dc", noteNames[nameIndex], octave,
                    (deviation > 0 ? "+" : "-"),
                    (int)(absDeviation * 100));
            } else {
                text = string::f("%s%d", noteNames[nameIndex], octave);
            }
        }

        void process(const ProcessArgs& args) override
        {
            if ((args.frame % 100) == 0) { // throttle
                for (int i = 0; i < NUM_INPUTS; i++) {
                    auto in = inputs[PITCH_INPUT_1 + i];

                    if (in.isConnected()) {
                        auto n = voltageToPitch(in.getVoltage());
                        auto fn = voltageToMicroPitch(in.getVoltage());
                        pitchToText(text[i], n, fn - ((float)n));
                    } else {
                        text[i] = "";
                    }
                }
            }
        }
    };

    static const NVGcolor textColor_red = nvgRGB(0xff, 0x33, 0x33);
    static const NVGcolor ledTextColor = textColor_red;

    struct NoteDisplayWidget : TransparentWidget {
        std::shared_ptr<Font> font;
        std::string fontPath;
        char displayStr[16];
        std::string* text;

        NoteDisplayWidget(std::string* t)
        {
            text = t;
            fontPath = std::string(
                asset::plugin(pluginInstance, "res/fonts/opensans/OpenSans-Bold.ttf"));
        }

        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1) {
                if (!(font = APP->window->loadFont(fontPath))) {
                    return;
                }
                nvgFontSize(args.vg, 18.0);
                nvgFontFaceId(args.vg, font->handle);

                Vec textPos = Vec(6, 24);

                nvgFillColor(args.vg, ledTextColor);

                nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
                nvgText(args.vg, textPos.x, textPos.y, text ? text->c_str() : "", NULL);
            }
        }
    };

#define FIRST_X -14.0
#define FIRST_Y 12.0
#define SPACING_X 20.0
#define SPACING_Y 9.5
#define FIRST_X_OUT -8.0
#define PAIR_SPACING 0.42
#define LABEL_OFFSET_X -19.0
#define LABEL_OFFSET_Y -12.0
#define LABEL_HEIGHT 22
#define LABEL_WIDTH 55

#define SPACING_X_OUT 14.1
#define LABEL_OFFSET_X_OUT (LABEL_OFFSET_X + 2.0)
#define LABEL_OFFSET_Y_OUT (LABEL_OFFSET_Y - 0.5) // leave space for the shading under the output jacks

#define LED_OFFSET_X 5.0
#define LED_OFFSET_Y -5.0

    struct NoteMeterWidget : ModuleWidget {
        NoteMeterWidget(NoteMeter* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/NoteMeter.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            int row, col;
            for (row = 0; row < NUM_INPUT_ROWS; row++) {
                auto y = FIRST_Y + row * SPACING_Y;
                for (col = 0; col < NUM_INPUT_COLS; col++) {
                    auto in = NoteMeter::PITCH_INPUT_1 + row * NUM_INPUT_COLS + col;
                    auto x = FIRST_X + SPACING_X + col * SPACING_X;
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(x, y)), module, in));

                    auto noteDisplay = new NoteDisplayWidget(module ? &module->text[row] : NULL);
                    noteDisplay->box.size = Vec(30, 10);
                    noteDisplay->box.pos = mm2px(Vec(x + LED_OFFSET_X, y + LED_OFFSET_Y));
                    addChild(noteDisplay);
                }
            }
        }
    };

} // namespace NoteMeter
} // namespace Chinenual

Model* modelNoteMeter = createModel<Chinenual::NoteMeter::NoteMeter,
    Chinenual::NoteMeter::NoteMeterWidget>("NoteMeter");
