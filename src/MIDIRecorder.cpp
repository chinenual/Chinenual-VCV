#include <osdialog.h>
#include "plugin.hpp"

#include "MidiFile.h"

#define NUM_TRACKS 10
#define NUM_PER_TRACK_INPUTS 6
#define MIDI_FILE_PPQ 960
#define SEC_PER_MINUTE 60

struct MidiCollector : dsp::MidiGenerator<PORT_MAX_CHANNELS>
{
	smf::MidiFile &midiFile;
	int track;
	int &tick;

	MidiCollector(smf::MidiFile &midiFile, int track, int &tick) : midiFile(midiFile), track(track), tick(tick) {}

	void onMessage(const midi::Message &message) override
	{
		/// do something
		// convert to the smf library's classes:
		smf::MidiMessage smfMsg(message.bytes);
		midiFile.addEvent(track, tick, smfMsg);
	}

	void reset()
	{
		MidiGenerator::reset();
	}
};

static const NVGcolor bpmTextColor = nvgRGB(0xff, 0x00, 0x00);

struct BPMDisplayWidget : TransparentWidget
{
	std::shared_ptr<Font> font;
	std::string fontPath;
	char displayStr[16];
	double *bpm_ptr;

	BPMDisplayWidget(double *bpm)
	{
		bpm_ptr = bpm;
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

			nvgFillColor(args.vg, bpmTextColor);

			unsigned int bpm = bpm_ptr ? std::round(*bpm_ptr) : 120;
			snprintf(displayStr, 16, "  %3u", bpm);

			nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
		}
	}
};

static void selectPath(Module *module);

struct MIDIClock
{
	int tick;
	double last_tick; // unrounded
	double time_at_last_tempo_change;
	double tick_at_last_tempo_change;
	double total_time_s;
	double bpm;

	void reset(double new_bpm)
	{
		bpm = new_bpm;
		total_time_s = 0.0f;
		last_tick = 0.0f;
		tick = 0;
		time_at_last_tempo_change = 0.0f;
		tick_at_last_tempo_change = 0.0f;
	}

	void incrementTick(float sampleTime, bool tempoChanged)
	{
		// this is called just before the first event of the tick is called.  So the resulting "tick"
		// is used for this sample's timestamp.  We want the very first event to be at tick=0, so
		// increment the "total_time_s" after computing the tick.

		// keep track of the time and tick the last time the temp changed.  If we naively
		// increment the tick one "samples worth" at a time, we risk accumulating error.
		// So, optimize by making a snapshot at each tempo change and then adding the elapsed time
		// since the tempo change as a chunk rather than as a series of small increments.
		if (tempoChanged)
		{
			time_at_last_tempo_change = total_time_s;
			tick_at_last_tempo_change = last_tick;
		}
		// PPQ = ticks/beat;  BPM = beat/minute;
		double incremental_tick = (total_time_s - time_at_last_tempo_change) * bpm / SEC_PER_MINUTE * MIDI_FILE_PPQ;
		last_tick = incremental_tick + tick_at_last_tempo_change;
		tick = std::round(last_tick);

		total_time_s += sampleTime;
	}
};

