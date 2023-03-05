#include <osdialog.h>

#include "PitchNote.hpp"
#include "plugin.hpp"

#define NUM_INPUT_ROWS 6
#define NUM_INPUT_COLS 2
#define NUM_INPUTS (NUM_INPUT_ROWS * NUM_INPUT_COLS)

namespace Chinenual {
namespace DrumMap {

    struct GeneralMIDI {
        int note;
        const char* name;
        const char* label; // abbreviated for display
    };

    std::vector<GeneralMIDI> generalMidiDefinitions = {
        { 27, "High Q", "HighQ" },
        { 28, "Slap", "Slap" },
        { 29, "Scratch Push", "ScrPush" },
        { 30, "Scratch Pull", "ScrPull" },
        { 31, "Sticks", "Stick" },
        { 32, "Square Click", "S-Clk" },
        { 33, "Metronome Click", "M-Clk" },
        { 34, "Metronome Bell", "M-Bell" },
        { 35, "Bass Drum 2", "Kick2" },
        { 36, "Bass Drum 1", "Kick" },
        { 37, "Side Stick", "Stick" },
        { 38, "Snare Drum 1", "Snare" },
        { 39, "Hand Clap", "Clap" },
        { 40, "Snare Drum 2", "Snare2" },
        { 41, "Low Tom 2", "L-Tom2" },
        { 42, "Closed Hi-hat", "C-Hat" },
        { 43, "Low Tom 1", "L-Tom" },
        { 44, "Pedal Hi-hat", "P-Hat" },
        { 45, "Mid Tom 2", "M-Tom2" },
        { 46, "Open Hi-hat", "O-Hat" },
        { 47, "Mid Tom 1", "M-Tom" },
        { 48, "High Tom 2", "H-Tom2" },
        { 49, "Crash Cymbal 1", "Crash" },
        { 50, "High Tom 1", "H-Tom" },
        { 51, "Ride Cymbal 1", "Ride" },
        { 52, "Chinese Cymbal", "Chine" },
        { 53, "Ride Bell", "RBell" },
        { 54, "Tambourine", "Tamb" },
        { 55, "Splash Cymbal", "Splash" },
        { 56, "Cowbell", "Cowbell" },
        { 57, "Crash Cymbal 2", "Crash2" },
        { 58, "Vibra Slap", "Vibra" },
        { 59, "Ride Cymbal 2", "Ride2" },
        { 60, "High Bongo", "HBongo" },
        { 61, "Low Bongo", "LBongo" },
        { 62, "Mute High Conga", "MHConga" },
        { 63, "Open High Conga", "OHConga" },
        { 64, "Low Conga", "LConga" },
        { 65, "High Timbale", "HTimbal" },
        { 66, "Low Timbale", "LTimbal" },
        { 67, "High Agogo", "HAgogo" },
        { 68, "Low Agogo", "LAgogo" },
        { 69, "Cabasa", "Cabasa" },
        { 70, "Maracas", "Maraca" },
        { 71, "Short Whistle", "SWhistl" },
        { 72, "Long Whistle", "LWhistl" },
        { 73, "Short Guiro", "SGuiro" },
        { 74, "Long Guiro", "LGuiro" },
        { 75, "Claves", "Claves" },
        { 76, "High Wood Block", "HWBlock" },
        { 77, "Low Wood Block", "LWBlock" },
        { 78, "Mute Cuica", "M-Cuica" },
        { 79, "Open Cuica", "O-Cuica" },
        { 80, "Mute Triangle", "M-Tri" },
        { 81, "Open Triangle", "O-Tri" },
        { 82, "Shaker", "Shaker" },
        { 83, "Jingle Bell", "Jingle" },
        { 84, "Belltree", "Belltre" },
        { 85, "Castanets", "Castanet" },
        { 86, "Mute Surdo", "M-Surdo" },
        { 87, "Open Surdo", "O-Surdo" },
    };
    const int FIRST_GENMIDI_NOTE = generalMidiDefinitions[0].note;
    int defaultMap[NUM_INPUTS] = {
        36 - FIRST_GENMIDI_NOTE, // kick
        43 - FIRST_GENMIDI_NOTE, // l-tom
        38 - FIRST_GENMIDI_NOTE, // snare
        47 - FIRST_GENMIDI_NOTE, // m-tom
        31 - FIRST_GENMIDI_NOTE, // rim
        50 - FIRST_GENMIDI_NOTE, // h-tom
        44 - FIRST_GENMIDI_NOTE, // p-hat
        39 - FIRST_GENMIDI_NOTE, // clap
        42 - FIRST_GENMIDI_NOTE, // c-hat
        49 - FIRST_GENMIDI_NOTE, // crash
        46 - FIRST_GENMIDI_NOTE, // o-hat
        51 - FIRST_GENMIDI_NOTE, // ride
    };

