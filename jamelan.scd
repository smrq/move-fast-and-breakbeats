Require("server-setup", always: true);

(
Instr(\sin, {|freq=440, gate=0, env|
	var amp = Latch.kr(gate, gate);
	var out = SinOsc.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!2
});
Instr(\tri, {|freq=440, gate=0, env|
	var amp = Latch.kr(gate, gate);
	var out = LFTri.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!2
});
Instr(\sqr, {|freq=440, width=0.5, gate=0, env|
	var amp = Latch.kr(gate, gate);
	var out = PulseDPW.ar(freq, width, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!2
});
Instr(\saw, {|freq=440, gate=0, env|
	var amp = Latch.kr(gate, gate);
	var out = SawDPW.ar(freq, mul: amp);
	out = out * EnvGen.ar(env, gate, doneAction: Done.freeSelf);
	out!2
});
Instr(\sample, {|freq=440, gate=0, buf|
	var out = PlayBuf.ar(1, buf, BufRateScale.kr(buf), doneAction: Done.freeSelf);
	out = out * gate;
	out!2
});
SynthDef(\vst, {|outbus, bypass|
	ReplaceOut.ar(outbus, VSTPlugin.ar(In.ar(outbus, 2), 2));
}).add;
)

(
~kit808 = SoundFile.collectIntoBuffers(thisProcess.nowExecutingPath.dirname +/+ "media/tr808/*");

~master = MixerChannel.new(\master, s, 2, 2, level: 1);
~sends = 1.collect({|n| MixerChannel.new(("send"++n).asSymbol, s, 2, 2, outbus: ~master)});
~tracks = 7.collect({|n| MixerChannel.new(("track" ++ n).asSymbol, s, 2, 2, outbus: ~master)});

~inst0 = Voicer(8, Instr.at(\sqr), [\env, Env.adsr(0, 0.5, 0.4, 1)], target: ~tracks[0]);
~inst1 = Voicer(8, Instr.at(\sin), [\env, Env.adsr(0, 2, 0.1, 1)], target: ~tracks[1]);
~inst2 = Voicer(8, Instr.at(\saw), [\env, Env.adsr(0, 2, 0.1, 1)], target: ~tracks[2]);
~inst3 = Voicer(8, Instr.at(\tri), [\env, Env.adsr(0, 2, 0.1, 1)], target: ~tracks[3]);
~instKick = Voicer(1, Instr.at(\sample), [\buf, ~kit808[11]], target: ~tracks[4]);
~instSnare = Voicer(1, Instr.at(\sample), [\buf, ~kit808[13]], target: ~tracks[5]);
~instHat = Voicer(1, Instr.at(\sample), [\buf, ~kit808[7]], target: ~tracks[6]);
~fxReverb = ~sends[0].playfx(\vst);
~fxLimiter = ~master.playfx(\vst);

~tracks[0].newPostSend(~sends[0], 0.2);
~tracks[1].newPostSend(~sends[0], 0.2);
~tracks[2].newPostSend(~sends[0], 0.2);
~tracks[3].newPostSend(~sends[0], 0.2);
~tracks[5].newPostSend(~sends[0], 0.05);
~tracks[6].newPostSend(~sends[0], 0.05);
)

(
~reverbCtrl = VSTPluginController(~fxReverb);
~reverbCtrl.open('TAL Reverb 4 Plugin.vst3', editor: true, action: {|ctrl|
	ctrl.setn(0, [1.0, 1.0, 0.0, 0.0, 0.96153128147125, 1.0, 0.28570315241814, 0.7017987370491, 0.23301695287228, 0.3206875026226 ]);
});

~limiterCtrl = VSTPluginController(~fxLimiter);
~limiterCtrl.open('Pro-L 2.vst3', editor: true, action: {|ctrl|
	ctrl.setn(0, [0.0, 0.050000000745058, 0.44911110401154, 0.49661111831665, 0.036597222089767, 0.20691666007042, 0.0, 0.0, 0.0, 0.66666668653488, 0.0, 0.0, 0.0, 1.0, 0.0]);
});
)

(
~tuningPelog = {|note|
	var tuning = [585, 626, 700, 782, 856, 918, 1034];
	var octave = div(note, 12) - 1;
	var tone = note % 12;
	var hz = tuning[[0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6][tone]];
	hz * (2 ** (octave - 5));
};
~midi0 = VoicerMIDISocket([0, 0], ~inst0, nil, nil, nil, ~tuningPelog);
~midi1 = VoicerMIDISocket([0, 1], ~inst1, nil, nil, nil, ~tuningPelog);
~midi2 = VoicerMIDISocket([0, 2], ~inst2, nil, nil, nil, ~tuningPelog);
~midi3 = VoicerMIDISocket([0, 3], ~inst3, nil, nil, nil, ~tuningPelog);
~midiKick = VoicerMIDISocket([0, 9], ~instKick, 24, 24);
~midiSnare = VoicerMIDISocket([0, 9], ~instSnare, 26, 26);
~midiHat = VoicerMIDISocket([0, 9], ~instHat, 28, 28);
)

(
[~midi0, ~midi1, ~midi2, ~midi3].do({|v|
	v.midiToFreq_(nil);
})
)
(
[~midi0, ~midi1, ~midi2, ~midi3].do({|v|
	v.midiToFreq_(~tuningPelog);
})
)

~board = MixingBoard(mixers: ~tracks ++ ~sends ++ [~master]);
~reverbCtrl.editor;
~limiterCtrl.editor;