struct MIDIRecorder : Module
{
	enum ParamId
	{
		RUN_PARAM,
		PARAMS_LEN
	};
	enum InputId
	{
		BPM_INPUT,
		RUN_INPUT,
		T1_PITCH_INPUT,
		T1_GATE_INPUT,
		T1_VEL_INPUT,
		T1_AFT_INPUT,
		T1_PW_INPUT,
		T1_MW_INPUT,
		T2_PITCH_INPUT,
		T2_GATE_INPUT,
		T2_VEL_INPUT,
		T2_AFT_INPUT,
		T2_PW_INPUT,
		T2_MW_INPUT,
		T3_PITCH_INPUT,
		T3_GATE_INPUT,
		T3_VEL_INPUT,
		T3_AFT_INPUT,
		T3_PW_INPUT,
		T3_MW_INPUT,
		T4_PITCH_INPUT,
		T4_GATE_INPUT,
		T4_VEL_INPUT,
		T4_AFT_INPUT,
		T4_PW_INPUT,
		T4_MW_INPUT,
		T5_PITCH_INPUT,
		T5_GATE_INPUT,
		T5_VEL_INPUT,
		T5_AFT_INPUT,
		T5_PW_INPUT,
		T5_MW_INPUT,
		T6_PITCH_INPUT,
		T6_GATE_INPUT,
		T6_VEL_INPUT,
		T6_AFT_INPUT,
		T6_PW_INPUT,
		T6_MW_INPUT,
		T7_PITCH_INPUT,
		T7_GATE_INPUT,
		T7_VEL_INPUT,
		T7_AFT_INPUT,
		T7_PW_INPUT,
		T7_MW_INPUT,
		T8_PITCH_INPUT,
		T8_GATE_INPUT,
		T8_VEL_INPUT,
		T8_AFT_INPUT,
		T8_PW_INPUT,
		T8_MW_INPUT,
		T9_PITCH_INPUT,
		T9_GATE_INPUT,
		T9_VEL_INPUT,
		T9_AFT_INPUT,
		T9_PW_INPUT,
		T9_MW_INPUT,
		T10_PITCH_INPUT,
		T10_GATE_INPUT,
		T10_VEL_INPUT,
		T10_AFT_INPUT,
		T10_PW_INPUT,
		T10_MW_INPUT,
		INPUTS_LEN
	};

	// range of one row of input:
	const InputId T1_FIRST_COLUMN = T1_PITCH_INPUT;
	const InputId T1_LAST_COLUMN = T1_MW_INPUT;

	enum OutputId
	{
		OUTPUTS_LEN
	};
	enum LightId
	{
		REC_LIGHT,
		LIGHTS_LEN
	};

	MIDIClock clock;
	bool running;
	bool rec_clicked;
	bool first_note_seen;
	std::string path_directory;
	std::string path_basename;
	std::string path_ext;

	// persisted state:
	std::string path;
	bool increment_path;
	bool align_to_first_note;

	smf::MidiFile midiFile;
	MidiCollector MidiCollectors[NUM_TRACKS] = {
		MidiCollector(midiFile, 0, clock.tick),
		MidiCollector(midiFile, 1, clock.tick),
		MidiCollector(midiFile, 2, clock.tick),
		MidiCollector(midiFile, 3, clock.tick),
		MidiCollector(midiFile, 4, clock.tick),
		MidiCollector(midiFile, 5, clock.tick),
		MidiCollector(midiFile, 6, clock.tick),
		MidiCollector(midiFile, 7, clock.tick),
		MidiCollector(midiFile, 8, clock.tick),
		MidiCollector(midiFile, 9, clock.tick),
	};

	dsp::Timer rateLimiterTimer;

	MIDIRecorder()
	{
		onReset();

		config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
		configInput(BPM_INPUT, "Tempo/BPM");
		configInput(RUN_INPUT, "Start/Stop Gate");
		configSwitch(RUN_PARAM, 0.0f, 1.0f, 0.0f, "Start/Stop");

		int i, t;
		for (t = 0; t < NUM_TRACKS; t++)
		{
			for (i = 0; i < NUM_PER_TRACK_INPUTS; i++)
			{
				auto e = T1_PITCH_INPUT + t * NUM_PER_TRACK_INPUTS + i;
				const char *paramName[NUM_PER_TRACK_INPUTS] = {"Note pitch (V/oct)", "Note gate", "Note velocity", "Aftertouch", "Pitchbend", "Modwheel"};
				configInput(e, string::f("Track %d %s", t + 1, paramName[i]));
			}
		}
	}

	void setPath(std::string new_path)
	{
		if (this->path == new_path)
			return;

		if (new_path == "")
		{
			this->path = "";
			path_directory = "";
			path_basename = "";
			return;
		}

		path_directory = system::getDirectory(new_path);
		path_basename = system::getStem(new_path);
		path_ext = system::getExtension(new_path);

		if (path_basename == "")
		{
			path = "";
			return;
		}
		path = path_directory + "/" + path_basename + ".mid";
	}