    struct DrumMap : Module {
        enum ParamId {
            PARAMS_LEN
        };
        enum InputId {
            GATE_INPUT_1,
            GATE_INPUT_2,
            GATE_INPUT_3,
            GATE_INPUT_4,
            GATE_INPUT_5,
            GATE_INPUT_6,
            GATE_INPUT_7,
            GATE_INPUT_8,
            GATE_INPUT_9,
            GATE_INPUT_10,
            GATE_INPUT_11,
            GATE_INPUT_12,
            VEL_INPUT_1,
            VEL_INPUT_2,
            VEL_INPUT_3,
            VEL_INPUT_4,
            VEL_INPUT_5,
            VEL_INPUT_6,
            VEL_INPUT_7,
            VEL_INPUT_8,
            VEL_INPUT_9,
            VEL_INPUT_10,
            VEL_INPUT_11,
            VEL_INPUT_12,
            INPUTS_LEN
        };
        enum OutputId {
            PITCH_OUTPUT,
            GATE_OUTPUT,
            VEL_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            LIGHTS_LEN
        };

        int map[NUM_INPUTS];

        DrumMap()
        {
            onReset();

            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            for (int i = GATE_INPUT_1; i < GATE_INPUT_1 + NUM_INPUTS; i++) {
                configInput(i, string::f("Gate %d", i - GATE_INPUT_1));
            }
            for (int i = VEL_INPUT_1; i < VEL_INPUT_1 + NUM_INPUTS; i++) {
                configInput(i, string::f("Velocity %d", i - VEL_INPUT_1));
            }
            configOutput(PITCH_OUTPUT, "Pitches (V/oct)");
            configOutput(VEL_OUTPUT, "Velocities");
            configOutput(GATE_OUTPUT, "Gates");
        }

        void onReset() override
        {
            for (int i = 0; i < NUM_INPUTS; i++) {
                map[i] = defaultMap[i];
            }
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_t* mapJ = json_array();
            for (int i = 0; i < NUM_INPUTS; i++) {
                json_array_append_new(mapJ, json_integer(map[i]));
            }
            json_object_set(rootJ, "map", mapJ);
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;
            json_t* mapJ = json_object_get(rootJ, "map");
            if (mapJ == 0)
                return;
            if (json_array_size(mapJ) != NUM_INPUTS)
                return;
            size_t i;
            json_t* val;
            json_array_foreach(mapJ, i, val)
            {
                map[i] = json_integer_value(val);
            }
        }

        float velToVoltage(int vel)
        {
            // based on VCV CORE MIDI_CV:
            return rescale(vel, 0, 127, 0.f, 10.f);
        }

        void process(const ProcessArgs& args) override
        {
            int out_c = 0;

            for (int i = 0; i < NUM_INPUTS; i++) {
                auto gate = inputs[GATE_INPUT_1 + i];

                if (gate.isConnected()) {
                    outputs[PITCH_OUTPUT].setVoltage(pitchToVoltage(generalMidiDefinitions[map[i]].note), out_c);
                    outputs[GATE_OUTPUT].setVoltage(gate.getVoltage(), out_c);

                    auto vel = inputs[VEL_INPUT_1 + i];
                    if (vel.isConnected()) {
                        outputs[VEL_OUTPUT].setVoltage(vel.getVoltage(), out_c);
                    } else {
                        // default velocity
                        int default_vel = 100;
                        outputs[VEL_OUTPUT].setVoltage(velToVoltage(default_vel), out_c);
                    }
                    out_c++;
                }
            }
            outputs[PITCH_OUTPUT].setChannels(out_c);
            outputs[GATE_OUTPUT].setChannels(out_c);
            outputs[VEL_OUTPUT].setChannels(out_c);
        }
    };

    // yellow - the logo color:
    static const NVGcolor textColor_yellow = nvgRGB(0xff, 0xd4, 0x56);

    struct LabelDisplayWidget : Widget {
        std::shared_ptr<Font> font;
        std::string fontPath;
        char displayStr[16];
        int* generalMidiIndexPtr;

        LabelDisplayWidget(int* generalMidiIndex)
        {
            generalMidiIndexPtr = generalMidiIndex;
            fontPath = std::string(
                asset::plugin(pluginInstance, "res/fonts/opensans/OpenSans-Bold.ttf"));
        }

        void onButton(const ButtonEvent& e) override
        {
            if (generalMidiIndexPtr != 0) {
                // a user-editable input
                if (e.action == GLFW_PRESS) {
                    onShowMenu();
                    e.consume(this);
                }
                if (e.action == GLFW_RELEASE) {
                    e.consume(this);
                }
            }
        }

