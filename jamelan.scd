(
Require("server-setup", always: true);
Require("synthdefs/*");
)

~kit808 = Require("loadKit").value("tr808");

(
s.plotTree;
s.meter;
)

(// hack HiDPI fix
10.collect({ Group.new });
AppClock.sched(0.5, {
	s.defaultGroup.freeAll;
});
)

(
~instBus = 16.collect({ Bus.audio(s, 1); });
~fxBus = 16.collect({ Bus.audio(s, 2); });
~masterBus = Bus.audio(s, 2);
)

(
~instGroup = Group.tail(s.defaultGroup);
~sendGroup = Group.tail(s.defaultGroup);
~fxGroup = Group.tail(s.defaultGroup);
~mixGroup = Group.tail(s.defaultGroup);
~masterGroup = Group.tail(s.defaultGroup);
)

(
Synth.new(\patch2, [\in, ~instBus[0], \out, ~fxBus[0]], ~sendGroup);
Synth.new(\patch2, [\in, ~fxBus[0], \out, ~masterBus], ~mixGroup);
Synth.tail(~masterGroup, \patch2, [\in, ~masterBus, \out, 0]);
)

(
~createMidiInstrument = {|name, fn, chan, src|
	var notes = Dictionary.new;
	MIDIdef.noteOn((name ++ "_on").asSymbol, {|vel, note|
		var synth = notes.at(note);
		if (synth != nil, {
			synth.set(\gate, 0);
		});
		synth = fn.value(vel, note);
		notes.add(note -> synth);
	}, nil, chan, src);
	MIDIdef.noteOff((name ++ "_off").asSymbol, {|vel, note|
		var synth = notes.at(note);
		if (synth != nil, {
			synth.set(\gate, 0);
		});
		notes.removeAt(note);
	}, nil, chan, src);
};
)

(
~createMidiInstrument.value(\gamel0, {|vel, note|
	var freq = ~midipelog.value(note);
	var amp = vel.linlin(0, 127, 0, 0.5);
	Synth.new(\square, [
		\out, ~instBus[0],
		\freq, freq,
		\amp, amp,
		\attack, 0,
		\decay, 0.5,
		\sustain, 0.4,
		\release, 1
	], ~instGroup)
}, 0, ~midiSrc[\iac]);

~createMidiInstrument.value(\gamel1, {|vel, note|
	var freq = ~midipelog.value(note);
	var amp = vel.linlin(0, 127, 0, 0.5);
	Synth.new(\sine, [
		\out, ~instBus[0],
		\freq, freq,
		\amp, amp,
		\attack, 0,
		\decay, 2,
		\sustain, 0.1,
		\release, 1
	], ~instGroup)
}, 1, ~midiSrc[\iac]);

~createMidiInstrument.value(\gamel2, {|vel, note|
	var freq = ~midipelog.value(note);
	var amp = vel.linlin(0, 127, 0, 0.5);
	Synth.new(\saw, [
		\out, ~instBus[0],
		\freq, freq,
		\amp, amp,
		\attack, 0,
		\decay, 2,
		\sustain, 0.1,
		\release, 1
	], ~instGroup)
}, 2, ~midiSrc[\iac]);

~createMidiInstrument.value(\gamel3, {|vel, note|
	var freq = ~midipelog.value(note);
	var amp = vel.linlin(0, 127, 0, 0.5);
	Synth.new(\triangle, [
		\out, ~instBus[0],
		\freq, freq,
		\amp, amp,
		\attack, 0,
		\decay, 2,
		\sustain, 0.1,
		\release, 1
	], ~instGroup)
}, 3, ~midiSrc[\iac]);
)

(
MIDIdef.noteOn(\ch10, {|velocity, note|
	var amp = velocity.linlin(0, 127, 0, 0.5);
	switch (note,
		24, { Synth.new(\sample1_2, [\out, ~masterBus, \buf, ~kit808["kick-down"], \amp, amp], ~instGroup); },
		26, { Synth.new(\sample1_2, [\out, ~masterBus, \buf, ~kit808["snare"], \amp, amp], ~instGroup); },
		28, { Synth.new(\sample1_2, [\out, ~masterBus, \buf, ~kit808["hat-shut"], \amp, amp], ~instGroup); },
	);
}, nil, 9);
)

(
~reverb = Synth.new(\vstinsert, [\bus, ~fxBus[0]], ~fxGroup);
~reverbCtrl = VSTPluginController(~reverb);
~reverbCtrl.open('TAL Reverb 4 Plugin.vst3', editor: true);

~limiter = Synth.new(\vstinsert, [\bus, ~masterBus], ~masterGroup);
~limiterCtrl = VSTPluginController(~limiter);
~limiterCtrl.open('Pro-L 2.vst3', editor: true);
)

(
~reverbCtrl.editor;
~limiterCtrl.editor;
)

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