	void clearRecording()
	{
		midiFile.clear();
		first_note_seen = false;
	}

	void onReset() override
	{
		clock.reset(120.0f);
		running = false;
		path = "";
		path_directory = "";
		path_basename = "";
		increment_path = true;
		align_to_first_note = true;
		rec_clicked = false;
		first_note_seen = false;

		clearRecording();
	}

	json_t *dataToJson() override
	{
		json_t *rootJ = json_object();
		json_object_set_new(rootJ, "path", json_string(path.c_str()));
		json_object_set_new(rootJ, "increment_path", json_boolean(increment_path));
		json_object_set_new(rootJ, "align_to_first_note", json_boolean(align_to_first_note));
		return rootJ;
	}

	void dataFromJson(json_t *rootJ) override
	{
		json_t *pathJ = json_object_get(rootJ, "path");
		if (pathJ)
			setPath(json_string_value(pathJ));

		json_t *increment_pathJ = json_object_get(rootJ, "increment_path");
		if (increment_pathJ)
			increment_path = json_boolean_value(increment_pathJ);

		json_t *align_to_first_noteJ = json_object_get(rootJ, "align_to_first_note");
		if (align_to_first_noteJ)
			align_to_first_note = json_boolean_value(align_to_first_noteJ);
	}

	bool trackIsActive(const int track)
	{
		const auto FIRST_INPUT = T1_FIRST_COLUMN + track * NUM_PER_TRACK_INPUTS;
		const auto LAST_INPUT = T1_LAST_COLUMN + track * NUM_PER_TRACK_INPUTS;
		for (int i = FIRST_INPUT; i <= LAST_INPUT; i++)
		{
			if (inputs[i].isConnected())
			{
				return true;
			}
		}
		return false;
	}

	void processMidiTrack(const ProcessArgs &args, const int track, const bool tempoChanged)
	{
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
		const double rateLimiterPeriod = 1 / 200.f;
		bool rateLimiterTriggered = (rateLimiterTimer.process(args.sampleTime) >= rateLimiterPeriod);
		if (rateLimiterTriggered)
			rateLimiterTimer.time -= rateLimiterPeriod;

		MidiCollectors[track].setFrame(args.frame);

		if (align_to_first_note && !first_note_seen)
		{
			// any note gates in this frame?
			for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++)
			{
				bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
				if (gate)
				{
					first_note_seen = true;
					// reset the clock so that first event is at tick=0:
					clock.reset(clock.bpm);
					break;
				}
			}
			// no gates seen yet - nothing to record
			return;
		}

		if (tempoChanged)
		{
			midiFile.addTempo(track, clock.tick, clock.bpm);
		}

		if (rateLimiterTriggered)
		{
			// The RACK CV-MIDI code doesn't guard each input with isConnected(), but I've noticed
			// noise on test recordings (e.g. a stray "polyphonic aftertouch" even if nothing
			// connected to those inputs).  So play it safe :
			if (inputs[PW_INPUT].isConnected())
			{
				int pw = (int)std::round((inputs[PW_INPUT].getVoltage() + 5.f) / 10.f * 0x4000);
				pw = clamp(pw, 0, 0x3fff);
				MidiCollectors[track].setPitchWheel(pw);
			}
			if (inputs[MW_INPUT].isConnected())
			{
				int mw = (int)std::round(inputs[MW_INPUT].getVoltage() / 10.f * 127);
				mw = clamp(mw, 0, 127);
				MidiCollectors[track].setModWheel(mw);
			}
		}

