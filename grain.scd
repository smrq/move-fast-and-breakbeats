Require("server-setup", always: true);
Require("synthdefs/*");
~kit808 = Require("loadKit").value("tr808");

( // create granulator
var in = 10;
var out = 0;
var unison = 4;
var bufLength = 10;

~group = Group.new;
~micBus = Bus.audio(s, 1);
~ptrBus = Bus.audio(s, 1);
~buf = Buffer.alloc(s, bufLength * s.sampleRate, 1);
~buf.zero;
~envbuf = Buffer.alloc(s, 1024, 1).loadCollection(Env.linen(0.2, 0.4, 0.4).asSignal(1024));

Synth.new(\audioIn, [\in, in, \out, ~micBus], ~group);
~delayLine = Synth.new(\delayLine, [
	\in, ~micBus,
	\buf, ~buf,
	\ptr, ~ptrBus
], ~group, \addToTail);

~granulatorGroup = Group.new(~group, \addToTail);
unison.collect({
	Synth.new(\granulator, [
		\amp, 1/unison,
		\out, out,
		\buf, ~buf,
		\ptr, ~ptrBus,
		\frequency, 130*4/60,
		\jitter, 0,
		\delaySamplesRand, 660,
		\length, 1/10,
		\grainEnv, ~envbuf
	], ~granulatorGroup);
});
)

(
~bpm=130;
~granulatorGroup.set(\length, 1/(4*~bpm/60));
~granulatorGroup.set(\frequency, 4*~bpm/60);
~granulatorGroup.set(\delaySamplesRand, s.sampleRate/(4*4*~bpm/60));
)

( // MIDIdefs
MIDIdef.noteOn(\ch1, {|velocity, note|
	var amp = velocity.linlin(0, 127, 0, 0.5);
	Synth.new(\sqblip, [\freq, note.midicps, \amp, amp]);
}, nil, 0);
MIDIdef.noteOn(\ch2, {|velocity, note|
	var amp = velocity.linlin(0, 127, 0, 0.5);
	Synth.new(\triblip, [\freq, note.midicps, \amp, amp]);
}, nil, 1);
MIDIdef.noteOn(\ch10, {|velocity, note|
	var amp = velocity.linlin(0, 127, 0, 0.5);
	switch (note,
		24, { Synth.new(\sample1_2, [\buf, ~kit["kick_low"], \amp, amp]); },
		26, { Synth.new(\sample1_2, [\buf, ~kit["snare"], \amp, amp]); },
		28, { Synth.new(\noiseblip, [\amp, amp, \sustain, 0.05]); },
		29, { Synth.new(\noiseblip, [\amp, amp]); }
	);
}, nil, 9);
)
(
MIDIdef.cc(\cc60, {|val|
	switch (val,
		1, {
			~delayLine.set(\freeze, 0);
			SystemClock.sched(60.0/(130*4), {
				~delayLine.set(\freeze, 1);
			});
		},
		2, {
			~delayLine.set(\freeze, 1);
		},
		3, {
			~delayLine.set(\freeze, 0);
	});
}, 60, 15);
)
(
MIDIdef.cc(\cc60, {
	~delayLine.set(\freeze, 0);
	SystemClock.sched(60.0/(130*4), {
		~delayLine.set(\freeze, 1);
	});
}, 60, 15);
)
(
MIDIdef.cc(\cc20, {|val|
	var fr = val.linexp(0, 127, 1, 60);
	~granulatorGroup.set(\frequency, fr);
	~granulatorGroup.set(\length, 2 / fr);
	["freq", fr].postln;
}, 20, 15);
MIDIdef.cc(\cc50, {|val|
	var samp = val.linlin(0, 127, 0, 3000);
	~granulatorGroup.set(\delaySamplesRand, samp);
	["rand delay", samp].postln;
}, 50, 15);
)
(
MIDIdef.noteOn(\lognote, {|velocity, note| ["noteon", note, velocity].postln; });
MIDIdef.cc(\logcc, {|val, num| ["cc", num, val].postln; }, nil, 15);
)

~granulatorGroup.set(\jitter, 1);
~granulatorGroup.set(\panRand, 2);
~granulatorGroup.set(\lengthRand, 2);

(
~group.free;
~micBus.free;
~ptrBus.free;
~buf.free;
)
