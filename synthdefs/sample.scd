SynthDef(\sample1, {|out, buf, amp = 1, rate = 1|
	var snd = PlayBuf.ar(1, buf, BufRateScale.kr(buf) * rate, doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;

SynthDef(\sample1_2, {|out, buf, amp = 1, rate = 1|
	var snd = PlayBuf.ar(1, buf!2, BufRateScale.kr(buf) * rate, doneAction: Done.freeSelf)!2;
	Out.ar(out, snd * amp);
}).add;

SynthDef(\sample2, {|out, buf, amp = 1, rate = 1|
	var snd = PlayBuf.ar(2, buf, BufRateScale.kr(buf) * rate, doneAction: Done.freeSelf);
	Out.ar(out, snd * amp);
}).add;
