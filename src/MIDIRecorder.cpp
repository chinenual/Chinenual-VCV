#include <osdialog.h>

#include "CVRange.hpp"
#include "MIDIBuffer.hpp"
#include "MIDIRecorderBase.hpp"
#include "MidiFile.h"
#include "plugin.hpp"

namespace Chinenual {
namespace MIDIRecorder {

#define MIDI_FILE_PPQ 960
#define SEC_PER_MINUTE 60

    struct MidiCollector : dsp::MidiGenerator<PORT_MAX_CHANNELS> {
        MIDIBuffer& midiBuffer;
        int track;
        int& tick;

        MidiCollector(MIDIBuffer& midiBuffer, int track, int& tick)
            : midiBuffer(midiBuffer)
            , track(track)
            , tick(tick)
        {
        }

        void onMessage(const midi::Message& message) override
        {
            /// do something
            // convert to the smf library's classes:
            smf::MidiMessage smfMsg(message.bytes);
            smf::MidiEvent smfEvent(tick, track, smfMsg);
            midiBuffer.appendEvent(track, smfEvent);
        }

        void reset() { MidiGenerator::reset(); }
    };

    struct BPMDisplayWidget : TransparentWidget {
        std::shared_ptr<Font> font;
        std::string fontPath;
        char displayStr[16];
        double* bpmPtr;

