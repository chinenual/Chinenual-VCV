#include "plugin.hpp"

Plugin* pluginInstance;

void init(Plugin* p)
{
    pluginInstance = p;

    // Add modules here
    p->addModel(modelMIDIRecorder);
    p->addModel(modelMIDIRecorderCC);
    p->addModel(modelDrumMap);
    p->addModel(modelTint);
    p->addModel(modelNoteMeter);
    p->addModel(modelInv);
    p->addModel(modelSplitSort);
    p->addModel(modelMergeSort);
    p->addModel(modelPolySort);

    // Any other plugin initialization may go here.
    // As an alternative, consider lazy-loading assets and lookup tables when your
    // module is created to reduce startup times of Rack.
}
