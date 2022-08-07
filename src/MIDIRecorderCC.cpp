#include "CVRange.hpp"
#include "MIDIRecorderBase.hpp"
#include "plugin.hpp"

namespace Chinenual {
namespace MIDIRecorder {

    struct CCConfig {
        int cc;
        bool is14bit;
        CVRangeIndex range;

        CCConfig(const int cc, const bool is14bit, CVRangeIndex range)
            : cc(cc)
            , is14bit(is14bit)
            , range(range)
        {
        }
    };

    struct MIDIRecorderCC : MIDIRecorderBase<5> {
        ExpanderToMasterMessage expander_to_master_message_a;
        ExpanderToMasterMessage expander_to_master_message_b;

        enum ParamId { PARAMS_LEN };
        enum InputId {
            T1_CC_1_INPUT,
            T1_CC_2_INPUT,
            T1_CC_3_INPUT,
            T1_CC_4_INPUT,
            T1_CC_5_INPUT,
            T2_CC_1_INPUT,
            T2_CC_2_INPUT,
            T2_CC_3_INPUT,
            T2_CC_4_INPUT,
            T2_CC_5_INPUT,
            T3_CC_1_INPUT,
            T3_CC_2_INPUT,
            T3_CC_3_INPUT,
            T3_CC_4_INPUT,
            T3_CC_5_INPUT,
            T4_CC_1_INPUT,
            T4_CC_2_INPUT,
            T4_CC_3_INPUT,
            T4_CC_4_INPUT,
            T4_CC_5_INPUT,
            T5_CC_1_INPUT,
            T5_CC_2_INPUT,
            T5_CC_3_INPUT,
            T5_CC_4_INPUT,
            T5_CC_5_INPUT,
            T6_CC_1_INPUT,
            T6_CC_2_INPUT,
            T6_CC_3_INPUT,
            T6_CC_4_INPUT,
            T6_CC_5_INPUT,
            T7_CC_1_INPUT,
            T7_CC_2_INPUT,
            T7_CC_3_INPUT,
            T7_CC_4_INPUT,
            T7_CC_5_INPUT,
            T8_CC_1_INPUT,
            T8_CC_2_INPUT,
            T8_CC_3_INPUT,
            T8_CC_4_INPUT,
            T8_CC_5_INPUT,
            T9_CC_1_INPUT,
            T9_CC_2_INPUT,
            T9_CC_3_INPUT,
            T9_CC_4_INPUT,
            T9_CC_5_INPUT,
            T10_CC_1_INPUT,
            T10_CC_2_INPUT,
            T10_CC_3_INPUT,
            T10_CC_4_INPUT,
            T10_CC_5_INPUT,
            INPUTS_LEN
        };

        // range of one row of input:
        static const InputId T1_FIRST_COLUMN = T1_CC_1_INPUT;
        static const InputId T1_LAST_COLUMN = T1_CC_5_INPUT;

        enum OutputId { OUTPUTS_LEN };
        enum LightId { LIGHTS_LEN };

        // persisted state:
        CCConfig cc_config[COLS_PER_TRACK] = {
            CCConfig(2, false, CV_RANGE_0_10),
            CCConfig(3, false, CV_RANGE_0_10),
            CCConfig(4, false, CV_RANGE_0_10),
            CCConfig(5, false, CV_RANGE_0_10),
            CCConfig(6, false, CV_RANGE_0_10),
        };

        MIDIRecorderCC()
            : MIDIRecorderBase(T1_CC_1_INPUT)
        {
            leftExpander.consumerMessage = &expander_to_master_message_a;
            leftExpander.producerMessage = &expander_to_master_message_b;

            onReset();

            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

            int i, t;
            for (t = 0; t < NUM_TRACKS; t++) {
                for (i = 0; i < COLS_PER_TRACK; i++) {
                    auto e = T1_FIRST_COLUMN + t * COLS_PER_TRACK + i;
                    configInput(e, string::f("Track %d CC#%d", t + 1, i + 1));
                }
            }
        }

