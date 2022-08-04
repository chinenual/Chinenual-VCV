# Change log for Chinenual-VCV

## 2.1.0b1

* New expander module for the MIDI Recorder.  Adds support for 5 additional
  CC values per track. Each can be configured with 
  * custom input voltage ranges, 
  * the CC control number and 
  * whether the value should be transmitted as 7-bit (0..127) or 14-bit (0..16384).
  * NOTE: if 14-bit CC is selected, two CC messages are created.  One at the configured control number (with the MSB part of the value) and one at control number+32 (with the LSB part).   
  * The module does not attempt to prevent you from configuring conflicting CC's (e.g. if column 1's settings are CC=2, 14-bit, and column 2's is CC=34, both columns will be producing CC messages for CC=34.  It does not prevent you from creating 14-bit CC pairs outside the "normal" range defined by the MIDI spec (only CC0 through CC31 have well defined paired CC's from 32 through 63, but the module will allow you to specify, for example, CC70 as 14 bit, which will emit CC70(MSB) and CC102(LSB))
  * Expander must be adjacent to the Recorder module - on its right.
  * Multiple expander modules can be used - they must be adjacent/touching to each other.

* The Recorder has new config options 
  * 14-bit CC can be sent for MW (CC1 & CC33). Default is 7-bit (CC1 only)
  * VEL, AFT, PW and MW voltage input ranges can be changed to match the source modulator outputs.   Defaults remain as in the 2.0.0 release (which matches the VCV Core MIDI-CC module behavior).
  * Brighter BPM LED color.

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
