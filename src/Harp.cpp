#include <osdialog.h>

#include "PitchNote.hpp"
#include "CVRange.hpp"
#include "plugin.hpp"

namespace Chinenual {
namespace Harp {
	
    struct Harp : Module {
        enum ParamId {
	    NOTE_RANGE_PARAM,
	    PITCH_CV_RANGE_PARAM,
            PARAMS_LEN
        };
        enum InputId {
            SCALE_INPUT,
	    PITCH_INPUT,
	    GATE_INPUT,
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

        //int noteRange; // number of notes to map into the cv range
	//MIDIRecorder::CVRangeIndex cvConfigPitch;

        const int numOutputChannels = 16;

        std::string rootNote_text;
        std::string playingNote_text;
	std::string debug_text;

	// transient properties:
	bool notePlaying;
        float currNote;
	int currDegree; 
        int currChan;
	    
        Harp()
        {
            onReset();
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

	    configInput(SCALE_INPUT, "Scale");
	    configInput(PITCH_INPUT, "Unquantized pitch");
	    configInput(GATE_INPUT, "Gate");

	    configOutput(PITCH_OUTPUT, "Pitch V/Oct");
            configOutput(GATE_OUTPUT, "Gate");

	    configParam(NOTE_RANGE_PARAM, 2.f, 16.f, 48.f,
			"Number of notes in pitch range");
	    configSwitch(PITCH_CV_RANGE_PARAM,
			0.f,
			MIDIRecorder::CVRangeNames.size()-1,
			MIDIRecorder::CV_RANGE_0_10,
			"Pitch CV voltage range",
			MIDIRecorder::CVRangeNames);
        }

        void onReset() override
        {
	    notePlaying = false;
	    currNote = -1.0f; // out of range so first use will detect note "change"
	    currDegree = 0;
	    rootNote_text = "";
	    playingNote_text = "";
	    debug_text = "";
	    currChan = 0;	    
        }

        void process(const ProcessArgs& args) override
        {
	    float prevNote = currNote;

	    if (inputs[GATE_INPUT].isConnected()) {
		notePlaying = inputs[GATE_INPUT].getVoltage() >= 1.f;
	    } else {
		notePlaying = true;
	    }
	    
	    if (notePlaying) {
		 auto v = inputs[PITCH_INPUT].getVoltage();
		 int cvConfigPitch = (int)params[PITCH_CV_RANGE_PARAM].getValue();
		 // number of voltage "buckets":
		 int noteRange = (int)params[NOTE_RANGE_PARAM].getValue();

		 auto inputCvMin = MIDIRecorder::CVRanges[cvConfigPitch].low;
		 auto inputCvMax = MIDIRecorder::CVRanges[cvConfigPitch].high;
		 int s = std::round(((v - inputCvMin) / (inputCvMax - inputCvMin)) * (noteRange-1));
		 float scaledPitch;
		 int scaleSize = 11; // default to chromatic
		 if (inputs[SCALE_INPUT].isConnected()) {
		     scaleSize = inputs[SCALE_INPUT].getChannels();
		 }
		 int degree = s % scaleSize;
	         int octave = s / scaleSize;
		 if (inputs[SCALE_INPUT].isConnected()) {
		     scaledPitch = inputs[SCALE_INPUT].getPolyVoltage(degree);
		 } else {
		     // default is chromatic with root at C4
		     const float C4 = 0.0f;
		     const float SEMITONE = 1.0f/12.f;
		     scaledPitch = C4 + (degree * SEMITONE);
		 }
		 //INFO("note: r:%d pi:%d s:%d d:%d o:%d cn:%f cd:%d",noteRange, cvConfigPitch,s,degree,octave,currNote,currDegree);
		 currNote = scaledPitch + (octave * 1.f);
		 currDegree = degree + (octave * scaleSize);
	    }

	    if (notePlaying) {
		bool noteChanged = prevNote != currNote;
		if (noteChanged) {
		    outputs[GATE_OUTPUT].setVoltage(0.f, currChan);
		    currChan = (currChan + 1) % numOutputChannels;
		}
		outputs[GATE_OUTPUT].setVoltage(10.f, currChan);
		outputs[PITCH_OUTPUT].setVoltage(currNote, currChan);
		
	    } else {
	        outputs[GATE_OUTPUT].setVoltage(0.f, currChan);
	    }
	    outputs[GATE_OUTPUT].setChannels(numOutputChannels);

            if ((args.frame % 100) == 0) { // throttle
		float root_v;
		float play_v = currNote;
		{
		    if (inputs[SCALE_INPUT].isConnected()) {
			root_v = inputs[SCALE_INPUT].getPolyVoltage(0);
		    } else {
			root_v = 0.0f;
		    }
		    auto n = voltageToPitch(root_v);
		    auto fn = voltageToMicroPitch(root_v);
		    pitchToText(rootNote_text, n, fn - ((float)n));
		}
		if (notePlaying) {
		    auto n = voltageToPitch(play_v);
		    auto fn = voltageToMicroPitch(play_v);
		    pitchToText(playingNote_text, n, fn - ((float)n));
		    //debug_text=rack::string::f("%d", currDegree);
		} else {
		    playingNote_text = "";
		}
	    }
	    outputs[PITCH_OUTPUT].setChannels(numOutputChannels);
        }
    };

