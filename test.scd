( // Server setup
s.options.device = "Fireface 802 (23913873)";
s.options.numOutputBusChannels = 12;
s.options.numInputBusChannels = 12;
s.options.memSize = 2.pow(20);
s.reboot;
MIDIClient.init;
MIDIIn.connectAll;
)

( // Load drum kit samples
var dirname = thisProcess.nowExecutingPath.dirname;
var loadKit = {|kitname|
	var folder;
	~kit = Array.new;
	~kitmap = Dictionary.new;
	folder = PathName.new(dirname +/+ "media" +/+ kitname);
	folder.entries.do({ |path, i|
		~kit = ~kit.add(Buffer.read(s, path.fullPath));
		~kitmap.put(path.fileNameWithoutExtension, i);
	});
};
loadKit.value("tr808");
)

( // SynthDefs
SynthDef(\sqblip, {|out, amp = 1, freq = 440|
	var snd = PulseDPW.ar(freq)!2;
	snd = snd * EnvGen.ar(Env.linen(0.001, 0.1, 0.03), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\triblip, {|out, amp = 1, freq = 440|
	var snd = LFTri.ar(freq)!2;
	snd = snd * EnvGen.ar(Env.linen(0.001, 0.1, 0.03), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\noiseblip, {|out, amp = 1, sustain = 0.01|
	var snd = WhiteNoise.ar()!2;
	snd = HPF.ar(snd, 2000);
	snd = snd * EnvGen.ar(Env.linen(0.005, sustain, 0.01), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\monosample, {|out, buf, amp = 1, rate = 1|
	var snd = PlayBuf.ar(1, buf!2, BufRateScale.kr(buf) * rate, doneAction: Done.freeSelf)!2;
	Out.ar(out, snd * amp);
}).add;

SynthDef.new(\mic, {|in = 0, out = 0, amp = 1|
	var sig = SoundIn.ar(in) * amp;
	Out.ar(out, sig);
}).add;

SynthDef.new(\delayLine, {|amp = 1, in = 0, buf = 0, ptr = 0, freeze = 0|
	var rampTime = 0.025;
	var env = EnvGen.ar(Env.asr(rampTime, 1, rampTime), 1-freeze);
	var writeRate = env > 0;
	var inputSignal = In.ar(in, 1);
	var writeHead = Phasor.ar(0, BufRateScale.kr(buf) * writeRate, 0, BufFrames.kr(buf));
	BufWr.ar(env * inputSignal, buf, writeHead);
	Out.ar(ptr, writeHead);
}).add;

SynthDef.new(\granulator, {|
	amp = 1,
	buf = 0, ptr = 0, out = 0,
	frequency = 40, jitter = 0,
	length = 0.05, lengthRand = 1,
	delaySamples = 0, delaySamplesRand = 0,
	pan = 0, panRand = 0,
	grainEnv = -1
|
	var writeHead = In.ar(ptr, 1);
	var grainTrigger = Select.ar(jitter, [Impulse.ar(frequency), Dust.ar(frequency)]);
	var grainLengthSeconds = length * LFNoise1.ar(100).exprange(1/lengthRand, lengthRand);
	var grainLengthSamples = grainLengthSeconds * SampleRate.ir;
	var grainDelaySamples = delaySamples + LFNoise1.ar(100).unipolar(delaySamplesRand);
	var grainPan = pan + LFNoise1.kr(100).bipolar(panRand);
	var grainPosition = (writeHead - grainLengthSamples - grainDelaySamples) / BufFrames.kr(buf);

	var outputSignal = GrainBuf.ar(
		2,
		grainTrigger,
		grainLengthSeconds,
		buf,
		1,
		grainPosition,
		2,
		grainPan,
		grainEnv
	);
	Out.ar(out, outputSignal * amp);
}).add;
)

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

Synth.new(\mic, [\in, in, \out, ~micBus], ~group);
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
		\length, 2/10
	], ~granulatorGroup);
});
)

( // MIDIdefs
MIDIdef.noteOn(\ch0, {|velocity, note| Synth.new(\sqblip, [\freq, note.midicps, \amp, velocity / 127]); }, nil, 0);
MIDIdef.noteOn(\ch1, {|velocity, note| Synth.new(\triblip, [\freq, note.midicps, \amp, velocity / 127]); }, nil, 1);
MIDIdef.noteOn(\percKick, {|velocity|
	Synth.new(\monosample, [\buf, ~kit[~kitmap.at("kick-down")].bufnum, \amp, velocity / 127]);
}, 24, 9);
MIDIdef.noteOn(\percSnare1, {|velocity|
	Synth.new(\monosample, [\buf, ~kit[~kitmap.at("snare")].bufnum, \amp, velocity / 127]);
}, 26, 9);
MIDIdef.noteOn(\percSnare2, {|velocity| Synth.new(\noiseblip, [\amp, velocity / 127, \sustain, 0.05]); }, 26, 9);
MIDIdef.noteOn(\percHat, {|velocity| Synth.new(\noiseblip, [\amp, velocity / 127]); }, 28, 9);

MIDIdef.cc(\cc0, {|val|
	~delayLine.set(\freeze, val / 127);
	["cc0", "freeze", val / 127].postln;
}, 0, 15);
MIDIdef.cc(\cc1, {|val|
	~delayLine.set(\jitter, val / 127);
	["cc1", "jitter", val / 127].postln;
}, 1, 15);
MIDIdef.cc(\cc2, {
	~delayLine.set(\freeze, 0);
});
MIDIdef.cc(\cc100, {|val|
	var fr = val.linexp(0, 127, 1, 60);
	~granulatorGroup.set(\frequency, fr);
	~granulatorGroup.set(\length, 2 / fr);
	["cc100", "freq", fr].postln;
}, 100, 15);
MIDIdef.cc(\cc110, {|val|
	var samp = val.linlin(0, 127, 0, 3000);
	~granulatorGroup.set(\delaySamplesRand, samp);
	["cc101", "rand delay", samp].postln;
}, 110, 15);

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
