#include <osdialog.h>

#include "plugin.hpp"

namespace Chinenual {
namespace Harp {

    struct Harp : Module {
        enum ParamId {
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

        const int noteRange = 16; // number of notes to map into the cv range
        const float inputCvMin = 0.f;
	const float inputCvMax = 10.f;
        const int numOutputChannels = 5;
	    
	bool notePlaying;
        float currNote;
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
        }

        void onReset() override
        {
	    notePlaying = false;
	    currNote = -1.0f; // out of range so first use will detect note "change"
	    currChan = 0;
        }

        json_t* dataToJson() override
        {
            json_t* rootJ = json_object();
            return rootJ;
        }

        void dataFromJson(json_t* rootJ) override
        {
            if (rootJ == 0)
                return;
        }

        void process(const ProcessArgs& args) override
        {
	    float prevNote = currNote;

	    bool notePlaying = inputs[GATE_INPUT].getVoltage() >= 1.f;
	    if (notePlaying) {
		 auto v = inputs[PITCH_INPUT].getVoltage();
		 int s = std::round((v - inputCvMin) / (inputCvMax - inputCvMin) * noteRange);
		 int degree = s % inputs[SCALE_INPUT].getChannels();
		 int octave = s / inputs[SCALE_INPUT].getChannels();
		 currNote = inputs[SCALE_INPUT].getPolyVoltage(degree) + (octave * 1.f);
		 //INFO("HARP %f %d %d %d %f\n",v,s,degree,octave,currNote);
	    }

	    if (notePlaying) {
		bool noteChanged = prevNote != currNote;
		if (noteChanged) {
		    //INFO("HARP: note change %d %f %f\n", currChan, prevNote,currNote);
		    outputs[GATE_OUTPUT].setVoltage(0.f, currChan);
		    currChan = (currChan + 1) % numOutputChannels;
		}
		//INFO("HARP: note ON %d %f\n", currChan, currNote);
		outputs[GATE_OUTPUT].setVoltage(10.f, currChan);
		outputs[PITCH_OUTPUT].setVoltage(currNote, currChan);
	    } else {
		//INFO("HARP: note OFF %d\n", currChan);
	        outputs[GATE_OUTPUT].setVoltage(0.f, currChan);
	    }
	    outputs[GATE_OUTPUT].setChannels(numOutputChannels);
	    outputs[PITCH_OUTPUT].setChannels(numOutputChannels);
        }
    };

#define FIRST_X -5.0
#define FIRST_Y 5.0
#define SPACING_X 20.0
#define SPACING_Y 15.0
#define FIRST_X_OUT -6.3
#define PAIR_SPACING 0.42
#define LABEL_OFFSET_X -19.0
#define LABEL_OFFSET_Y -12.0
#define LABEL_HEIGHT 22
#define LABEL_WIDTH 55

#define SPACING_X_OUT 14.1
#define LABEL_OFFSET_X_OUT (LABEL_OFFSET_X + 2.0)
#define LABEL_OFFSET_Y_OUT (LABEL_OFFSET_Y - 0.5) // leave space for the shading under the output jacks

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
        }
    };

} // namespace Harp
} // namespace Chinenual

Model* modelHarp = createModel<Chinenual::Harp::Harp,
    Chinenual::Harp::HarpWidget>("Harp");