        void onShowMenu()
        {
            if (generalMidiIndexPtr != 0) {
                // a user-editable input
                std::vector<std::string> generalMidiMenuNames;
                for (auto v : generalMidiDefinitions) {
                    auto name = v.name;
                    auto note = v.note;
                    generalMidiMenuNames.push_back(string::f("%s (%d)", name, note));
                }

                auto menu = rack::createMenu();
                menu->addChild(createIndexSubmenuItem(
                    "General MIDI", generalMidiMenuNames,
                    [=]() { return *generalMidiIndexPtr; },
                    [=](int val) {
                        *generalMidiIndexPtr = val;
                    }));
            }
        }

        void setStaticLabel(const char* txt)
        {
            snprintf(displayStr, 16, "%s", txt);
        }

        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1) {
                if (!(font = APP->window->loadFont(fontPath))) {
                    return;
                }
                nvgFontSize(args.vg, 15.0);
                nvgFontFaceId(args.vg, font->handle);
                nvgShapeAntiAlias(args.vg, false);

                Vec textPos = Vec(box.getWidth() / 2.f,
                    (box.getHeight() / 2.f) + 4.f); // bias down a bit

#if 0
                nvgFillColor(args.vg, nvgRGB(0xff, 0x00, 0x00));
                nvgBeginPath(args.vg);
                nvgRect(args.vg, 0.f, 0.f, box.getWidth(), box.getHeight());
                nvgFill(args.vg);
#endif
                nvgFillColor(args.vg, textColor_yellow);

                if (generalMidiIndexPtr) {
                    snprintf(displayStr, 16, "%s", generalMidiDefinitions[*generalMidiIndexPtr].label);
                }
                nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

                nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
            }
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

    struct DrumMapWidget : ModuleWidget {
        DrumMapWidget(DrumMap* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/DrumMap.svg")));
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
                    auto e_gate = DrumMap::GATE_INPUT_1 + row * NUM_INPUT_COLS + col;
                    auto e_vel = DrumMap::VEL_INPUT_1 + row * NUM_INPUT_COLS + col;
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(FIRST_X + SPACING_X + col * SPACING_X, y)), module, e_gate));
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(FIRST_X + SPACING_X + (col + PAIR_SPACING) * SPACING_X, y)), module, e_vel));

                    {
                        int i = row * NUM_INPUT_COLS + col;
                        auto labelDisplay = new LabelDisplayWidget(module ? &module->map[i] : &defaultMap[i]);
                        labelDisplay->box.size = Vec(LABEL_WIDTH, LABEL_HEIGHT);
                        labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X + SPACING_X + col * SPACING_X, y + LABEL_OFFSET_Y));
                        addChild(labelDisplay);
                    }
                }
            }
            {
                auto y = FIRST_Y + 6 * SPACING_Y;
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, y)), module, DrumMap::PITCH_OUTPUT));
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 1 * SPACING_X_OUT, y)), module, DrumMap::GATE_OUTPUT));
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 2 * SPACING_X_OUT, y)), module, DrumMap::VEL_OUTPUT));
                {
                    auto labelDisplay = new LabelDisplayWidget(NULL);
                    labelDisplay->setStaticLabel("V/oct");
                    labelDisplay->box.size = Vec(LABEL_WIDTH, LABEL_HEIGHT);
                    labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, y + LABEL_OFFSET_Y_OUT));
                    addChild(labelDisplay);
                }
                {
                    auto labelDisplay = new LabelDisplayWidget(NULL);
                    labelDisplay->setStaticLabel("Gate");
                    labelDisplay->box.size = Vec(LABEL_WIDTH, LABEL_HEIGHT);
                    labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X_OUT + SPACING_X_OUT + 1 * SPACING_X_OUT, y + LABEL_OFFSET_Y_OUT));
                    addChild(labelDisplay);
                }
                {
                    auto labelDisplay = new LabelDisplayWidget(NULL);
                    labelDisplay->setStaticLabel("Vel");
                    labelDisplay->box.size = Vec(LABEL_WIDTH, LABEL_HEIGHT);
                    labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X_OUT + SPACING_X_OUT + 2 * SPACING_X_OUT, y + LABEL_OFFSET_Y_OUT));
                    addChild(labelDisplay);
                }
            }
        }
    };

} // namespace DrumMap
} // namespace Chinenual

Model* modelDrumMap = createModel<Chinenual::DrumMap::DrumMap,
    Chinenual::DrumMap::DrumMapWidget>("DrumMap");
