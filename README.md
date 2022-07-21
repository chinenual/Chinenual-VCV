# Chinenual-VCV

Virtual Eurorack modules for [VCV Rack](https://vcvrack.com), available in the [plugin library](https://library.vcvrack.com/).
Version 2.0.0b1

**(NOTE: not yet submitted to the library - pending beta testing).**

Feedback and bug reports (and [donations](https://paypal.me/chinenual)) are always appreciated!

The [Changelog](./CHANGELOG.md) describes changes.

## Modules

### MIDI Recorder

![module-screenshot](./doc/MIDIRecorder.png)

A multi-track recorder to capture a VCV performance to a standard MIDI
file.   Supports up to 10 polyphonic tracks, capturing CV in the same
way the VCV core CV-MIDI module does.

Each row of inputs corresponds to one track of MIDI. 

#### Motivation

I run VCV Rack on a fairly low performance laptop (by current
standards).  I can't run Rack and a DAW at the same time, so I can't
just stream MIDI out of Rack and record it in my DAW.   I suspect I'm
not alone.   This module lets me capture 10 tracks of MIDI into a standard MIDI
file and then import it into my DAW.

####  How it works

* **RUN**  - If the **GATE** input is unconnected, you can press
  this button to start and end the recording.  It changes color to red
  while recording.
  
* **GATE** - You can control the recording start/stop with a gate
  signal (e.g. from Count Modula's FADE module).   Recording starts
  when the signal
  is above 0.0v; stops when it drops to or below 0.0v.   The Run
  button turns red during recording.
  
* **BPM** - Use this to set the tempo of the MIDI file.  Uses same
  conventions as Impromptu's CLOCKED BPM output (BPM = 120 * 2^voltage).  If unconnected, sets the MIDI tempo
  to 120 BPM.

The remaining inputs accept the same signals that the VCV core
CV-MIDI module, each row feeding a separate polyphonic track in the
target MIDI file:

* **V/OCT** - polyphonic. Note pitch (1V/oct)
* **GATE** - polyphonic. Note gates (0 .. 10V)
* **VEL**  - polyphonic. Note "velocity" (0 .. 10V)
* **AFT** - polyphonic. Aftertouch/Key Pressure (0V .. 10V)
* **PW** - monophonic. Pitchbend (-5V .. 5V)
* **MW** - monophonic. Mod Wheel (0V .. 10V)


Target file selection works in a similar way to the VCV Recorder
plugin.  You can select a file from the context menu, or if none has
been selected, you'll get a popup dialog the first time you try to
record something:

* **Output File** - the path to the target MIDI file to be produced.
* **Append -001, -002, etc.** - If checked, the plugin creates unique files if the target
  file already exists (`/my/file.mid`,  `/my/file-001.mid`,
  `/my/file-002.mid`). If unchecked, the file is overwritten.


## Acknowledgements

The MIDIRecorder leverages builtin functionality of the VCV Rack core
MIDI support and uses the same third party MIDI File library as
Squinky Lab's SEQ++ (Craig Stewart's [midifile](https://github.com/craigsapp/midifile) library). 

