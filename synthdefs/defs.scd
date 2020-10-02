SynthDef(\vst, {|outbus, bypass|
	ReplaceOut.ar(outbus, VSTPlugin.ar(In.ar(outbus, 2), 2));
}).add;

Instr(\input, {|in=0, amp=1, channels=1|
	var out = SoundIn.ar(in, mul: amp);
	out!channels
});
Instr(\sin, {|freq=440, gate=0, env, channels=2|
	var amp = Latch.kr(gate, gate);
	var out = SinOsc.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!channels
});
Instr(\tri, {|freq=440, gate=0, env, channels=2|
	var amp = Latch.kr(gate, gate);
	var out = LFTri.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!channels
});
Instr(\sqr, {|freq=440, width=0.5, gate=0, env, channels=2|
	var amp = Latch.kr(gate, gate);
	var out = PulseDPW.ar(freq, width, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!channels
});
Instr(\saw, {|freq=440, gate=0, env, channels=2|
	var amp = Latch.kr(gate, gate);
	var out = SawDPW.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!channels
});
Instr(\sample, {|freq=440, baseFreq=440, gate=0, buf, inChannels=1|
	var out = PlayBuf.ar(inChannels, buf, freq / baseFreq * BufRateScale.kr(buf), doneAction: Done.freeSelf);
	out = out * gate;
	out
});
