#include "plugin.hpp"

#include "MidiFile.h"

#define NUM_TRACKS 10
#define NUM_PER_TRACK_INPUTS 6
#define MIDI_FILE_PPQ 960
#define SEC_PER_MINUTE 60

struct MidiCollector : dsp::MidiGenerator<PORT_MAX_CHANNELS> {

	smf::MidiFile &midiFile;
	int track;
	int &tick;

	MidiCollector(smf::MidiFile &midiFile, int track, int &tick) : midiFile(midiFile), track(track), tick(tick) { }
	
	void onMessage(const midi::Message& message) override {
		/// do something
		// convert to the smf library's classes:
		smf::MidiMessage smfMsg(message.bytes);
		midiFile.addEvent(track, tick, smfMsg);
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
	int tick;
	float total_time_s;
	
	smf::MidiFile midiFile;
	MidiCollector MidiCollectors[NUM_TRACKS] = {
		MidiCollector(midiFile, 0,tick),
		MidiCollector(midiFile, 1,tick),
		MidiCollector(midiFile, 2,tick),
		MidiCollector(midiFile, 3,tick),
		MidiCollector(midiFile, 4,tick),
		MidiCollector(midiFile, 5,tick),
		MidiCollector(midiFile, 6,tick),
		MidiCollector(midiFile, 7,tick),
		MidiCollector(midiFile, 8,tick),
		MidiCollector(midiFile, 9,tick),
	};
	
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

	void clearRecording() {
		midiFile.clear();
	}
	
	void onReset() override {
		bpm = 120.0f;
		running = false;
		tick = 0;
		clearRecording();
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

		MidiCollectors[track].setFrame(args.frame);

		for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
			int vel = (int) std::round(inputs[VEL_INPUT].getNormalPolyVoltage(10.f * 100 / 127, c) / 10.f * 127);
			vel = clamp(vel, 0, 127);
			MidiCollectors[track].setVelocity(vel, c);

			int note = (int) std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
			note = clamp(note, 0, 127);
			bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
			MidiCollectors[track].setNoteGate(note, gate, c);

			int aft = (int) std::round(inputs[AFT_INPUT].getPolyVoltage(c) / 10.f * 127);
			aft = clamp(aft, 0, 127);
			MidiCollectors[track].setKeyPressure(aft, c);
		}

		if (rateLimiterTriggered) {
			int pw = (int) std::round((inputs[PW_INPUT].getVoltage() + 5.f) / 10.f * 0x4000);
			pw = clamp(pw, 0, 0x3fff);
			MidiCollectors[track].setPitchWheel(pw);

			int mw = (int) std::round(inputs[MW_INPUT].getVoltage() / 10.f * 127);
			mw = clamp(mw, 0, 127);
			MidiCollectors[track].setModWheel(mw);
		}

	}
	
	void processMidi(const ProcessArgs& args)  {
		total_time_s += args.sampleTime;
		// PPQ = ticks/beat;  BPM = beat/minute;
		tick = std::round(total_time_s * bpm / SEC_PER_MINUTE * MIDI_FILE_PPQ);
		for (int i = 0; i < NUM_TRACKS; i++) {
			processMidiTrack(args, i);
		}
	}

	void startRecording(const ProcessArgs& args) {
		clearRecording();
		// max track where inputs are connected?
		int num_tracks = NUM_TRACKS;
		/*
		for (int t = NUM_TRACKS-1; t>=0; t--) {
			// are any of the inputs on this row connected?
			for (int i = 0; i < NUM_PER_TRACK_INPUTS; i++) {
				auto id = T1_PITCH_INPUT + i + t*NUM_PER_TRACK_INPUTS;
				if (inputs[id].isConnected()) {
					num_tracks = t+1;
					break;
				}
			}
		}
		*/
		midiFile.addTracks(num_tracks);

		midiFile.setTPQ(MIDI_FILE_PPQ);		
		midiFile.makeAbsoluteTicks();

		for (int t = 0; t < num_tracks; t++) {
			midiFile.addTempo(t, 0, bpm);
		}
		
		total_time_s = 0.0f;
		INFO("Start Recording... BPM: %f num_tracks: %d", bpm, num_tracks);
		running = true;
	}
	
	void stopRecording(const ProcessArgs& args) {
		running = false;
		INFO("Stop Recording.  total_time_s=%f ticks=%d",total_time_s,tick);
		for (int t = 0; t < midiFile.getNumTracks(); t++) {
			if (midiFile[t].size() <= 2) {
				// unused track - just the tempo info
				midiFile[t].clear();
			}
		}
		midiFile.write("/tmp/test.mid");
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

		auto was_running = running;
		int run_requested;
		// Run button:
		if (inputs[RUN_INPUT].isConnected()) {
			run_requested = inputs[RUN_INPUT].getVoltage() > 0.0f;
		} else {
			run_requested = params[RUN_PARAM].getValue() > 0.0f;
		}

		if (run_requested) {
			if (! was_running) {
				startRecording(args);
			}
			processMidi(args);
		} else {
			if (was_running) {
				stopRecording(args);
			}
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
			for (i = 0; i < NUM_PER_TRACK_INPUTS; i++) {
				auto e = MIDIRecorder::T1_PITCH_INPUT + t*NUM_PER_TRACK_INPUTS + i;
				addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X + SPACING + i*SPACING, y)), module, e));
			}
		}
	}
};


Model* modelMIDIRecorder = createModel<MIDIRecorder, MIDIRecorderWidget>("MIDIRecorder");