        void onReset() override
        {
            MIDIRecorderBase::onReset();
            for (int i = 0; i < COLS_PER_TRACK; i++) {
                cc_config[i].cc = 2 + i;
                cc_config[i].is14bit = false;
                cc_config[i].range = CV_RANGE_0_10;
            }
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_t* cc_config_arrayJ = json_array();
            for (int i = 0; i < COLS_PER_TRACK; i++) {
                json_t* cc_configJ = json_object();
                json_object_set_new(cc_configJ, "is14bit",
                    json_boolean(cc_config[i].is14bit));
                json_object_set_new(cc_configJ, "cc", json_integer(cc_config[i].cc));
                json_object_set_new(cc_configJ, "range",
                    json_integer(cc_config[i].range));
                json_array_append_new(cc_config_arrayJ, cc_configJ);
            }
            json_object_set_new(rootJ, "cc_config", cc_config_arrayJ);

            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            json_t* cc_config_arrayJ = json_object_get(rootJ, "cc_config");
            if (cc_config_arrayJ) {
                size_t i;
                json_t* eleJ;
                json_array_foreach(cc_config_arrayJ, i, eleJ)
                {
                    json_t* ccJ = json_object_get(eleJ, "cc");
                    if (ccJ) {
                        cc_config[i].cc = json_integer_value(ccJ);
                    }
                    json_t* is14bitJ = json_object_get(eleJ, "is14bit");
                    if (is14bitJ) {
                        cc_config[i].is14bit = json_boolean_value(is14bitJ);
                    }
                    json_t* rangeJ = json_object_get(eleJ, "range");
                    if (rangeJ) {
                        cc_config[i].range = (CVRangeIndex)json_integer_value(rangeJ);
                    }
                }
            }
        }

        void processMidiTrack(const int track, const ProcessArgs& args)
        {
            ExpanderToMasterMessage* expanderMsg = (ExpanderToMasterMessage*)leftExpander.producerMessage;

            const auto COL0_INPUT = FIRST_INPUT_ID + track * COLS_PER_TRACK;
            for (int i = 0; i < COLS_PER_TRACK; i++) {
                int inputId = COL0_INPUT + i;
                if (inputs[inputId].isConnected()) {
                    float v = inputs[inputId].getVoltage();
                    if (cc_config[i].is14bit) {
                        int val = CVRanges[cc_config[i].range].to14bit(v);
                        int msb, lsb;
                        CVRanges[cc_config[i].range].split14bit(val, msb, lsb);
                        smf::MidiMessage ccMsg_1, ccMsg_2;
                        ccMsg_1.makeController(0, cc_config[i].cc, msb);
                        expanderMsg->msgs[track].push_back(ccMsg_1);
                        if (cc_config[i].cc + 32 <= 127) {
                            // silently ignore attempt to write invalid CCnumber
                            ccMsg_2.makeController(0, cc_config[i].cc + 32, lsb);
                            expanderMsg->msgs[track].push_back(ccMsg_2);
                        }
                    } else {
                        int val = CVRanges[cc_config[i].range].to7bit(v);
                        smf::MidiMessage ccMsg;
                        ccMsg.makeController(0, cc_config[i].cc, val);
                        expanderMsg->msgs[track].push_back(ccMsg);
                    }
                }
            }
        }

        void process(const ProcessArgs& args) override
        {
            MIDIRecorderBase::process(args);
            // are we connected to a master module (possibly with other expanders in between?)
            bool connected = false;
            Module* m = this;
            Module* master = 0;
            while (m) {
                if (m->model == modelMIDIRecorder) {
                    connected = true;
                    master = m;
                    break;
                }
                if (m->model != modelMIDIRecorderCC) {
                    break;
                }
                m = m->leftExpander.module;
            }
            if (connected) {
                auto consumerMessage = (MasterToExpanderMessage*)master->rightExpander.consumerMessage;
                auto producerMessage = (ExpanderToMasterMessage*)leftExpander.producerMessage;
                if (consumerMessage->isRecording) {
                    for (int t = 0; t < NUM_TRACKS; t++) {
                        producerMessage->msgs[t].clear();
                        producerMessage->active[t] = trackIsActive(t);
                        if (rateLimiterTriggered && trackIsActive(t)) {
                            processMidiTrack(t, args);
                        }
                        if (producerMessage->msgs[t].size() > 0) {
                            INFO("TRACK %d %lu msgs", t, producerMessage->msgs[t].size());
                        }
                    }
                    leftExpander.requestMessageFlip();
                }
            }
        }
    };

    struct ccNumField : TextField {
        MIDIRecorderCC* module;
        unsigned int index = 0;

        ccNumField(unsigned int index)
        {
            this->index = index;
            this->box.pos.x = 90; // position of the left side of the text field
            this->box.size.x = 50; // width of the text field
            this->multiline = false;
        }

        void onChange(const event::Change& e) override
        {
            // Force Integer in range 0..127 from the text.  Type anything else and it
            // get cooerced to something in the range.

            int val = 0;
            // if empty text, leave it be - since the user may be intending to type a
            // key. when that happens, we'll reparse the string and get a new number
            //
            // otherwise, keep the string up to date with what we've parsed (this will
            // prevent non-number characters from ever even getting displayed:
            if (text != "") {
                val = (int)std::stol(text);
                val = rack::clamp((int)val, 0, 127);
                text = std::to_string(val);

                // HACK: without this, the cursor may be outside the text range and
                // trigger an assertion error. Can't just call setText() since that will
                // re-invoke the onChange event.  So set selection and cursor here
                // manually:
                selection = cursor = text.size();
            }

            module->cc_config[index].cc = val;
        };
    };

