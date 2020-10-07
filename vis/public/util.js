export function linlin(value, in1, in2, out1, out2, clamp = false) {
	if (clamp && value < in1) value = in1;
	if (clamp && value > in2) value = in2;
	const t = (value - in1) / (in2 - in1);
	return out1 + (out2 - out1) * t;
}

export function rebinFft(frequencyData, outputBinCount, minFreq, maxFreq, sampleRate) {
	const minFreqLn = Math.log(minFreq);
	const maxFreqLn = Math.log(maxFreq);
	const bandSizeLn = (maxFreqLn - minFreqLn) / outputBinCount;

	const bins = [];
	for (let i = 0; i < outputBinCount; ++i) {
		const bandLowLn = minFreqLn + i * bandSizeLn;
		const bandHighLn = minFreqLn + (i + 1) * bandSizeLn;
		const bandLow = Math.exp(bandLowLn);
		const bandHigh = Math.exp(bandHighLn);
		const power = getBandPower(frequencyData, bandLow, bandHigh, sampleRate);
		bins.push(power);
	}
	return bins;
}

export function getBandPower(frequencyData, bandLow, bandHigh, sampleRate) {
	const binCount = frequencyData.length;
	const hzPerBin = (sampleRate / 2) / binCount;

	const [indexLow, tLow] = frequencyToBinIndex(bandLow);
	const [indexHigh, tHigh] = frequencyToBinIndex(bandHigh);

	let power = 0;
	for (let i = indexLow; i <= indexHigh; ++i) {
		power += frequencyData[i];
		if (i === indexLow) {
			power -= frequencyData[i] * tLow;
		}
		if (i === indexHigh) {
			power -= frequencyData[i] * (1 - tHigh);
		}
	}
	return power;

	function frequencyToBinIndex(frequency) {
		const fractional = frequency / hzPerBin;
		const index = fractional | 0;
		const t = fractional % 1;
		return [index, t];
	}
}

export function powerToDb(power) {
	return 20*Math.log10(power);
}
