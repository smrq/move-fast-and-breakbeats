(
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

~tracks[0].level = 0.4;
~tracks[1].level = 0.75;
~tracks[2].level = 0.63;
~tracks[3].level = 0.44;
~tracks[4].level = 0.44;

~tracks[0].newPostSend(~sends[0], 0.34);
~tracks[1].newPostSend(~sends[0], 0.39);
~tracks[2].newPostSend(~sends[0], 0.47);
~tracks[3].newPostSend(~sends[0], 0.62);
~tracks[4].newPostSend(~sends[0], 0.15);

~fxReverb = ~sends[0].playfx(\vst);
// ~fxLimiter = ~master.playfx(\vst);
)

(
~reverbCtrl = VSTPluginController(~fxReverb);
~reverbCtrl.open('TAL Reverb 4 Plugin.vst3', editor: true, action: {|ctrl|
	ctrl.setn(0, [1.0, 1.0, 0.0, 0.0, 0.96153128147125, 0.69659376144409, 0.19865627586842, 0.16583979129791, 0.4949432015419, 1.0]);
});
// ~limiterCtrl = VSTPluginController(~fxLimiter);
// ~limiterCtrl.open('Pro-L 2.vst3', editor: true);
)

(
var dir = thisProcess.nowExecutingPath.dirname +/+ "../media/spc";
Buffer.freeAll;
b = [
	(path: dir +/+ "sd3-117_012.wav", freq: 880, blocks: 911, loopBlock: 3, loopFrom: 2, loopTo: 1, a: 4, r: 2),
	(path: dir +/+ "sd3-117_008.wav", freq: 880, blocks: 642, loopBlock: 3, loopFrom: 2, loopTo: 1, a: 4, r: 2),
	(path: dir +/+ "sd3-117_009.wav", freq: 880, blocks: 563, loopBlock: 456, loopFrom: 2, loopTo: 1, a: 0, r: 1),
	(path: dir +/+ "sd3-117_011.wav", freq: 880, blocks: 730, loopBlock: 562, loopFrom: 3, loopTo: 2, a: 0, r: 1),
	(path: dir +/+ "sd3-112_013.wav", freq: 880, blocks: 281, loopBlock: 234, loopFrom: 3, loopTo: 1, a: 0, r: 1),
];
b.do({|x|
	var initialBlockCount = x.loopBlock;
	var loopBlockCount = x.blocks - x.loopBlock;
	x.buf = Buffer.read(s, x.path);
	x.startLoop = 16*(initialBlockCount + (x.loopTo * loopBlockCount));
	x.endLoop = 16*(initialBlockCount + (x.loopFrom * loopBlockCount));
});
)

(
~inst = b.collect({|x, n|
	Voicer.new(4, Instr.at(\monosample), [
		\buf, x.buf,
		\startLoop, x.startLoop,
		\endLoop, x.endLoop,
		\attack, x.a,
		\decay, 0,
		\sustain, 1,
		\release, x.r,
		\baseFreq, x.freq
	], target: ~tracks[n])
});
~midi = ~inst.collect({|inst, n| VoicerMIDISocket([0, n], inst)});
)

~board = MixingBoard(mixers: ~tracks ++ ~sends ++ [~master]);
~reverbCtrl.editor;
// ~limiterCtrl.editor;

~inst.do(_.free);
~midi.do(_.free);
MixerChannel.freeAll;

s.record(thisProcess.nowExecutingPath.dirname +/+ "recording.aiff", 0, 2);
s.stopRecording;