    struct CCDisplayWidget : TransparentWidget {
        std::shared_ptr<Font> font;
        std::string fontPath;
        char displayStr[16];
        CCConfig* cc_config_ptr;

        CCDisplayWidget(CCConfig* cc_config)
        {
            cc_config_ptr = cc_config;
            fontPath = std::string(
                asset::plugin(pluginInstance, "res/fonts/DSEG14Modern-BoldItalic.ttf"));
        }

        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1) {
                if (!(font = APP->window->loadFont(fontPath))) {
                    return;
                }
                nvgFontSize(args.vg, 11.0);
                nvgFontFaceId(args.vg, font->handle);

                Vec textPos = Vec(6, 24);

                nvgFillColor(args.vg, ledTextColor);

                if (!cc_config_ptr) {
                    snprintf(displayStr, 16, "---");
                } else {
                    snprintf(displayStr, 16, "%3u", cc_config_ptr->cc);
                }
                nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);

                nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
            }
        }
    };

#define FIRST_X 10.0
#define FIRST_Y 20.0
#define SPACING_X 10.0
#define SPACING_Y 10.5
#define LED_OFFSET_X 1.6
#define LED_OFFSET_Y -4.5

    struct MIDIRecorderCCWidget : ModuleWidget {
        MIDIRecorderCCWidget(MIDIRecorderCC* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/MIDIRecorderCC.svg")));

            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            int t, i;
            for (t = 0; t < NUM_TRACKS; t++) {
                auto y = FIRST_Y + t * SPACING_Y;
                for (i = 0; i < MIDIRecorderCC::COLS_PER_TRACK; i++) {
                    auto e = MIDIRecorderCC::T1_FIRST_COLUMN + t * MIDIRecorderCC::COLS_PER_TRACK + i;
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(FIRST_X + i * SPACING_X, y)), module, e));
                }
            }
            for (i = 0; i < MIDIRecorderCC::COLS_PER_TRACK; i++) {
                auto ccDisplay = new CCDisplayWidget(module ? &module->cc_config[i] : NULL);
                ccDisplay->box.size = Vec(30, 10);
                const int CCDISPLAY_Y = FIRST_Y - SPACING_Y + LED_OFFSET_Y;
                ccDisplay->box.pos = mm2px(Vec(FIRST_X + i * SPACING_X + LED_OFFSET_X, CCDISPLAY_Y));
                addChild(ccDisplay);
            }
        }

        void appendContextMenu(Menu* menu) override
        {
            MIDIRecorderCC* module = dynamic_cast<MIDIRecorderCC*>(this->module);

            menu->addChild(new MenuSeparator);

            for (int i = 0; i < MIDIRecorderCC::COLS_PER_TRACK; i++) {
                menu->addChild(
                    createSubmenuItem(string::f("CC#%d", i + 1), "", [=](Menu* menu) {
                        menu->addChild(createIndexSubmenuItem(
                            "Input Range", CVRangeNames,
                            [=]() { return module->cc_config[i].range; },
                            [=](int val) {
                                module->cc_config[i].range = (CVRangeIndex)val;
                            }));

                        menu->addChild(createBoolMenuItem(
                            "14bit", "", [=]() { return module->cc_config[i].is14bit; },
                            [=](bool val) { module->cc_config[i].is14bit = val; }));

                        {
                            // adapted from Voxglitch's DigitalSequencerXP:
                            // Add label input
                            auto holder = new rack::Widget;
                            holder->box.size.x = 170; //  width of the menu
                            holder->box.size.y = 20;

                            auto lab = new rack::Label;
                            lab->text = "MIDI CC: ";
                            lab->box.size.y = 50; // label box size determins the bounding
                                                  // box around #1, #2, #3 etc.
                            lab->box.size.x = 100; // label box size determins the bounding
                                                   // box around #1, #2, #3 etc.
                            holder->addChild(lab);

                            auto textfield = new ccNumField(i);
                            textfield->module = module;
                            textfield->text = std::to_string(module->cc_config[i].cc);
                            holder->addChild(textfield);

                            menu->addChild(holder);
                        }
                    }));
            }
        }
    };

} // namespace MIDIRecorder
} // namespace Chinenual

Model* modelMIDIRecorderCC = createModel<Chinenual::MIDIRecorder::MIDIRecorderCC,
    Chinenual::MIDIRecorder::MIDIRecorderCCWidget>(
    "MIDIRecorderCC");