        static const NVGcolor textColor_red = nvgRGB(0xff, 0x33, 0x33);
    static const NVGcolor ledTextColor = textColor_red;

    struct NoteDisplayWidget : TransparentWidget {
        std::shared_ptr<Font> font;
        std::string fontPath;
        char displayStr[16];
        std::string* text;
        std::string fakeData;
	
        NoteDisplayWidget(std::string* t, std::string fakeData)
        {
            text = t;
	    this->fakeData = fakeData;
            fontPath = std::string(
                asset::plugin(pluginInstance, "res/fonts/opensans/OpenSans-Bold.ttf"));
        }

        void drawLayer(const DrawArgs& args, int layer) override
        {
            if (layer == 1) {
                if (!(font = APP->window->loadFont(fontPath))) {
                    return;
                }
                nvgFontSize(args.vg, 18.0);
                nvgFontFaceId(args.vg, font->handle);

                Vec textPos = Vec(6, 24);

                nvgFillColor(args.vg, ledTextColor);

                nvgTextAlign(args.vg, NVG_ALIGN_LEFT | NVG_ALIGN_BOTTOM);
                nvgText(args.vg, textPos.x, textPos.y, text ? text->c_str() : fakeData.c_str(), NULL);
            }
        }
    };

#define FIRST_X -5.0
#define FIRST_Y 5.0
#define LAST_Y 125.0
#define SPACING_X 20.0
#define SPACING_Y 15.0
#define FIRST_X_OUT -6.3
#define PAIR_SPACING 0.42
#define LABEL_OFFSET_X -19.0
#define LABEL_OFFSET_Y -12.0
#define LABEL_HEIGHT 22
#define LABEL_WIDTH 55

#define STRIP_X_MIN (FIRST_X+SPACING_X)
#define STRIP_Y_MIN (FIRST_Y+(SPACING_Y/2.0))
#define STRIP_Y_MAX LAST_Y
#define STRIP_LED_WIDTH 25.0
#define STRIP_LED_X_OFFSET 5.0
#define STRIP_WIDTH 12.0
#define STRIP_HEIGHT (115.0-STRIP_Y_MIN)
    
#define SPACING_X_OUT 14.1
#define LABEL_OFFSET_X_OUT (LABEL_OFFSET_X + 2.0)
#define LABEL_OFFSET_Y_OUT (LABEL_OFFSET_Y - 0.5) // leave space for the shading under the output jacks

#define LED_OFFSET_X -5.0
#define LED_OFFSET_Y -5.0

    struct StripDisplay : LedDisplay {

	Harp* module;

	void drawLayer(const DrawArgs& args, int layer) override {
	    if (layer != 1)
		return;
	    
	    if ((!module) || module->notePlaying) {
		
		int noteRange;
		int currDegree;
		if (module) {
		    noteRange = (int)module->params[Harp::NOTE_RANGE_PARAM].getValue();
		    currDegree = module->currDegree;
		} else {
		    // fake data for the module browser:
		    noteRange = 24;
		    currDegree = 6;
		}

#define STRIP_LED_Y_OFFSET 5.0		
		float height =  (box.getHeight()-(2.f*STRIP_LED_Y_OFFSET)) / noteRange;
		float x = STRIP_LED_X_OFFSET;
		float y = STRIP_LED_X_OFFSET + (height * ((noteRange-1)-currDegree));
		nvgBeginPath(args.vg);
		nvgFillColor(args.vg, ledTextColor);
		//INFO("rect %f %f %f %f %f",x, y, STRIP_LED_WIDTH, height, ratio);
		nvgRect(args.vg, x, y, STRIP_LED_WIDTH, height);
		nvgClosePath(args.vg);
		nvgFill(args.vg);
	    }
	    
	}
    };
    
