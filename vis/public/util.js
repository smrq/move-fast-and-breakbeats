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

export const hilbert = (() => {
	function rot(n, x, y, rx, ry) {
		if (ry == 0) {
			if (rx == 1) {
				x = n - 1 - x;
				y = n - 1 - y;
			}
			return [y, x];
		} else {
			return [x, y];
		}
	}

	function coordsToIndex(n, x, y) {
		let d = 0;
		for (let s = n >> 1; s > 0; s >>= 1) {
			const rx = (x & s) ? 1 : 0;
			const ry = (y & s) ? 1 : 0;
			d += s * s * ((3 * rx) ^ ry);
			[x, y] = rot(n, x, y, rx, ry);
		}
		return d;
	}

	function indexToCoords(n, d) {
		let x = 0, y = 0, t = d;
		for (let s = 1; s < n; s <<= 1) {
			const rx = 1 & (t >> 1);
			const ry = 1 & (t ^ rx);
			[x, y] = rot(s, x, y, rx, ry);
			x += s * rx;
			y += s * ry;
			t = t >> 2;
		}
		return [x, y];
	}

	return { coordsToIndex, indexToCoords };
})();

export function downloadJson(obj, filename) {
	const blob = new Blob([JSON.stringify(obj)], { type: "text/json" });
	const link = document.createElement("a");
	link.download = filename;
	link.href = window.URL.createObjectURL(blob);
	link.dataset.downloadurl = ["text/json", link.download, link.href].join(":");
	const evt = new MouseEvent('click', {
		view: window,
		bubbles: true,
		cancelable: true,
	});
	link.dispatchEvent(evt);
	link.remove();
}
