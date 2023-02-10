# Change log for Chinenual-VCV

## 2.3.0

*Improved support for capturing percussive gates:
 * If a track's V/Oct input is not connected, the recorder defaults to 
  MIDI C4 (so for drums, only a GATE input is necessary). When a polyphonic gate is connected, but no V/Oct, then each channel of the GATE gets a unique MIDI note pitch.
 * A new module, DrumMapper, maps named gates to General MIDI "drum" notes.

## 2.2.0

* New Output Port: **ACTIVE** can be used to synchronize
  the MIDI Recoding with an audio recording.  When **Align At First
  Note** is enabled, this goes high after the first note is
  detected, else as soon as the recording is started.
*  An LED under the **ACTIVE** port lights up when the recorder
   is capturing events (it lights up when the **ACTIVE**
   gate is high).
* Panel layout changes - the first column of ports is shifted left a
  bit to align with the **Record** button.
* The **Record** button is now labeled **REC** (was **RUN**). 
* The **Start/Stop Gate** input is no longer labeled **GATE**. It  shares
  the **REC** label with the **Record** button.
* Fixes a bug that could cause Rack to crash if Rack is shut down 
  while the recorder is actively recording. 

## 2.1.1

Fix URLs in the plugin.json manifest.

## 2.1.0

* Adds the new MIDI Recorder CC expander module.  
* Fixes a memory allocation performance bug.

## 2.1.0b2

* Fixes memory allocation bug: no longer allocate from the audio thread.
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