    struct HarpWidget : ModuleWidget {
        HarpWidget(Harp* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/Harp.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 2)), module, Harp::SCALE_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 3)), module, Harp::PITCH_INPUT));
            addInput(createInputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 4)), module, Harp::GATE_INPUT));

            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 6)), module, Harp::PITCH_OUTPUT));
            addOutput(createOutputCentered<PJ301MPort>(
                mm2px(Vec(FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, FIRST_Y + SPACING_Y * 7)), module, Harp::GATE_OUTPUT));

	    auto rootNoteDisplay = new NoteDisplayWidget(module ? &module->rootNote_text : NULL, "C4");
	    rootNoteDisplay->box.size = Vec(30, 10);
	    rootNoteDisplay->box.pos = mm2px(Vec(LED_OFFSET_X + FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, LED_OFFSET_Y + FIRST_Y + SPACING_Y * 1));
	    addChild(rootNoteDisplay);

	    auto playingNoteDisplay = new NoteDisplayWidget(module ? &module->playingNote_text : NULL, "E4");
	    playingNoteDisplay->box.size = Vec(30, 10);
	    playingNoteDisplay->box.pos = mm2px(Vec(LED_OFFSET_X + FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, LED_OFFSET_Y + FIRST_Y + SPACING_Y * 5));
	    addChild(playingNoteDisplay);
	    
	    auto debugDisplay = new NoteDisplayWidget(module ? &module->debug_text : NULL, "");
	    debugDisplay->box.size = Vec(30, 10);
	    debugDisplay->box.pos = mm2px(Vec(LED_OFFSET_X + FIRST_X_OUT + SPACING_X_OUT + 0 * SPACING_X_OUT, LED_OFFSET_Y + FIRST_Y + SPACING_Y * 4.5));
	    addChild(debugDisplay);
	    
StripDisplay* display = createWidget<StripDisplay>(mm2px(Vec(STRIP_X_MIN, STRIP_Y_MIN)));
		display->box.size = mm2px(Vec(STRIP_WIDTH, STRIP_HEIGHT));
		display->module = module;
		addChild(display);
        }

	void appendContextMenu(Menu* menu) override
        {
            Harp* module = dynamic_cast<Harp*>(this->module);

            menu->addChild(new MenuSeparator);

	    /* Grrr, no ready-made way to get numeric input via text box or slider.   Punt and just produce a list of likely options. */
            menu->addChild(createIndexSubmenuItem(
						  "Number of notes mapped to the input CV pitch range",
						  {/*"0", "1",*/ "2","3","4","5","6","7","8","9",
							 "10", "11", "12","13","14","15","16","17","18","19",
							 "20", "21", "22","23","24","25","26","27","28","29",
							 "30", "31", "32","33","34","35","36","37","38","39",
							 "40", "41", "42","43","44","45","46","47","48" },
													 
						  [=]() { return module->params[Harp::NOTE_RANGE_PARAM].getValue()-2; },
                [=](int val) {
                    module->params[Harp::NOTE_RANGE_PARAM].setValue((int)val+2);
                }));
            menu->addChild(createIndexSubmenuItem(
                "Pitch CV Input Range", MIDIRecorder::CVRangeNames,
                [=]() { return module->params[Harp::PITCH_CV_RANGE_PARAM].getValue(); },
                [=](int val) {
		    module->params[Harp::PITCH_CV_RANGE_PARAM].setValue((MIDIRecorder::CVRangeIndex)val);
                }));
        }
	
    };

} // namespace Harp
} // namespace Chinenual

Model* modelHarp = createModel<Chinenual::Harp::Harp,
    Chinenual::Harp::HarpWidget>("Harp");