        BPMDisplayWidget(double* bpm)
        {
            bpmPtr = bpm;
            fontPath = std::string(
                asset::plugin(pluginInstance, "res/fonts/DSEG14Modern-BoldItalic.ttf"));
        }

        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1) {
                if (!(font = APP->window->loadFont(fontPath))) {
                    return;
                }
                nvgFontSize(args.vg, 17);
                nvgFontFaceId(args.vg, font->handle);

                Vec textPos = Vec(6, 24);

                nvgFillColor(args.vg, ledTextColor);

                unsigned int bpm = bpmPtr ? std::round(*bpmPtr) : 120;
                snprintf(displayStr, 16, "%3u", bpm);

                nvgTextAlign(args.vg, NVG_ALIGN_RIGHT | NVG_ALIGN_BOTTOM);
                nvgText(args.vg, textPos.x, textPos.y, displayStr, NULL);
            }
        }
    };

    static void selectPath(Module* module);

    struct MIDIClock {
        int tick;
        double lastTick; // unrounded
        double timeAtLastTempoChange;
        double tickAtLastTempoChange;
        double totalTimeSecs;
        double bpm;

        void reset(double newBpm)
        {
            bpm = newBpm;
            totalTimeSecs = 0.0f;
            lastTick = 0.0f;
            tick = 0;
            timeAtLastTempoChange = 0.0f;
            tickAtLastTempoChange = 0.0f;
        }

        void incrementTick(float sampleTime, bool tempoChanged)
        {
            // this is called just before the first event of the tick is called.  So
            // the resulting "tick" is used for this sample's timestamp.  We want the
            // very first event to be at tick=0, so increment the "totalTimeSecs" after
            // computing the tick.

            // keep track of the time and tick the last time the temp changed.  If we
            // naively increment the tick one "samples worth" at a time, we risk
            // accumulating error. So, optimize by making a snapshot at each tempo
            // change and then adding the elapsed time since the tempo change as a
            // chunk rather than as a series of small increments.
            if (tempoChanged) {
                timeAtLastTempoChange = totalTimeSecs;
                tickAtLastTempoChange = lastTick;
            }
            // PPQ = ticks/beat;  BPM = beat/minute;
            double incrementalTick = (totalTimeSecs - timeAtLastTempoChange) * bpm / SEC_PER_MINUTE * MIDI_FILE_PPQ;
            lastTick = incrementalTick + tickAtLastTempoChange;
            tick = std::round(lastTick);

            totalTimeSecs += sampleTime;
        }
    };

    struct MIDIRecorder : MIDIRecorderBase<6> {
        MasterToExpanderMessage master_to_expander_message_a;
        MasterToExpanderMessage master_to_expander_message_b;

        enum ParamId { RUN_PARAM,
            PARAMS_LEN };
        enum InputId {
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

        enum OutputId {
            RUNNING_OUTPUT,
            OUTPUTS_LEN
        };
        enum LightId {
            REC_LIGHT,
            RUNNING_LIGHT,
            LIGHTS_LEN
        };

        MIDIClock clock;
        bool running;
        bool recClicked;
        bool firstNoteSeen;
        std::string pathDirectory;
        std::string pathBasename;
        std::string pathExt;

        // persisted state:
        std::string path;
        bool incrementPath;
        bool alignToFirstNote;
        CVRangeIndex cvConfigVel;
        CVRangeIndex cvConfigAft;
        CVRangeIndex cvConfigPw;
        CVRangeIndex cvConfigMw;
        bool mwIs14bit;

        smf::MidiFile midiFile;
        MIDIBuffer midiBuffer;
        MidiCollector midiCollectors[NUM_TRACKS] = {
            MidiCollector(midiBuffer, 0, clock.tick),
            MidiCollector(midiBuffer, 1, clock.tick),
            MidiCollector(midiBuffer, 2, clock.tick),
            MidiCollector(midiBuffer, 3, clock.tick),
            MidiCollector(midiBuffer, 4, clock.tick),
            MidiCollector(midiBuffer, 5, clock.tick),
            MidiCollector(midiBuffer, 6, clock.tick),
            MidiCollector(midiBuffer, 7, clock.tick),
            MidiCollector(midiBuffer, 8, clock.tick),
            MidiCollector(midiBuffer, 9, clock.tick),
        };

        MIDIRecorder()
            : MIDIRecorderBase(T1_PITCH_INPUT)
            , midiBuffer(midiFile)
        {
            rightExpander.consumerMessage = &master_to_expander_message_a;
            rightExpander.producerMessage = &master_to_expander_message_a;

            onReset();

            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
            configInput(BPM_INPUT, "Tempo/BPM");
            configInput(RUN_INPUT, "Start/Stop Gate");
            configOutput(RUNNING_OUTPUT, "Is Actively Recording Gate");
            configSwitch(RUN_PARAM, 0.0f, 1.0f, 0.0f, "Start/Stop");

            int i, t;
            for (t = 0; t < NUM_TRACKS; t++) {
                for (i = 0; i < COLS_PER_TRACK; i++) {
                    auto e = T1_PITCH_INPUT + t * COLS_PER_TRACK + i;
                    const char* paramName[] = {
                        "Note pitch (V/oct)", "Note gate", "Note velocity",
                        "Aftertouch", "Pitchbend", "Modwheel"
                    };
                    configInput(e, string::f("Track %d %s", t + 1, paramName[i]));
                }
            }
        }

        void setPath(std::string new_path)
        {
            if (this->path == new_path)
                return;

            if (new_path == "") {
                this->path = "";
                pathDirectory = "";
                pathBasename = "";
                return;
            }

            pathDirectory = system::getDirectory(new_path);
            pathBasename = system::getStem(new_path);
            pathExt = system::getExtension(new_path);

            if (pathBasename == "") {
                path = "";
                return;
            }
            path = pathDirectory + "/" + pathBasename + ".mid";
        }

        void clearRecording()
        {
            midiFile.clear();
            firstNoteSeen = false;
        }

        void onReset() override
        {
            MIDIRecorderBase::onReset();

            clock.reset(120.0f);
            running = false;
            path = "";
            pathDirectory = "";
            pathBasename = "";
            incrementPath = true;
            alignToFirstNote = true;
            recClicked = false;
            firstNoteSeen = false;

            cvConfigVel = CV_RANGE_0_10;
            cvConfigAft = CV_RANGE_0_10;
            cvConfigPw = CV_RANGE_n5_5;
            cvConfigMw = CV_RANGE_0_10;
            mwIs14bit = false;

            clearRecording();
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            json_object_set_new(rootJ, "path", json_string(path.c_str()));
            json_object_set_new(rootJ, "incrementPath", json_boolean(incrementPath));
            json_object_set_new(rootJ, "alignToFirstNote",
                json_boolean(alignToFirstNote));
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            json_t* pathJ = json_object_get(rootJ, "path");
            if (pathJ)
                setPath(json_string_value(pathJ));

            json_t* incrementPathJ = json_object_get(rootJ, "incrementPath");
            if (incrementPathJ)
                incrementPath = json_boolean_value(incrementPathJ);

            json_t* alignToFirstNoteJ = json_object_get(rootJ, "alignToFirstNote");
            if (alignToFirstNoteJ)
                alignToFirstNote = json_boolean_value(alignToFirstNoteJ);
        }

        bool trackIsActive(const int track) override
        {
            if (MIDIRecorderBase::trackIsActive(track)) {
                return true;
            }
            // check expanders:
            Module* m = rightExpander.module;
            while (m) {
                if (m->model == modelMIDIRecorderCC) {
                    auto consumerMessage = (ExpanderToMasterMessage*)m->leftExpander.consumerMessage;
                    if (consumerMessage->active[track]) {
                        return true;
                    }
                } else {
                    break;
                }
                m = m->rightExpander.module;
            }
            return false;
        }

        bool isActivelyRecording()
        {
            return running && ((!alignToFirstNote) || firstNoteSeen);
        }

        void processMidiTrack(const ProcessArgs& args, const int track,
            const bool tempoChanged)
        {
            const auto PITCH_INPUT = T1_PITCH_INPUT + track * COLS_PER_TRACK;
            const auto GATE_INPUT = T1_GATE_INPUT + track * COLS_PER_TRACK;
            const auto VEL_INPUT = T1_VEL_INPUT + track * COLS_PER_TRACK;
            const auto AFT_INPUT = T1_AFT_INPUT + track * COLS_PER_TRACK;
            const auto PW_INPUT = T1_PW_INPUT + track * COLS_PER_TRACK;
            const auto MW_INPUT = T1_MW_INPUT + track * COLS_PER_TRACK;

            // MIDI specific processing adapted from VCV Core's CV_MIDI.cpp:

            midiCollectors[track].setFrame(args.frame);

            if (alignToFirstNote && !firstNoteSeen) {
                // any note gates in this frame?
                for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                    bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
                    if (gate) {
                        firstNoteSeen = true;
                        // reset the clock so that first event is at tick=0:
                        clock.reset(clock.bpm);
#ifdef SDTDEBUG
                        INFO("SAW FIRST NOTE");
#endif
                        break;
                    }
                }
                // no gates seen yet - nothing to record
                return;
            }

            if (tempoChanged) {
                smf::MidiMessage tempoMsg;
                tempoMsg.makeTempo(clock.bpm);
                smf::MidiEvent tempoEvent(clock.tick, track, tempoMsg);
                midiBuffer.appendEvent(track, tempoEvent);
            }

            {
                // check expanders - they have indepentent rate limiters, so check every frame
                Module* m = rightExpander.module;
                while (m) {
                    if (m->model == modelMIDIRecorderCC) {
                        auto consumerMessage = (ExpanderToMasterMessage*)m->leftExpander.consumerMessage;
                        for (auto&& msg : consumerMessage->msgs[track]) {
#if 0
                            INFO("data from expander: %d %2x", track, msg.getCommandByte());
#endif
                            /// midiCollectors[track].setCc(msg.getControllerValue(), msg.getControllerNumber());
                            smf::MidiEvent event(clock.tick, track, msg);
                            midiBuffer.appendEvent(track, event);
                        }
                    } else {
                        break;
                    }
                    m = m->rightExpander.module;
                }
            }

            if (rateLimiterTriggered) {
                // The RACK CV-MIDI code doesn't guard each input with isConnected(),
                // but I've noticed noise on test recordings (e.g. a stray "polyphonic
                // aftertouch" even if nothing connected to those inputs).  So play it
                // safe :
                if (inputs[PW_INPUT].isConnected()) {
                    // PW is always 14bit:
                    int pw = CVRanges[cvConfigPw].to14bit(inputs[PW_INPUT].getVoltage());
                    midiCollectors[track].setPitchWheel(pw);
                }
                if (inputs[MW_INPUT].isConnected()) {
                    if (mwIs14bit) {
                        int lsb, msb;
                        int val = CVRanges[cvConfigMw].to14bit(inputs[MW_INPUT].getVoltage());
                        CVRanges[cvConfigMw].split14bit(val, msb, lsb);
                        midiCollectors[track].setCc(1, msb);
                        midiCollectors[track].setCc(33, lsb);
                    } else {
                        int mw = CVRanges[cvConfigMw].to7bit(inputs[MW_INPUT].getVoltage());
                        midiCollectors[track].setModWheel(mw);
                    }
                }
            }

            for (int c = 0; c < inputs[PITCH_INPUT].getChannels(); c++) {
                if (inputs[VEL_INPUT].isConnected()) {
                    int vel = CVRanges[cvConfigVel].to7bit(inputs[VEL_INPUT].getPolyVoltage(c));
                    midiCollectors[track].setVelocity(vel, c);
                } else {
                    // default is 100
                    midiCollectors[track].setVelocity(100, c);
                }

                int note = 60;
                if (inputs[PITCH_INPUT].isConnected()) {
                    note = (int)std::round(inputs[PITCH_INPUT].getVoltage(c) * 12.f + 60.f);
                    note = clamp(note, 0, 127);
                }

                if (inputs[GATE_INPUT].isConnected()) {
                    bool gate = inputs[GATE_INPUT].getPolyVoltage(c) >= 1.f;
                    midiCollectors[track].setNoteGate(note, gate, c);
                }

                if (inputs[AFT_INPUT].isConnected()) {
                    int aft = CVRanges[cvConfigAft].to7bit(inputs[AFT_INPUT].getPolyVoltage(c));
                    midiCollectors[track].setKeyPressure(aft, c);
                }
            }
        }

        void processMidi(const ProcessArgs& args)
        {
            double newBpm = getBPM();
            bool tempoChanged = newBpm != clock.bpm;
            clock.bpm = newBpm;

            clock.incrementTick(args.sampleTime, tempoChanged);

#if 0
            INFO("ACTIVE: %d %d %d %d %d %d %d %d %d %d", trackIsActive(0), trackIsActive(1), trackIsActive(2), trackIsActive(3), trackIsActive(4), trackIsActive(5), trackIsActive(6), trackIsActive(7), trackIsActive(8), trackIsActive(9));
#endif
            for (int t = 0; t < NUM_TRACKS; t++) {
                if (trackIsActive(t)) {
                    processMidiTrack(args, t, tempoChanged);
                }
            }
        }

        void startRecording(const ProcessArgs& args)
        {
            midiBuffer.stop();

            if (path == "") {
                INFO("ERROR: No Path in startRecording");
                return;
            }

            clearRecording();
            // max track where inputs are connected?
            int num_tracks = NUM_TRACKS;

            midiFile.addTracks(num_tracks);

            midiFile.setTPQ(MIDI_FILE_PPQ);
            midiFile.makeAbsoluteTicks();

            clock.bpm = getBPM();
            for (int t = 0; t < num_tracks; t++) {
                if (trackIsActive(t)) {
                    midiFile.addTempo(t, 0, clock.bpm);
                }
            }
            midiBuffer.start();

            clock.reset(clock.bpm);
            INFO("Start Recording... BPM: %f num_tracks: %d", clock.bpm, num_tracks);
            running = true;
        }

        void stopRecording(const ProcessArgs& args)
        {
            midiBuffer.stop();

            running = false;
            int numEvents = 0;
            for (int t = 0; t < midiFile.getNumTracks(); t++) {
                if (midiFile[t].size() <= 2) {
                    // unused track - just the tempo info
                    midiFile[t].clear();
                } else {
                    numEvents += midiFile[t].size();
                }
            }
            std::string newPath = path;
            if (incrementPath) {
                std::string extension = "mid";
                for (int i = 0; i <= 999; i++) {
                    newPath = pathDirectory + "/" + pathBasename;
                    if (i > 0)
                        newPath += string::f("-%03d", i);
                    newPath += "." + extension;
                    // Skip if file exists
                    if (!system::isFile(newPath))
                        break;
                }
            }

            INFO("Stop Recording.  totalTimeSecs=%f ticks=%d events=%d.  Writing to %s",
                clock.totalTimeSecs, clock.tick, numEvents, newPath.c_str());
            midiFile.write(newPath);

#ifdef SDTDEBUG
            auto dbgPath = newPath + ".txt";
            midiFile.writeBinascWithComments(dbgPath);
#endif
            // free memory:
            clearRecording();
        }

        double getBPM()
        {
            // From Impromptu's Clocked : bpm = 120*2^V
            double newBpm;
            if (inputs[BPM_INPUT].isConnected()) {
                newBpm = 120.0f * std::pow(2.0f, inputs[BPM_INPUT].getVoltage());
            } else {
                newBpm = 120.0f; // default
            }
            return newBpm;
        }

        void process(const ProcessArgs& args) override
        {
            MIDIRecorderBase::process(args);

            auto wasRunning = running;
            int runRequested;
            // Run button:
            if (inputs[RUN_INPUT].isConnected()) {
                runRequested = inputs[RUN_INPUT].getVoltage() > 0.0f;
            } else {
                //			runRequested = params[RUN_PARAM].getValue() >
                // 0.0f;
                runRequested = recClicked;
            }

            if (runRequested) {
                if (!wasRunning) {
                    startRecording(args);
                }
                processMidi(args);
            } else {
                if (wasRunning) {
                    stopRecording(args);
                }
            }
            if (!running) {
                // while running, the recorder processes BPM changes; when not
                // recordning, we need to do it here so that we have up to date display
                // changes
                clock.bpm = getBPM();
            }
            {
                // tell any expanders that we're expecting them to send us data
                auto producerMessage = (MasterToExpanderMessage*)rightExpander.producerMessage;
                producerMessage->isRecording = running;
                rightExpander.requestMessageFlip();
            }
            lights[REC_LIGHT].setBrightness(running ? 1.0f : 0.0f);
            outputs[RUNNING_OUTPUT].setVoltage(isActivelyRecording() ? 10.0f : 0.0f);
            lights[RUNNING_LIGHT].setBrightness(isActivelyRecording() ? 1.0f : 0.0f);
            // INFO("isactivelyrecording: %d %d %d %d", isActivelyRecording(), running, alignToFirstNote, firstNoteSeen);
        }
    };

    static const char MIDI_FILTERS[] = "MIDI files (.mid):mid";

    static void selectPath(Module* m)
    {
        MIDIRecorder* module = dynamic_cast<MIDIRecorder*>(m);
        std::string dir;
        std::string filename;

        if (module->path != "") {
            dir = system::getDirectory(module->path);
            filename = system::getFilename(module->path);
        } else {
            dir = asset::user("recordings");
            system::createDirectory(dir);
            filename = "Untitled";
        }

        osdialog_filters* filters = osdialog_filters_parse(MIDI_FILTERS);
        DEFER({ osdialog_filters_free(filters); });

        char* path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), filename.c_str(), filters);
        if (path) {
            module->setPath(path);
            free(path);
        }
    }

    struct RecButton : SvgButton {
        MIDIRecorder* module;

        RecButton()
        {
            addFrame(Svg::load(asset::plugin(pluginInstance, "res/rec_button.svg")));
        }

        void onDragStart(const event::DragStart& e) override
        {
            if (e.button == GLFW_MOUSE_BUTTON_LEFT) {
                if (module && module->path == "") {
                    selectPath(module);
                }
                if (module && module->path != "") {
                    module->recClicked = !module->recClicked;
                }
            }

            SvgButton::onDragStart(e);
        }
    };

    struct RecLight : RedLight {
        RecLight()
        {
            bgColor = nvgRGB(0x66, 0x66, 0x66);
            box.size = mm2px(Vec(11.0, 11.00));
        }
    };

