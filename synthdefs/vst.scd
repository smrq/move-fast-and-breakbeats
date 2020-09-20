SynthDef.new(\vstinsert, {|bus=0, bypass|
	ReplaceOut.ar(bus, VSTPlugin.ar(In.ar(bus, 2), 2));
}).add;

SynthDef.new(\vst, {|in=0, out=0, bypass|
	Out.ar(out, VSTPlugin.ar(In.ar(in, 2), 2));
}).add;