(
var fmtSrc = {|src| switch(src,
	~midiSrc[\iac], {"IAC"},
	~midiSrc[\fireface], {"FF "},
	~midiSrc[\apc], {"APC"},
	{"???"}
)};
MIDIdef.noteOn(\lognoteon, {|velocity, note, chan, src|
	"[%] #% note % on  = %".format(fmtSrc.value(src), chan, note, velocity).postln;
});
MIDIdef.noteOff(\lognoteoff, {|velocity, note, chan, src|
	"[%] #% note % off = %".format(fmtSrc.value(src), chan, note, velocity).postln;
});
MIDIdef.cc(\logcc, {|val, num, chan, src|
	"[%] #% cc% = %".format(fmtSrc.value(src), chan, num, val).postln;
});
)