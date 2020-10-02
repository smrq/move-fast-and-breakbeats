const sampleRate = 44100;
const channels = 2;
const fftSize = 2048;
const smoothing = 0.7;
const maxLength = sampleRate * 60 * 10;
const fps = 60;

const filepath = 'my own hostage.mp3';
const audioArrayBuffer = fetch(filepath).then(res => res.arrayBuffer());

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('2d', { alpha: false });
canvasCtx.fillStyle = 'rgb(0, 0, 0)';
canvasCtx.fillRect(0, 0, canvas.width, canvas.height);

function drawFrame(frequencyData, timeData, frame) {
	console.log('drawing frame ' + frame);

	canvasCtx.fillStyle = 'rgb(0, 0, 0)';
	canvasCtx.fillRect(0, 0, canvas.width, canvas.height);

	drawOscilloscope(timeData);
}

function drawOscilloscope(timeData) {
	canvasCtx.lineWidth = 1;
	canvasCtx.strokeStyle = 'rgb(255, 255, 255)';
	canvasCtx.shadowBlur = 40;
	canvasCtx.shadowColor = 'rgb(255, 255, 255)';
	canvasCtx.beginPath();

	const sliceWidth = canvas.width / timeData.length;
	let x = 0;
	for (let i = 0; i < timeData.length; ++i) {
		let v = timeData[i] * 200;
		let y = canvas.height / 2 + v;
		if (i === 0) {
			canvasCtx.moveTo(x, y);
		} else {
			canvasCtx.lineTo(x, y);
		}
		x += sliceWidth;
	}

	canvasCtx.lineTo(canvas.width, canvas.height / 2);
	canvasCtx.stroke();
}

async function renderOffline(startFrame = 1, endFrame = null) {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: sampleRate,
		numberOfChannels: channels,
	});

	const buffer = await audioArrayBuffer.then(audioCtx.decodeAudioData.bind(audioCtx));
	const source = audioCtx.createBufferSource();
	source.buffer = buffer;

	const analyzer = new AnalyserNode(audioCtx);
	analyzer.fftSize = fftSize;
	analyzer.smoothingTimeConstant = smoothing;
	const frequencyData = new Float32Array(analyzer.frequencyBinCount);
	const timeData = new Float32Array(fftSize);

	source.connect(analyzer);
	source.start();

	let frame = startFrame;
	audioCtx.suspend(frame / fps).then(handleSuspend);
	return audioCtx.startRendering().then(() => {
		console.log('rendering finished');
	});

	async function handleSuspend() {
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);

		drawFrame(frequencyData, timeData, frame);

		++frame;
		const t = frame / fps;
		if (endFrame == null ? t < buffer.duration : frame < endFrame) {
			audioCtx.suspend(t).then(handleSuspend);
		}
		audioCtx.resume();
	}
}

async function renderOnline(startFrame = 1) {
	const audioCtx = new AudioContext();

	const [buffer] = await Promise.all([
		audioArrayBuffer.then(audioCtx.decodeAudioData.bind(audioCtx)),
		StartAudioContext(audioCtx)
	]);
	const source = audioCtx.createBufferSource();
	source.buffer = buffer;

	const analyzer = new AnalyserNode(audioCtx);
	analyzer.fftSize = fftSize;
	analyzer.smoothingTimeConstant = smoothing;
	const frequencyData = new Float32Array(analyzer.frequencyBinCount);
	const timeData = new Float32Array(fftSize);

	source.connect(analyzer);
	analyzer.connect(audioCtx.destination);
	source.start();

	let ended = false;
	source.onended = () => { ended = true; };

	let frame = startFrame;
	requestAnimationFrame(handleRaf);

	function handleRaf() {
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);

		drawFrame(frequencyData, timeData, frame);

		++frame;
		if (!ended) {
			requestAnimationFrame(handleRaf);
		}
	}
}

function online() {
	renderOnline(0);
}

function offline() {
	renderOffline(1);
}

renderOffline(16000, 16000);
