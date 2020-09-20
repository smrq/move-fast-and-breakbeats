SynthDef(\sqblip, {|out = 0, amp = 1, freq = 440|
	var snd = PulseDPW.ar(freq)!2;
	snd = snd * EnvGen.ar(Env.linen(0.001, 0.1, 0.03), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\triblip, {|out = 0, amp = 1, freq = 440|
	var snd = LFTri.ar(freq)!2;
	snd = snd * EnvGen.ar(Env.linen(0.001, 0.1, 0.03), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\noiseblip, {|out = 0, amp = 1, sustain = 0.01|
	var snd = WhiteNoise.ar()!2;
	snd = HPF.ar(snd, 2000);
	snd = snd * EnvGen.ar(Env.linen(0.005, sustain, 0.01), doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\sine, {|
	out = 0, amp = 1,
	freq = 440,
	attack = 0.1,
	decay = 0.1,
	sustain = 0.8,
	release = 0.25,
	gate = 1
|
	var snd = SinOsc.ar(freq!2);
	snd = snd * EnvGen.ar(
		Env.adsr(attack, decay, sustain, release),
		gate,
		doneAction: Done.freeSelf
	);
	snd = snd * amp;
	Out.ar(out, snd);
}).add;

SynthDef(\triangle, {|
	out = 0, amp = 1,
	freq = 440,
	attack = 0.1,
	decay = 0.1,
	sustain = 0.8,
	release = 0.25,
	gate = 1
|
	var snd = LFTri.ar(freq!2);
	snd = snd * EnvGen.ar(
		Env.adsr(attack, decay, sustain, release),
		gate,
		doneAction: Done.freeSelf
	);
	snd = snd * amp;
	Out.ar(out, snd);
}).add;

SynthDef(\square, {|
	out = 0, amp = 1,
	freq = 440,
	attack = 0.1,
	decay = 0.1,
	sustain = 0.8,
	release = 0.25,
	width = 0.5,
	gate = 1
|
	var snd = PulseDPW.ar(freq!2, width);
	snd = snd * EnvGen.ar(
		Env.adsr(attack, decay, sustain, release),
		gate,
		doneAction: Done.freeSelf
	);
	snd = snd * amp;
	Out.ar(out, snd);
}).add;

SynthDef(\saw, {|
	out = 0, amp = 1,
	freq = 440,
	attack = 0.1,
	decay = 0.1,
	sustain = 0.8,
	release = 0.25,
	gate = 1
|
	var snd = SawDPW.ar(freq!2);
	snd = snd * EnvGen.ar(
		Env.adsr(attack, decay, sustain, release),
		gate,
		doneAction: Done.freeSelf
	);
	snd = snd * amp;
	Out.ar(out, snd);
}).add;
