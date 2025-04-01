# Change log for Chinenual-VCV

## 2.7.1

* Implements [issue #9](https://github.com/chinenual/Chinenual-VCV/issues/9): New option for Harp and NoteMeter - choose between displaying notes with Sharps vs. Flats via a new context menu option.
* Implements [issue #10](https://github.com/chinenual/Chinenual-VCV/issues/10): NoteMeter can now display raw voltage with configurable precision via a new context menu option.

## 2.7.0

* New module, Harp; a reimagining of Iasos's Golden Harp.

## 2.6.1

* Fixes [issue #7](https://github.com/chinenual/Chinenual-VCV/issues/7) a bug in the Tintinabulator that sometimes caused it to mis-track the input Chord values as they change.

## 2.6.0

* New modules, SplitSort and MergeSort, split and merge polyphonic cables while sorting the channels.  Sort criteria can be shared across modules via a LINK cable. A reimplementation of Aria Salvatrice's Splort and Smerge modules. 
* A related module, PolySort, sorts polyphonic signals, optionally reusing sort order foe each signal.

## 2.5.0

* New module, Inv, produces a chromatically inverted V/oct pitch relative to a specified "pivot" pitch.

## 2.4.2

* Implements [issue #5](https://github.com/chinenual/Chinenual-VCV/issues/5).
  The Tintinabulator now supports non-equal temperament tunings. 
* Implements [issue #6](https://github.com/chinenual/Chinenual-VCV/issues/6).
  The Tintinabulator has a new mode, Quantize, that simply snaps the
  input melody to the nearest "Chord" frequency.   This is technically
  not "tintinnabulation" since if the melody note matches a chord
  note, it will not create a "harmony" note.  However, this allows the
  Tintinnabulator to be used as a general purpose "chord quantizer".
* Fixes [issue #4](https://github.com/chinenual/Chinenual-VCV/issues/4)
  NoteMeter crash reported on Windows.

## 2.4.1

* Skipped due to release build mixup.

## 2.4.0

* New module, Tintinnabulator.   Produce a harmonized pitch from an
  input chord and melody using Avro Pärt-style tintinnabulation.  Supports
  several variants of the harmonization - select first available pitch
  or next one, in both upwards or downwards direction.  Also supports
  bidirectional tintinnabulation, where every other note switches
  between upward or downwards selection.  An Octave offset can be used
  to offset the harmony up or down from the reference melody line.
* New module, NoteMeter,  a polyphonic metering module that displays note names
 from V/Oct inputs.	If the input voltage does not align exactly with a
 note voltage it indicates the deviance from the closest note as
 "cents". 

## 2.3.1

* Fix DrumMap preview image.

## 2.3.0

* Improved support for capturing percussion events:
  * In the MIDIRecorder, if a track's **V/OCT** input is not connected, the recorder defaults to 
  MIDI C4 (so for drums, only a **GATE** input is necessary). When a polyphonic gate is connected, but no **V/OCT**, then each channel of the **GATE** gets a unique MIDI note pitch.
  * A new module, DrumMap, maps named gates to General MIDI "drum" notes (which can then be plugged into the MIDIRecorder or sent to a live drum machine via the core CV-MIDI module).

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
