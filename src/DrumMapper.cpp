#include <osdialog.h>

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
    int defaultMap[NUM_INPUTS] = {
        36, // kick
        43, // l-tom
        38, // snare
        47, // m-tom
        31, // rim
        50, // h-tom
        44, // p-hat
        39, // clap
        42, // c-hat
        49, // crash
        46, // o-hat
        51, // ride
    };

    GeneralMIDI generalMidiNames[] = {
        { 0, "unused", "unused" },
        { 1, "1", "1" },
        { 2, "2", "2" },
        { 3, "3", "3" },
        { 4, "4", "4" },
        { 5, "5", "5" },
        { 6, "6", "6" },
        { 7, "7", "7" },
        { 8, "8", "8" },
        { 9, "9", "9" },
        { 10, "10", "10" },
        { 11, "11", "11" },
        { 12, "12", "12" },
        { 13, "13", "13" },
        { 14, "14", "14" },
        { 15, "15", "15" },
        { 16, "16", "16" },
        { 17, "17", "17" },
        { 18, "18", "18" },
        { 19, "19", "19" },
        { 20, "20", "20" },
        { 21, "21", "21" },
        { 22, "22", "22" },
        { 23, "23", "23" },
        { 24, "24", "24" },
        { 25, "25", "25" },
        { 26, "26", "26" },
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
        { 44, "Pedal Hi-hat", "P-hat" },
        { 45, "Mid Tom 2", "M-Tom2" },
        { 46, "Open Hi-hat", "O-hat" },
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
        { 88, "88", "88" },
        { 89, "89", "89" },
        { 90, "90", "90" },
        { 91, "91", "91" },
        { 92, "92", "92" },
        { 93, "93", "93" },
        { 94, "94", "94" },
        { 95, "95", "95" },
        { 96, "96", "96" },
        { 97, "97", "97" },
        { 98, "98", "98" },
        { 99, "99", "99" },
        { 100, "100", "100" },
        { 101, "101", "101" },
        { 102, "102", "102" },
        { 103, "103", "103" },
        { 104, "104", "104" },
        { 105, "105", "105" },
        { 106, "106", "106" },
        { 107, "107", "107" },
        { 108, "108", "108" },
        { 109, "109", "109" },
        { 110, "110", "110" },
        { 111, "111", "111" },
        { 112, "112", "112" },
        { 113, "113", "113" },
        { 114, "114", "114" },
        { 115, "115", "115" },
        { 116, "116", "116" },
        { 117, "117", "117" },
        { 118, "118", "118" },
        { 119, "119", "119" },
        { 120, "120", "120" },
        { 121, "121", "121" },
        { 122, "122", "122" },
        { 123, "123", "123" },
        { 124, "124", "124" },
        { 125, "125", "125" },
        { 126, "126", "126" },
        { 127, "127", "127" }
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
            INPUTS_LEN
        };
        enum OutputId {
            PITCH_OUTPUT,
            GATE_OUTPUT,
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
            for (int i = GATE_INPUT_1; i < INPUTS_LEN; i++) {
                configInput(i, string::f("Gate %d", i));
            }
            configOutput(PITCH_OUTPUT, "Pitches (V/oct)");
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
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
        }

        void process(const ProcessArgs& args) override
        {
        }
    };

    // light yellow - the logo color:
    static const NVGcolor textColor_yellow = nvgRGB(0xff, 0xd4, 0x56);

    struct LabelDisplayWidget : TransparentWidget {
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

                Vec textPos = Vec(6, 24);

                nvgFillColor(args.vg, textColor_yellow);

                if (generalMidiIndexPtr) {
                    snprintf(displayStr, 16, "%s", generalMidiNames[*generalMidiIndexPtr].label);
                }
                nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);

                nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
            }
        }
    };

#define FIRST_X -3.0
#define FIRST_Y 20.0
#define SPACING_X 15.0
#define SPACING_Y 16.0
#define LABEL_OFFSET_X -3.0
#define LABEL_OFFSET_Y -16.0

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
                    auto e = DrumMap::GATE_INPUT_1 + row * NUM_INPUT_COLS + col;
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(FIRST_X + SPACING_X + col * SPACING_X, y)), module, e));

                    {
                        int i = row * NUM_INPUT_COLS + col;
                        auto labelDisplay = new LabelDisplayWidget(module ? &module->map[i] : NULL);
                        labelDisplay->box.size = Vec(30, 10);
                        labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X + SPACING_X + col * SPACING_X, y + LABEL_OFFSET_Y));
                        addChild(labelDisplay);
                    }
                }
            }
            {
                auto y = FIRST_Y + 6 * SPACING_Y;
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X + 0 * SPACING_X, y)), module, DrumMap::PITCH_OUTPUT));
                addOutput(createOutputCentered<PJ301MPort>(
                    mm2px(Vec(FIRST_X + SPACING_X + 1 * SPACING_X, y)), module, DrumMap::GATE_OUTPUT));
                {
                    auto labelDisplay = new LabelDisplayWidget(NULL);
                    labelDisplay->setStaticLabel("V/OCT");
                    labelDisplay->box.size = Vec(30, 10);
                    labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X + SPACING_X + 0 * SPACING_X, y + LABEL_OFFSET_Y));
                    addChild(labelDisplay);
                }
                {
                    auto labelDisplay = new LabelDisplayWidget(NULL);
                    labelDisplay->setStaticLabel("GATE");
                    labelDisplay->box.size = Vec(30, 10);
                    labelDisplay->box.pos = mm2px(Vec(LABEL_OFFSET_X + SPACING_X + 1 * SPACING_X, y + LABEL_OFFSET_Y));
                    addChild(labelDisplay);
                }
            }
        }
    };

} // namespace DrumMap
} // namespace Chinenual

Model* modelDrumMap = createModel<Chinenual::DrumMap::DrumMap,
    Chinenual::DrumMap::DrumMapWidget>("DrumMap");
