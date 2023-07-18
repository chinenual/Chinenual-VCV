#include <osdialog.h>

#include "plugin.hpp"

namespace Chinenual {
namespace SplitSort {

    struct SplitSort : Module {
        enum ParamId {
            PARAMS_LEN
        };
        enum InputId {
            INPUTS_LEN
        };
        enum OutputId {
            OUTPUTS_LEN
        };
        enum LightId {
            LIGHTS_LEN
        };

        SplitSort()
        {
            onReset();
            config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        }

        void onReset() override
        {
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
        }
    };

    struct SplitSortWidget : ModuleWidget {
        SplitSortWidget(SplitSort* module)
        {
            setModule(module);
            setPanel(
                createPanel(asset::plugin(pluginInstance, "res/SplitSort.svg")));
            addChild(createWidget<ScrewBlack>(Vec(RACK_GRID_WIDTH, 0)));
            addChild(
                createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
            addChild(createWidget<ScrewBlack>(
                Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
            addChild(createWidget<ScrewBlack>(Vec(box.size.x - 2 * RACK_GRID_WIDTH,
                RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
        }
    };

} // namespace SplitSort
} // namespace Chinenual

Model* modelSplitSort = createModel<Chinenual::SplitSort::SplitSort,
    Chinenual::SplitSort::SplitSortWidget>("SplitSort");
