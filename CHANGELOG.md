# Change log for Chinenual-VCV

## 2.1.0b2

* Fixes memory deallocation bug that caused memory to not be deleted
  as soon as it could be. 

## 2.1.0b1

* New expander module for the MIDI Recorder.  Adds support for 5 additional
  CC values per track. Each can be configured with 
  * custom input voltage ranges, 
  * the CC control number and 
  * whether the value should be transmitted as 7-bit (0..127) or 14-bit (0..16384).
  * Multiple expander modules can be used - they must be adjacent/touching to each other.

* The Recorder has new config options 
  * 14-bit CC can be sent for MW (CC1 & CC33). Default is 7-bit (CC1 only)
  * VEL, AFT, PW and MW voltage input ranges can be changed to match
    the source modulator outputs.   Defaults remain as in the 2.0.0
    release (which matches the VCV Core CV-MIDI module behavior).
  * Revamped the UI - brighter BPM LED color, more readable labels. Track row labels.

## 2.0.0

* Official release for the VCV Library.

## 2.0.0b5

* Fixes timing error that caused recorded notes to drift off tempo.  
* Adds support for tracking tempo changes during the recording.

## 2.0.0b4

* Fixes bug that caused crash when the module library browser creates a preview image of the module.

## 2.0.0b3

* Layout change to put the BPM jack closer to the new BPM display. 

## 2.0.0b2

* Adds BPM display

## 2.0.0b1

* First beta release of the new MIDI Recorder.