		for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++)
		{
			if (inputs[VEL_INPUT].isConnected())
			{
				int vel = (int)std::round(inputs[VEL_INPUT].getNormalPolyVoltage(10.f * 100 / 127, c) / 10.f * 127);
				vel = clamp(vel, 0, 127);
				MidiCollectors[track].setVelocity(vel, c);
			}

			int note = 60;
			if (inputs[PITCH_INPUT].isConnected())
			{
				note = (int)std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
				note = clamp(note, 0, 127);
			}

			if (inputs[GATE_INPUT].isConnected())
			{
				bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
				MidiCollectors[track].setNoteGate(note, gate, c);
			}

			if (inputs[AFT_INPUT].isConnected())
			{
				int aft = (int)std::round(inputs[AFT_INPUT].getPolyVoltage(c) / 10.f * 127);
				aft = clamp(aft, 0, 127);
				MidiCollectors[track].setKeyPressure(aft, c);
			}
		}
	}

	void
	processMidi(const ProcessArgs &args)
	{
		double new_bpm = getBPM();
		bool tempoChanged = new_bpm != clock.bpm;
		clock.bpm = new_bpm;

		clock.incrementTick(args.sampleTime, tempoChanged);

		for (int i = 0; i < NUM_TRACKS; i++)
		{
			if (trackIsActive(i))
			{
				processMidiTrack(args, i, tempoChanged);
			}
		}
	}

	void startRecording(const ProcessArgs &args)
	{
		if (path == "")
		{
			INFO("ERROR: No Path in startRecording");
			return;
		}

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

		clock.bpm = getBPM();
		for (int t = 0; t < num_tracks; t++)
		{
			if (trackIsActive(t))
			{
				midiFile.addTempo(t, 0, clock.bpm);
			}
		}

		clock.reset(clock.bpm);
		INFO("Start Recording... BPM: %f num_tracks: %d", clock.bpm, num_tracks);
		running = true;
	}

	void stopRecording(const ProcessArgs &args)
	{
		running = false;
		int num_events = 0;
		for (int t = 0; t < midiFile.getNumTracks(); t++)
		{
			if (midiFile[t].size() <= 2)
			{
				// unused track - just the tempo info
				midiFile[t].clear();
			}
			else
			{
				num_events += midiFile[t].size();
			}
		}
		std::string newPath = path;
		if (increment_path)
		{
			std::string extension = "mid";
			for (int i = 0; i <= 999; i++)
			{
				newPath = path_directory + "/" + path_basename;
				if (i > 0)
					newPath += string::f("-%03d", i);
				newPath += "." + extension;
				// Skip if file exists
				if (!system::isFile(newPath))
					break;
			}
		}

		INFO("Stop Recording.  total_time_s=%f ticks=%d events=%d.  Writing to %s", clock.total_time_s, clock.tick, num_events, newPath.c_str());
		midiFile.write(newPath);
	}

	double getBPM()
	{
		// From Impromptu's Clocked : bpm = 120*2^V
		double new_bpm;
		if (inputs[BPM_INPUT].isConnected())
		{
			new_bpm = 120.0f * std::pow(2.0f, inputs[BPM_INPUT].getVoltage());
		}
		else
		{
			new_bpm = 120.0f; // default
		}
		return new_bpm;
	}

	void process(const ProcessArgs &args) override
	{
		auto was_running = running;
		int run_requested;
		// Run button:
		if (inputs[RUN_INPUT].isConnected())
		{
			run_requested = inputs[RUN_INPUT].getVoltage() > 0.0f;
		}
		else
		{
			//			run_requested = params[RUN_PARAM].getValue() > 0.0f;
			run_requested = rec_clicked;
		}

		if (run_requested)
		{
			if (!was_running)
			{
				startRecording(args);
			}
			processMidi(args);
		}
		else
		{
			if (was_running)
			{
				stopRecording(args);
			}
		}
		if (!running)
		{
			// while running, the recorder processes BPM changes; when not recordning, we need to do it
			// here so that we have up to date display changes
			clock.bpm = getBPM();
		}
		lights[REC_LIGHT].setBrightness(running);
	}
};

static const char MIDI_FILTERS[] = "MIDI files (.mid):mid";

