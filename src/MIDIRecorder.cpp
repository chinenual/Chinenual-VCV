#include "plugin.hpp"

#define NUM_TRACKS 10
#define NUM_PER_TRACK_INPUTS 6


struct MidiTrack : dsp::MidiGenerator<PORT_MAX_CHANNELS> {
	void onMessage(const midi::Message& message) override {
		/// do something
	}

	void reset() {
		MidiGenerator::reset();
	}
};

struct MIDIRecorder : Module {
	enum ParamId {
		RUN_PARAM,
		PARAMS_LEN
	};
	enum InputId {
		BPM_INPUT,
		RUN_INPUT,
		T1_PITCH_INPUT, T1_GATE_INPUT, T1_VEL_INPUT, T1_AFT_INPUT, T1_PW_INPUT, T1_MW_INPUT,
		T2_PITCH_INPUT, T2_GATE_INPUT, T2_VEL_INPUT, T2_AFT_INPUT, T2_PW_INPUT, T2_MW_INPUT,
		T3_PITCH_INPUT, T3_GATE_INPUT, T3_VEL_INPUT, T3_AFT_INPUT, T3_PW_INPUT, T3_MW_INPUT,
		T4_PITCH_INPUT, T4_GATE_INPUT, T4_VEL_INPUT, T4_AFT_INPUT, T4_PW_INPUT, T4_MW_INPUT,
		T5_PITCH_INPUT, T5_GATE_INPUT, T5_VEL_INPUT, T5_AFT_INPUT, T5_PW_INPUT, T5_MW_INPUT,
		T6_PITCH_INPUT, T6_GATE_INPUT, T6_VEL_INPUT, T6_AFT_INPUT, T6_PW_INPUT, T6_MW_INPUT,
		T7_PITCH_INPUT, T7_GATE_INPUT, T7_VEL_INPUT, T7_AFT_INPUT, T7_PW_INPUT, T7_MW_INPUT,
		T8_PITCH_INPUT, T8_GATE_INPUT, T8_VEL_INPUT, T8_AFT_INPUT, T8_PW_INPUT, T8_MW_INPUT,
		T9_PITCH_INPUT, T9_GATE_INPUT, T9_VEL_INPUT, T9_AFT_INPUT, T9_PW_INPUT, T9_MW_INPUT,
		T10_PITCH_INPUT, T10_GATE_INPUT, T10_VEL_INPUT, T10_AFT_INPUT, T10_PW_INPUT, T10_MW_INPUT,
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
	MidiTrack midiTracks[NUM_TRACKS];
	dsp::Timer rateLimiterTimer;

	MIDIRecorder() {
		onReset();
		
		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(BPM_INPUT, "Tempo/BPM");
		configInput(RUN_INPUT, "Start/Stop Gate");
		configSwitch(RUN_PARAM, 0.0f, 1.0f, 0.0f, "Start/Stop");
		
		int i,t;
		for (t = 0; t < NUM_TRACKS; t++) {
			for (i = 0; i < NUM_PER_TRACK_INPUTS; i++) {
				auto e = T1_PITCH_INPUT + t*NUM_PER_TRACK_INPUTS + i;
				const char* paramName[NUM_PER_TRACK_INPUTS] = {"Note pitch (V/oct)", "Note gate", "Note velocity", "Aftertouch", "Pitchbend", "Modwheel"};
				configInput(e, string::f("Track %d %s", t+1, paramName[i]));
			}
		}
	}

	void onReset() override {
		bpm = 120.0f;
		running = false;
		for (int i = 0; i < NUM_TRACKS; i++) {
			midiTracks[i].reset();
		}
	}
	
	void processMidiTrack(const ProcessArgs& args, const int track)  {
		const auto PITCH_INPUT = T1_PITCH_INPUT + track * NUM_PER_TRACK_INPUTS;
		const auto GATE_INPUT = T1_GATE_INPUT + track * NUM_PER_TRACK_INPUTS;
		const auto VEL_INPUT = T1_VEL_INPUT + track * NUM_PER_TRACK_INPUTS;
		const auto AFT_INPUT = T1_AFT_INPUT + track * NUM_PER_TRACK_INPUTS;
		const auto PW_INPUT = T1_PW_INPUT + track * NUM_PER_TRACK_INPUTS;
		const auto MW_INPUT = T1_MW_INPUT + track * NUM_PER_TRACK_INPUTS;

		// MIDI specific processing adapted from VCV Core's CV_MIDI.cpp:
		
		// MIDI baud rate is 31250 b/s, or 3125 B/s.
		// CC messages are 3 bytes, so we can send a maximum of 1041 CC messages per second.
		// Since multiple CCs can be generated, play it safe and limit the CC rate to 200 Hz.
		const float rateLimiterPeriod = 1 / 200.f;
		bool rateLimiterTriggered = (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
		if (rateLimiterTriggered)
			rateLimiterTimer.time -= rateLimiterPeriod;

		midiTracks[track].setFrame(args.frame);

		for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
			int vel = (int) std::round(inputs[VEL_INPUT].getNormalPolyVoltage(10.f * 100 / 127, c) / 10.f * 127);
			vel = clamp(vel, 0, 127);
			midiTracks[track].setVelocity(vel, c);

			int note = (int) std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
			note = clamp(note, 0, 127);
			bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
			midiTracks[track].setNoteGate(note, gate, c);

			int aft = (int) std::round(inputs[AFT_INPUT].getPolyVoltage(c) / 10.f * 127);
			aft = clamp(aft, 0, 127);
			midiTracks[track].setKeyPressure(aft, c);
		}

		if (rateLimiterTriggered) {
			int pw = (int) std::round((inputs[PW_INPUT].getVoltage() + 5.f) / 10.f * 0x4000);
			pw = clamp(pw, 0, 0x3fff);
			midiTracks[track].setPitchWheel(pw);

			int mw = (int) std::round(inputs[MW_INPUT].getVoltage() / 10.f * 127);
			mw = clamp(mw, 0, 127);
			midiTracks[track].setModWheel(mw);
		}

	}
	
	void processMidi(const ProcessArgs& args)  {
		for (int i = 0; i < NUM_TRACKS; i++) {
			processMidiTrack(args, i);
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

		if (running) {
			processMidi(args);
		}
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
			for (i = 0; i < NUM_PER_TRACK_INPUTS; i++) {
				auto e = MIDIRecorder::T1_PITCH_INPUT + t*NUM_PER_TRACK_INPUTS + i;
				addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X + SPACING + i*SPACING, y)), module, e));
			}
		}
	}
};


Model* modelMIDIRecorder = createModel<MIDIRecorder, MIDIRecorderWidget>("MIDIRecorder");
