#include "plugin.hpp"
#include "MIDIRecorder.hpp"

static const NVGcolor ccTextColor = nvgRGB(0xff, 0x00, 0x00);

struct CCDisplayWidget : TransparentWidget
{
	std::shared_ptr<Font> font;
	std::string fontPath;
	char displayStr[16];

	CCDisplayWidget()
	{
		fontPath = std::string(asset::plugin(pluginInstance, "res/fonts/Segment14.ttf"));
	}

	void drawLayer(const DrawArgs &args, int layer) override
	{
		if (layer == 1)
		{
			if (!(font = APP->window->loadFont(fontPath)))
			{
				return;
			}
			nvgFontSize(args.vg, 12);
			nvgFontFaceId(args.vg, font->handle);

			Vec textPos = Vec(6, 24);

			nvgFillColor(args.vg, ccTextColor);

			int cc = 1;
			snprintf(displayStr, 16, "  %3u", cc);

			nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
		}
	}
};

struct CCConfig
{
	int cc;
	bool is14bit;

	CCConfig(int cc, bool is14bit) : cc(cc), is14bit(is14bit) {}
};

struct MIDIRecorderCC : Module
{
	enum ParamId
	{
		PARAMS_LEN
	};
	enum InputId
	{
		T1_CC_1_INPUT,
		T1_CC_2_INPUT,
		T1_CC_3_INPUT,
		T1_CC_4_INPUT,
		T1_CC_5_INPUT,
		INPUTS_LEN
	};

	// range of one row of input:
	static const int NUM_COLS = 5;
	static const InputId T1_FIRST_COLUMN = T1_CC_1_INPUT;
	static const InputId T1_LAST_COLUMN = T1_CC_5_INPUT;

	enum OutputId
	{
		OUTPUTS_LEN
	};
	enum LightId
	{
		LIGHTS_LEN
	};

	// persisted state:

	CCConfig cc_config[NUM_COLS] = {
		CCConfig(10, false),
		CCConfig(11, false),
		CCConfig(12, false),
		CCConfig(13, false),
		CCConfig(14, false),
	};

	MIDIRecorderCC()
	{
		onReset();

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

		int i, t;
		for (t = 0; t < NUM_TRACKS; t++)
		{
			for (i = 0; i < NUM_COLS; i++)
			{
				auto e = T1_FIRST_COLUMN + t * NUM_COLS + i;
				configInput(e, string::f("Track %d CC#%d", t + 1, i + 1));
			}
		}
	}

	void onReset() override
	{
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		//		json_object_set_new(rootJ, "path", json_string(path.c_str()));
		//		json_object_set_new(rootJ, "increment_path", json_boolean(increment_path));
		//		json_object_set_new(rootJ, "align_to_first_note", json_boolean(align_to_first_note));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		//		json_t *pathJ = json_object_get(rootJ, "path");
		//		if (pathJ)
		//			setPath(json_string_value(pathJ));
	}

	void process(const ProcessArgs &args) override
	{
	}
};

#define FIRST_X 10.0
#define FIRST_Y 19.0
#define SPACING 11.0

struct MIDIRecorderCCWidget : ModuleWidget
{
	MIDIRecorderCCWidget(MIDIRecorderCC *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MIDIRecorderCC.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		int t, i;
		for (t = 0; t < NUM_TRACKS; t++)
		{
			auto y = FIRST_Y + t * SPACING;
			for (i = 0; i < MIDIRecorderCC::NUM_COLS; i++)
			{
				auto e = MIDIRecorderCC::T1_FIRST_COLUMN + t * MIDIRecorderCC::NUM_COLS + i;
				addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X + SPACING + i * SPACING, y)), module, e));
			}
		}
	}

	void appendContextMenu(Menu *menu) override
	{
		MIDIRecorderCC *module = dynamic_cast<MIDIRecorderCC *>(this->module);

		menu->addChild(new MenuSeparator);

		for (int i = 0; i < MIDIRecorderCC::NUM_COLS; i++)
		{
			menu->addChild(createBoolMenuItem(
				"14bit",
				"",
				[=]()
				{ return module->cc_config[i].is14bit; },
				[=](bool val)
				{ module->cc_config[i].is14bit = val; }));
			menu->addChild(createIndexSubmenuItem(
				"CC",
				{"1", "2", "3"},
				[=]()
				{ return module->cc_config[i].cc; },
				[=](int val)
				{ module->cc_config[i].cc = val; }));
		}
	}
};

Model *modelMIDIRecorderCC = createModel<MIDIRecorderCC, MIDIRecorderCCWidget>("MIDIRecorderCC");