function createBlackmanWindow(alpha, size) {
	const arr = new Float32Array(size);
	const a0 = (1 - alpha) / 2;
	const a1 = 1/2;
	const a2 = alpha / 2;
	for (let i = 0; i < size; ++i) {
		arr[i] = a0 - (a1 * Math.cos(2*Math.PI*i / size)) + (a2 * Math.cos(4*Math.PI*i / size));
	}
	return arr;
}

export class Analyzer {
	constructor(signal, fftSize, smoothing) {
		this.signal = signal;
		this.fftSize = fftSize;
		this.smoothing = smoothing;

		this.fft = new FFTJS(fftSize);
		this.blackman = createBlackmanWindow(0.16, fftSize);
		this.fftInput = new Float32Array(fftSize);
		this.fftOutput = new Float32Array(fftSize * 2);
		this.timeResult = new Float32Array(fftSize);
		this.freqResult = new Float32Array(fftSize / 2).fill(0);
		this.lastFreqResult = new Float32Array(fftSize / 2).fill(0);
	}

	analyze(lastSample) {
		[this.lastFreqResult, this.freqResult] = [this.freqResult, this.lastFreqResult];

		const firstSample = lastSample - this.fftSize + 1;
		for (let i = 0; i < this.fftSize; ++i) {
			this.timeResult[i] = this.signal[i + firstSample] || 0;
			this.fftInput[i] = this.blackman[i] * this.timeResult[i];
		}

		this.fft.realTransform(this.fftOutput, this.fftInput);

		for (let i = 0; i < this.fftSize / 2; ++i) {
			this.freqResult[i] = Math.sqrt(this.fftOutput[2*i]**2 + this.fftOutput[2*i+1]**2) / this.fftSize;
			this.freqResult[i] = this.smoothing * this.lastFreqResult[i] + (1 - this.smoothing) * this.freqResult[i];
		}

		return [this.timeResult, this.freqResult];
	}

	resetSmoothing() {
		this.freqResult.fill(0);
		this.lastFreqResult.fill(0);
	}
}
