SynthDef.new(\audioIn, {|in = 0, out = 0, amp = 1|
	var sig = SoundIn.ar(in) * amp;
	Out.ar(out, sig);
}).add;

SynthDef.new(\audioIn2, {|in = 0, out = 0, amp = 1|
	var sig = SoundIn.ar(in) * amp;
	Out.ar(out, sig!2);
}).add;

SynthDef.new(\mono2stereo, {|in=0, out=0|
	var sig = In.ar(in, 1);
	Out.ar(out, sig!2);
}).add;

SynthDef.new(\patch1, {|in=0, out=0|
	Out.ar(out, In.ar(in, 1));
}).add;

SynthDef.new(\patch2, {|in=0, out=0|
	Out.ar(out, In.ar(in, 2));
}).add;