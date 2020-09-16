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
	folder = PathName.new(~dirname +/+ "media" +/+ kitname);
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
SynthDef(\monoinput, {|in = 0, out = 0, amp = 1|
	var snd = SoundIn.ar(in!2);
	Out.ar(out, snd * amp);
}).add;
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
MIDIdef.noteOn(\lognote, {|velocity, note| ["noteon", note, velocity].postln; });

MIDIdef.cc(\cc0, {|val, num| ["cc", num, val].postln; }, nil, 15);
)

( // Test sounds
Synth.new(\sqblip, [\amp: 0.3, \freq: 660]);
Synth.new(\triblip, [\amp: 0.5, \freq: 220]);
Synth.new(\noiseblip, [\amp: 0.2, \sustain: 0.05]);
)


b = Buffer.alloc(s, s.sampleRate * 10, 1);

(
SynthDef.new(\mic, {|in = 0, out = 0, amp = 1|
	var sig = SoundIn.ar(in) * amp;
	Out.ar(out, sig);
}).add;
SynthDef.new(\ptr, {|out = 0, buf = 0, rate = 1|
	var sig = Phasor.ar(0, BufRateScale.kr(buf) * rate, 0, BufFrames.kr(buf));
	Out.ar(out, sig);
}).add;
SynthDef.new(\rec, {|ptrIn = 0, micIn = 0, buf = 0|
	var ptr, sig;
	ptr = In.ar(ptrIn, 1);
	sig = In.ar(micIn, 1);
	BufWr.ar(sig, buf, ptr);
}).add;
SynthDef.new(\gran, {|
	amp = 1,
	ptrBus = 0, buf = 0, out = 0,
	frequency = 40, jitter = 0,
	length = 0.05, lengthRand = 1,
	delaySamples = 0, delaySamplesRand = 0,
	pan = 0, panRand = 0,
	grainEnv = (-1)
|
	var writeHead = In.ar(ptrBus, 1);
	var grainTrigger = Select.ar(jitter, [Impulse.ar(frequency), Dust.ar(frequency)]);
	var grainLengthSeconds = length * LFNoise1.ar(100).exprange(1/lengthRand, lengthRand);
	var grainLengthSamples = grainLengthSeconds * SampleRate.ir;
	var grainDelaySamples = delaySamples + LFNoise1.ar(100).unipolar(delaySamplesRand);
	var grainPan = pan + LFNoise1.kr(100).bipolar(panRand);
	var grainPosition = (writeHead - grainLengthSamples - grainDelaySamples) / BufFrames.kr(buf);

	var sig = GrainBuf.ar(
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
	Out.ar(out, sig * amp);
}).add;
)

(
~micBus = Bus.audio(s, 1);
~ptrBus = Bus.audio(s, 1);
)

(
~micGrp = Group.new;
~ptrGrp = Group.after(~micGrp);
~recGrp = Group.after(~ptrGrp);
~granGrp = Group.after(~recGrp);

MIDIdef.cc(\cc0, {
	"freeze".postln;
	~ptrGrp.set(\rate, 0);
}, 0, 0);
MIDIdef.cc(\cc1, {
	"unfreeze".postln;
	~ptrGrp.set(\rate, 1);
}, 1, 0);
)

(
b.zero;
Synth(\mic, [\in, 10, \out, ~micBus], ~micGrp);
Synth(\ptr, [\buf, b, \out, ~ptrBus], ~ptrGrp);
Synth(\rec, [\ptrIn, ~ptrBus, \micIn, ~micBus, \buf, b], ~recGrp);
g = 10.collect({|n|
	Synth(\gran, [
		\amp, 0.2,
		\ptrBus, ~ptrBus,
		\buf, b,
		\delaySamplesRand, 44100
	], ~granGrp)
});
)

s.defaultGroup.deepFree;
