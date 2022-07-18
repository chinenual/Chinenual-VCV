#include "plugin.hpp"

#define NUM_TRACKS 10

struct MIDIRecorder : Module {
	enum ParamId {
		RUN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BPM_INPUT,
		RUN_INPUT,
		T1_NOTE_INPUT, T1_GATE_INPUT, T1_VEL_INPUT, T1_AFT_INPUT, T1_PW_INPUT, T1_MW_INPUT,
		T2_NOTE_INPUT, T2_GATE_INPUT, T2_VEL_INPUT, T2_AFT_INPUT, T2_PW_INPUT, T2_MW_INPUT,
		T3_NOTE_INPUT, T3_GATE_INPUT, T3_VEL_INPUT, T3_AFT_INPUT, T3_PW_INPUT, T3_MW_INPUT,
		T4_NOTE_INPUT, T4_GATE_INPUT, T4_VEL_INPUT, T4_AFT_INPUT, T4_PW_INPUT, T4_MW_INPUT,
		T5_NOTE_INPUT, T5_GATE_INPUT, T5_VEL_INPUT, T5_AFT_INPUT, T5_PW_INPUT, T5_MW_INPUT,
		T6_NOTE_INPUT, T6_GATE_INPUT, T6_VEL_INPUT, T6_AFT_INPUT, T6_PW_INPUT, T6_MW_INPUT,
		T7_NOTE_INPUT, T7_GATE_INPUT, T7_VEL_INPUT, T7_AFT_INPUT, T7_PW_INPUT, T7_MW_INPUT,
		T8_NOTE_INPUT, T8_GATE_INPUT, T8_VEL_INPUT, T8_AFT_INPUT, T8_PW_INPUT, T8_MW_INPUT,
		T9_NOTE_INPUT, T9_GATE_INPUT, T9_VEL_INPUT, T9_AFT_INPUT, T9_PW_INPUT, T9_MW_INPUT,
		T10_NOTE_INPUT, T10_GATE_INPUT, T10_VEL_INPUT, T10_AFT_INPUT, T10_PW_INPUT, T10_MW_INPUT,
		INPUTS_LEN
	};
	enum OutputId {
		OUTPUTS_LEN
	};
	enum LightId {
		RUN_LIGHT,
		LIGHTS_LEN
	};

	float bpm;
	bool running;
	
	MIDIRecorder() {
		bpm = 120.0f;
		running = false;
		
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(BPM_INPUT, "Tempo/BPM");
		configInput(RUN_INPUT, "Start/Stop Gate");
		configSwitch(RUN_PARAM, 0.0f, 1.0f, 0.0f, "Start/Stop");
		
		int i,t;
		for (t = 0; t < NUM_TRACKS; t++) {
			for (i = 0; i < 6; i++) {
				auto e = T1_NOTE_INPUT + t*6 + i;
				const char* paramName[6] = {"Note pitch (V/oct)", "Note gate", "Note velocity", "Aftertouch", "Pitchbend", "Modwheel"};
				configInput(e, string::f("Track %d %s", t+1, paramName[i]));
			}
		}
	}

	void process(const ProcessArgs& args) override {
		// From Impromptu's Clocked : bpm = 120*2^V
		float new_bpm;
		if (inputs[BPM_INPUT].isConnected()) {
			new_bpm = 120.0f * std::pow(2.0f, inputs[BPM_INPUT].getVoltage());
		} else {
			new_bpm = 120.0f; // default
		}
		if (new_bpm != bpm) {
			INFO("BPM: %f", new_bpm);
		}
		bpm = new_bpm;

		// Run button:
		if (inputs[RUN_INPUT].isConnected()) {
			running = params[RUN_INPUT].getValue() > 0.0f;
		} else {
			running = params[RUN_PARAM].getValue() > 0.0f;
		}
		lights[RUN_LIGHT].setBrightness(running);
	}
};

#define FIRST_X 10.0
#define FIRST_Y 19.0
#define SPACING 11.0

struct MIDIRecorderWidget : ModuleWidget {
	MIDIRecorderWidget(MIDIRecorder* module) {
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MIDIRecorder.svg")));


		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));			       

		addParam(createLightParamCentered<VCVLightBezelLatch<RedLight>>(mm2px(Vec(FIRST_X, FIRST_Y+4*SPACING)), module, MIDIRecorder::RUN_PARAM, MIDIRecorder::RUN_LIGHT));
		
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X, FIRST_Y+7*SPACING)), module, MIDIRecorder::RUN_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X, FIRST_Y+9*SPACING)), module, MIDIRecorder::BPM_INPUT));

		int t, i;
		for (t = 0; t < NUM_TRACKS; t++) {
			auto y = FIRST_Y + t * SPACING;
			for (i = 0; i < 6; i++) {
				auto e = MIDIRecorder::T1_NOTE_INPUT + t*6 + i;
				addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X + SPACING + i*SPACING, y)), module, e));
			}
		}
	}
};


Model* modelMIDIRecorder = createModel<MIDIRecorder, MIDIRecorderWidget>("MIDIRecorder");
