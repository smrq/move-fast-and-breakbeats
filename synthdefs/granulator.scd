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