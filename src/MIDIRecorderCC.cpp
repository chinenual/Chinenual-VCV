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
      : cc(cc), is14bit(is14bit), range(range) {}
};

struct MIDIRecorderCC : MIDIRecorderBase {
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
  static const int NUM_COLS = 5;
  static const InputId T1_FIRST_COLUMN = T1_CC_1_INPUT;
  static const InputId T1_LAST_COLUMN = T1_CC_5_INPUT;

  enum OutputId { OUTPUTS_LEN };
  enum LightId { LIGHTS_LEN };

  // persisted state:

  CCConfig cc_config[NUM_COLS] = {
      CCConfig(10, false, CV_RANGE_n10_10),
      CCConfig(11, false, CV_RANGE_n10_10),
      CCConfig(12, false, CV_RANGE_n10_10),
      CCConfig(13, false, CV_RANGE_n10_10),
      CCConfig(14, false, CV_RANGE_n10_10),
  };

  MIDIRecorderCC() {
    onReset();

    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    int i, t;
    for (t = 0; t < NUM_TRACKS; t++) {
      for (i = 0; i < NUM_COLS; i++) {
        auto e = T1_FIRST_COLUMN + t * NUM_COLS + i;
        configInput(e, string::f("Track %d CC#%d", t + 1, i + 1));
      }
    }
  }

  void onReset() override {}
#if 0
	json_t *dataToJson() override
	{
		// json_t *rootJ = json_object();
		//		json_object_set_new(rootJ, "path", json_string(path.c_str()));
		//		json_object_set_new(rootJ, "increment_path", json_boolean(increment_path));
		//		json_object_set_new(rootJ, "align_to_first_note", json_boolean(align_to_first_note));
		// return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		//		json_t *pathJ = json_object_get(rootJ, "path");
		//		if (pathJ)
		//			setPath(json_string_value(pathJ));
	}
#endif

  void process(const ProcessArgs &args) override {
    MIDIRecorderBase::process(args);
  }
};

struct ccNumField : TextField {
  MIDIRecorderCC *module;
  unsigned int index = 0;

  ccNumField(unsigned int index) {
    this->index = index;
    this->box.pos.x = 90;   // position of the left side of the text field
    this->box.size.x = 50;  // width of the text field
    this->multiline = false;
  }

  void onChange(const event::Change &e) override {
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
  CCConfig *cc_config_ptr;

  CCDisplayWidget(CCConfig *cc_config) {
    cc_config_ptr = cc_config;
    fontPath = std::string(
        asset::plugin(pluginInstance, "res/fonts/DSEG14Modern-BoldItalic.ttf"));
  }

  void drawLayer(const DrawArgs &args, int layer) override {
    if (layer == 1) {
      if (!(font = APP->window->loadFont(fontPath))) {
        return;
      }
      nvgFontSize(args.vg, 9.2);
      nvgFontFaceId(args.vg, font->handle);

      Vec textPos = Vec(6, 24);

      nvgFillColor(args.vg, ledTextColor);

      if (!cc_config_ptr) {
        snprintf(displayStr, 16, "---");
      } else {
        snprintf(displayStr, 16, "%3u", cc_config_ptr->cc);
      }
      nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
    }
  }
};

#define FIRST_X 10.0
#define FIRST_Y 19.0
#define SPACING 11.0

struct MIDIRecorderCCWidget : ModuleWidget {
  MIDIRecorderCCWidget(MIDIRecorderCC *module) {
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
      auto y = FIRST_Y + t * SPACING;
      for (i = 0; i < MIDIRecorderCC::NUM_COLS; i++) {
        auto e =
            MIDIRecorderCC::T1_FIRST_COLUMN + t * MIDIRecorderCC::NUM_COLS + i;
        addInput(createInputCentered<PJ301MPort>(
            mm2px(Vec(FIRST_X + i * SPACING, y)), module, e));
      }
    }
    for (i = 0; i < MIDIRecorderCC::NUM_COLS; i++) {
      auto ccDisplay =
          new CCDisplayWidget(module ? &module->cc_config[i] : NULL);
      ccDisplay->box.size = Vec(30, 10);
      const int CCDISPLAY_Y = FIRST_Y - 1.3 * SPACING;
      ccDisplay->box.pos =
          mm2px(Vec(FIRST_X + i * SPACING - SPACING / 1.8, CCDISPLAY_Y));
      addChild(ccDisplay);
    }
  }

  void appendContextMenu(Menu *menu) override {
    MIDIRecorderCC *module = dynamic_cast<MIDIRecorderCC *>(this->module);

    menu->addChild(new MenuSeparator);

    for (int i = 0; i < MIDIRecorderCC::NUM_COLS; i++) {
      menu->addChild(
          createSubmenuItem(string::f("CC#%d", i + 1), "", [=](Menu *menu) {
            menu->addChild(createIndexSubmenuItem(
                "Input Range",
                {
                    "-10 to 10",
                    "  0 to 10",
                    " -5 to 5",
                    "  0 to 5",
                    " -3 to 3",
                    "  0 to 3",
                    " -1 to 1",
                    "  0 to 1",
                },
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
              holder->box.size.x = 170;  //  width of the menu
              holder->box.size.y = 20;

              auto lab = new rack::Label;
              lab->text = "MIDI CC: ";
              lab->box.size.y = 50;   // label box size determins the bounding
                                      // box around #1, #2, #3 etc.
              lab->box.size.x = 100;  // label box size determins the bounding
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

Model *modelMIDIRecorderCC =
    createModel<MIDIRecorderCC, MIDIRecorderCCWidget>("MIDIRecorderCC");

}  // namespace MIDIRecorder
}  // namespace Chinenual