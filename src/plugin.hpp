#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
extern Model* modelMIDIRecorder;
extern Model* modelMIDIRecorderCC;
extern Model* modelDrumMap;
extern Model* modelTint;
