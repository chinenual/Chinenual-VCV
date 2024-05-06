#pragma once
#include <rack.hpp>

using namespace rack;
using namespace rack::plugin;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelDrumMap;
extern Model* modelHarp;
extern Model* modelInv;
extern Model* modelMergeSort;
extern Model* modelMIDIRecorder;
extern Model* modelMIDIRecorderCC;
extern Model* modelNoteMeter;
extern Model* modelPolySort;
extern Model* modelSplitSort;
extern Model* modelTint;