static void selectPath(Module *m)
{
	MIDIRecorder *module = dynamic_cast<MIDIRecorder *>(m);
	std::string dir;
	std::string filename;

	if (module->path != "")
	{
		dir = system::getDirectory(module->path);
		filename = system::getFilename(module->path);
	}
	else
	{
		dir = asset::user("recordings");
		system::createDirectory(dir);
		filename = "Untitled";
	}

	osdialog_filters *filters = osdialog_filters_parse(MIDI_FILTERS);
	DEFER({
		osdialog_filters_free(filters);
	});

	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
	if (path)
	{
		module->setPath(path);
		free(path);
	}
}

struct RecButton : SvgButton
{
	MIDIRecorder *module;

	RecButton()
	{
		addFrame(Svg::load(asset::plugin(pluginInstance, "res/rec_button.svg")));
	}

	void onDragStart(const event::DragStart &e) override
	{
		if (e.button == GLFW_MOUSE_BUTTON_LEFT)
		{
			if (module && module->path == "")
			{
				selectPath(module);
			}
			if (module && module->path != "")
			{
				module->rec_clicked = !module->rec_clicked;
			}
		}

		SvgButton::onDragStart(e);
	}
};

struct RecLight : RedLight
{
	RecLight()
	{
		bgColor = nvgRGB(0x66, 0x66, 0x66);
		box.size = mm2px(Vec(9.0, 9.00));
	}
};

#define FIRST_X 10.0
#define FIRST_Y 19.0
#define SPACING 11.0

struct MIDIRecorderWidget : ModuleWidget
{
	MIDIRecorderWidget(MIDIRecorder *module)
	{
		setModule(module);
		setPanel(createPanel(asset::plugin(pluginInstance, "res/MIDIRecorder.svg")));

		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		RecButton *recButton = createWidgetCentered<RecButton>(mm2px(Vec(FIRST_X, FIRST_Y + 4 * SPACING)));
		recButton->module = module;
		addChild(recButton);

		addChild(createLightCentered<RecLight>(mm2px(Vec(FIRST_X, FIRST_Y + 4 * SPACING)), module, MIDIRecorder::REC_LIGHT));

		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X, FIRST_Y + 7 * SPACING)), module, MIDIRecorder::BPM_INPUT));
		addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X, FIRST_Y + 9 * SPACING)), module, MIDIRecorder::RUN_INPUT));

		int t, i;
		for (t = 0; t < NUM_TRACKS; t++)
		{
			auto y = FIRST_Y + t * SPACING;
			for (i = 0; i < NUM_PER_TRACK_INPUTS; i++)
			{
				auto e = MIDIRecorder::T1_PITCH_INPUT + t * NUM_PER_TRACK_INPUTS + i;
				addInput(createInputCentered<PJ301MPort>(mm2px(Vec(FIRST_X + SPACING + i * SPACING, y)), module, e));
			}
		}

		auto bpmDisplay = new BPMDisplayWidget(module ? &module->clock.bpm : NULL);
		bpmDisplay->box.size = Vec(30, 10);
		bpmDisplay->box.pos = mm2px(Vec(FIRST_X, FIRST_Y + 5 * SPACING + SPACING / 2).minus(bpmDisplay->box.size.div(2)));
		addChild(bpmDisplay);
	}

	void appendContextMenu(Menu *menu) override
	{
		MIDIRecorder *module = dynamic_cast<MIDIRecorder *>(this->module);

		menu->addChild(new MenuSeparator);
		menu->addChild(createMenuLabel("Output file"));

		std::string path = string::ellipsizePrefix(module->path, 30);
		menu->addChild(createMenuItem((path != "") ? path : "Select...", "",
									  [=]()
									  { selectPath(module); }));

		menu->addChild(createBoolPtrMenuItem("Append -001, -002, etc.", "", &module->increment_path));
		menu->addChild(createBoolPtrMenuItem("Start at first note gate", "", &module->align_to_first_note));
	}
};

Model *modelMIDIRecorder = createModel<MIDIRecorder, MIDIRecorderWidget>("MIDIRecorder");