#define FIRST_X 10.0
#define FIRST_Y 20.0
#define SPACING_X 10.0
#define SPACING_Y 10.5
#define LED_OFFSET_X 3.0
#define LED_OFFSET_Y -9.5
#define BUTTON_OFFSET_X -1.0
#define BUTTON_OFFSET_Y -5.0
// shift the entire first column over a nudge to make room for a large button, but keep
// all the ports and lights aligned with the big button:
#define FIRST_COL_X (FIRST_X + BUTTON_OFFSET_X)

    struct MIDIRecorderWidget : ModuleWidget {
        MIDIRecorderWidget(MIDIRecorder* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/MIDIRecorder.svg")));

            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            RecButton* recButton = createWidgetCentered<RecButton>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 2 * SPACING_Y + BUTTON_OFFSET_Y)));
            recButton->module = module;
            addChild(recButton);

            addChild(createLightCentered<RecLight>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 2 * SPACING_Y + BUTTON_OFFSET_Y)), module,
                MIDIRecorder::REC_LIGHT));

            addChild(createLightCentered<MediumLight<RedLight>>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 6 * SPACING_Y)), module,
                MIDIRecorder::RUNNING_LIGHT));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 3 * SPACING_Y)), module,
                MIDIRecorder::RUN_INPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 5 * SPACING_Y)), module,
                MIDIRecorder::RUNNING_OUTPUT));

            int t, i;
            for (t = 0; t < NUM_TRACKS; t++) {
                auto y = FIRST_Y + t * SPACING_Y;
                for (i = 0; i < MIDIRecorder::COLS_PER_TRACK; i++) {
                    auto e = MIDIRecorder::T1_PITCH_INPUT + t * MIDIRecorder::COLS_PER_TRACK + i;
                    addInput(createInputCentered<PJ301MPort>(
                        mm2px(Vec(FIRST_X + SPACING_X + i * SPACING_X, y)), module, e));
                }
            }

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_COL_X, FIRST_Y + 9 * SPACING_Y)), module,
                MIDIRecorder::BPM_INPUT));
            auto bpmDisplay = new BPMDisplayWidget(module ? &module->clock.bpm : NULL);
            bpmDisplay->box.size = Vec(30, 10);
            bpmDisplay->box.pos = mm2px(Vec(FIRST_X + LED_OFFSET_X, FIRST_Y + 8 * SPACING_Y + LED_OFFSET_Y));
            addChild(bpmDisplay);
        }

        void appendContextMenu(Menu* menu) override
        {
            MIDIRecorder* module = dynamic_cast<MIDIRecorder*>(this->module);

            menu->addChild(new MenuSeparator);
            menu->addChild(createMenuLabel("Output file"));

            std::string path = string::ellipsizePrefix(module->path, 30);
            menu->addChild(createMenuItem((path != "") ? path : "Select...", "",
                [=]() { selectPath(module); }));

            menu->addChild(createBoolPtrMenuItem("Append -001, -002, etc.", "",
                &module->incrementPath));
            menu->addChild(createBoolPtrMenuItem("Start at first note gate", "",
                &module->alignToFirstNote));

            menu->addChild(createIndexSubmenuItem(
                "VEL Input Range", CVRangeNames,
                [=]() { return module->cvConfigVel; },
                [=](int val) {
                    module->cvConfigVel = (CVRangeIndex)val;
                }));
            menu->addChild(createIndexSubmenuItem(
                "AFT Input Range", CVRangeNames,
                [=]() { return module->cvConfigAft; },
                [=](int val) {
                    module->cvConfigAft = (CVRangeIndex)val;
                }));
            menu->addChild(createIndexSubmenuItem(
                "PW Input Range", CVRangeNames,
                [=]() { return module->cvConfigPw; },
                [=](int val) {
                    module->cvConfigPw = (CVRangeIndex)val;
                }));
            menu->addChild(createIndexSubmenuItem(
                "MW Input Range", CVRangeNames,
                [=]() { return module->cvConfigMw; },
                [=](int val) {
                    module->cvConfigMw = (CVRangeIndex)val;
                }));
            menu->addChild(createBoolMenuItem(
                "MW is 14bit", "", [=]() { return module->mwIs14bit; },
                [=](bool val) { module->mwIs14bit = val; }));
        }
    };

} // namespace MIDIRecorder
} // namespace Chinenual

Model* modelMIDIRecorder = createModel<Chinenual::MIDIRecorder::MIDIRecorder,
    Chinenual::MIDIRecorder::MIDIRecorderWidget>("MIDIRecorder");
