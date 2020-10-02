(
var wtcount = 10;
var wtsize = 1024;
var signals = wtcount.collect({|n|
	var segments = n.linexp(0, wtcount, 4, 40).round;
	Env(
		[0] ++ ({1.0.rand}!(segments-1) * [1,-1]).scramble ++ [0],
		{exprand(1, n.linexp(0, wtcount, 1, 50))}!segments,
		{[\sine, 0, exprand(1, 20) * [1,-1].choose].wchoose([wtcount-n, wtcount/3, n].normalizeSum)}!segments
	).asSignal(wtsize);
});

// Allocate 1 extra empty buffer because VOsc fails at the last allocated buffer
// https://github.com/supercollider/supercollider/issues/4302
~wavetables = Buffer.allocConsecutive(wtcount+1, s, wtsize*2, 1,
	{|buf, n| if (n < wtcount, { buf.setnMsg(0, signals[n].asWavetable) });
});
)

(
Instr(\wt, {|unison=1, buf, freq=440, detune=0, gate=1, pan=0, env|
	var amp = Latch.kr(gate, gate);
	var detuneCtrl = unison.splay.collect({|n| (n*detune).midiratio});
	var sig = Osc.ar(buf, freq * detuneCtrl, mul: 1/unison);
	sig = Mix.ar(sig);
	sig = LeakDC.ar(sig);
	sig = Pan2.ar(sig, pan, amp);
	sig = sig * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	sig
}, [StaticIntegerSpec(1, 16, 1)]);

Instr(\vwt, {|unison=1,
	bufnum, bufCount=1, bufPos=0, freq=440, detune=0, deshape=0, gate=1, pan=0, env
|
	var amp = Latch.kr(gate, gate);
	var detuneCtrl = (unison.splay * detune).collect(_.midiratio);
	var pos = (unison.splay * deshape + bufPos).linlin(0, 1, bufnum, bufnum + bufCount - 1);
	var sig = VOsc.ar(pos, freq * detuneCtrl, mul: 1/unison);
	sig = Mix.ar(sig);
	sig = LeakDC.ar(sig);
	sig = Pan2.ar(sig, pan, amp);
	sig = sig * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	sig
}, [StaticIntegerSpec(1, 16, 1)]);

Instr(\monosample, {|startLoop, endLoop, freq, baseFreq, gate=1, buf, attack, decay, sustain, release|
	var amp = Latch.kr(gate, gate);
	var rate = BufRateScale.kr(buf) * (freq / baseFreq);
	var sig = LoopBuf.ar(1, buf, rate, gate: amp, startLoop: startLoop, endLoop: endLoop, interpolation: 4);
	sig = sig * amp;
	sig = sig * EnvGen.ar(Env.adsr(attack, decay, sustain, release), gate, doneAction: Done.freeSelf);
	sig!2
});

SynthDef(\vst, {|outbus, bypass|
	ReplaceOut.ar(outbus, VSTPlugin.ar(In.ar(outbus, 2), 2, bypass));
}).add;
)

(
~master = MixerChannel.new(\master, s, 2, 2, level: 1);
~sends = 1.collect({|n| MixerChannel.new(("send"++n).asSymbol, s, 2, 2, outbus: ~master)});
~tracks = 8.collect({|n| MixerChannel.new(("track" ++ n).asSymbol, s, 2, 2, outbus: ~master)});

~fxReverb = ~sends[0].playfx(\vst);
~fxLimiter = ~master.playfx(\vst);
)

(
~reverbCtrl = VSTPluginController(~fxReverb);
~reverbCtrl.open('TAL Reverb 4 Plugin.vst3', editor: true, action: {|ctrl|
	ctrl.setn(0, [1.0, 1.0, 0.0, 0.0, 0.96153128147125, 0.69659376144409, 0.19865627586842, 0.16583979129791, 0.4949432015419, 1.0]);
});
~limiterCtrl = VSTPluginController(~fxLimiter);
~limiterCtrl.open('Pro-L 2.vst3', editor: true);
)

(
Buffer.freeAll;
b = [
	Buffer.read(s, "/Users/gregsm/Downloads/snes/samples/sd3-117_012.wav"),
	Buffer.read(s, "/Users/gregsm/Downloads/snes/samples/sd3-117_008.wav"),
	Buffer.read(s, "/Users/gregsm/Downloads/snes/samples/sd3-117_009.wav"),
	Buffer.read(s, "/Users/gregsm/Downloads/snes/samples/sd3-117_011.wav")
];
)

(
~inst0 = Voicer.new(4, Instr.at(\monosample), [\buf, b[0], \startLoop, 48, \endLoop, 14576, \attack, 4, \decay, 0, \sustain, 1, \release, 2, \baseFreq, 880], target: ~tracks[0]);
~inst1 = Voicer.new(4, Instr.at(\monosample), [\buf, b[1], \startLoop, 48, \endLoop, 10272, \attack, 4, \decay, 0, \sustain, 1, \release, 2, \baseFreq, 880], target: ~tracks[1]);
~inst2 = Voicer.new(4, Instr.at(\monosample), [\buf, b[2], \startLoop, 7296, \endLoop, 9008, \attack, 0, \decay, 0, \sustain, 1, \release, 1, \baseFreq, 880], target: ~tracks[2]);
~inst3 = Voicer.new(4, Instr.at(\monosample), [\buf, b[3], \startLoop, 8992, \endLoop, 11680, \attack, 0, \decay, 0, \sustain, 1, \release, 1, \baseFreq, 880], target: ~tracks[2]);
~midi0 = VoicerMIDISocket([0, 0], ~inst0);
~midi1 = VoicerMIDISocket([0, 1], ~inst1);
~midi2 = VoicerMIDISocket([0, 2], ~inst2);
~midi3 = VoicerMIDISocket([0, 3], ~inst3);
)

(
~tracks[0].newPostSend(~sends[0], 0.2);
~tracks[1].newPostSend(~sends[0], 0.2);
~tracks[2].newPostSend(~sends[0], 0.2);
~tracks[3].newPostSend(~sends[0], 0.2);
)

(
~inst0.free;
~inst1.free;
~inst2.free;
~midi0.free;
~midi1.free;
~midi2.free;
)


~board = MixingBoard(mixers: ~tracks ++ ~sends ++ [~master]);
~reverbCtrl.editor;
~limiterCtrl.editor;

~tracks[0].play({SinOsc.ar(440!2) * EnvGen.ar(Env.perc)})
p = { Patch(\sample, (buf: b, startLoop: 48, endLoop: 10272, gate: 1, env: ~spcAdsr.value(16rA, 7, 7, 16r08), freq: 16r800, baseFreq: 16r1000))!2 }.play
p.set(\gate, 0);

MIDIClient.externalDestinations

(
MixerChannel.freeAll
)