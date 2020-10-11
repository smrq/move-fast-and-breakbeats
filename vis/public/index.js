import { linlin } from './util.js';
import { initScene, drawFrame } from './vis.js';
import { Analyzer } from './analyzer.js';
import { TarWriter } from './tarball.js';

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

window.WIDTH = canvas.width;
window.HEIGHT = canvas.height;
window.SAMPLE_RATE = 44100;
window.FPS = 60;

const inFrames = 45;
const framesPerArchive = 100;

const fftSize = 2**12 * 2;
const smoothing = 0.6;
const audioDelaySamples = fftSize * 6/8;

const filepath = 'media/tetrik.flac';
const audioArrayBuffer = fetch(filepath).then(res => res.arrayBuffer());
const stereoData = audioArrayBuffer
	.then(buf => {
		const audioCtx = new OfflineAudioContext({ length: 1, sampleRate: SAMPLE_RATE, numberOfChannels: 2 });
		return audioCtx.decodeAudioData(buf.slice());
	})
	.then(data => [0, 1].map(c => data.getChannelData(c)));
const monoData = stereoData.then(stereoData => {
		const monoData = new Float32Array(stereoData[0].length);
		for (let i = 0; i < monoData.length; ++i) {
			monoData[i] = 0.5 * (stereoData[0][i] + stereoData[1][i]);
		}
		return monoData;
	});

stereoData.then(stereo => {
	window.stereo = stereo;
});
monoData.then(mono => {
	window.mono = mono;
	window.analyzer = new Analyzer(mono, fftSize, smoothing);
});

initScene(canvasCtx);
renderSingleFrame(10000);

async function renderSingleFrame(frame) {
	const signal = await monoData;
	const analyzer = new Analyzer(signal, fftSize, smoothing);

	const lastSample = Math.floor((frame - inFrames) * SAMPLE_RATE / FPS);
	const [timeData, frequencyData] = analyzer.analyze(lastSample + audioDelaySamples);
	drawFrame(frequencyData, timeData, frame);
}

async function renderOffline(startFrame = 0, endFrame = null) {
	const signal = await monoData;
	const analyzer = new Analyzer(signal, fftSize, smoothing);

	if (!endFrame) {
		endFrame = inFrames + Math.floor(signal.length * FPS / SAMPLE_RATE);
	}

	console.log(`Rendering frames ${startFrame} - ${endFrame-1}`);

	let archive = new TarWriter();
	let frame = startFrame;
	for (; frame < endFrame; ++frame) {
		const t0 = Date.now();
		const lastSample = Math.floor((frame - inFrames) * SAMPLE_RATE / FPS);
		const [timeData, frequencyData] = analyzer.analyze(lastSample + audioDelaySamples);
		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		console.log(`frame ${frame} / ${endFrame-1} done in ${Date.now() - t0}ms`);
	}
	saveArchive(frame);

	async function saveFrame(frame) {
		const filename = `frame_${String(frame).padStart(6, '0')}.png`;
		const blob = await new Promise(resolve => {
			canvas.toBlob(blob => resolve(blob), 'image/png');
		});
		archive.addFile(filename, blob);

		if ((frame + 1) % framesPerArchive === 0) {
			await saveArchive(frame);
			archive = new TarWriter();
		}
	}

	async function saveArchive(frame) {
		const t0 = Date.now();
		const filename = `frames_${String(frame).padStart(6, '0')}.tar`;
		await archive.download(filename);
		console.log(`saved ${filename} in ${Date.now() - t0}ms`);
	}
}

async function renderOnline() {
	const audioCtx = new AudioContext();
	const [buffer] = await Promise.all([
		audioArrayBuffer.then(b => b.slice()).then(audioCtx.decodeAudioData.bind(audioCtx)),
		StartAudioContext(audioCtx)
	]);
	const source = audioCtx.createBufferSource();
	source.buffer = buffer;

	const analyzer = new AnalyserNode(audioCtx, {
		fftSize: fftSize,
		smoothingTimeConstant: smoothing,
	});

	const delay = new DelayNode(audioCtx, {
		delayTime: audioDelaySamples / SAMPLE_RATE,
	});

	const frequencyData = new Float32Array(analyzer.frequencyBinCount);
	const timeData = new Float32Array(fftSize);

	source.connect(analyzer);
	analyzer.connect(delay);
	delay.connect(audioCtx.destination);

	let frame = 0;
	requestAnimationFrame(handleRaf);

	function handleRaf() {
		if (frame == inFrames) {
			source.start();
		}

		analyzer.getFloatTimeDomainData(timeData);
		analyzer.getFloatFrequencyData(frequencyData);
		for (let i = 0; i < frequencyData.length; ++i) {
			frequencyData[i] = 10**(frequencyData[i] / 20);
		}
		drawFrame(frequencyData, timeData, frame++);
		requestAnimationFrame(handleRaf);
	}
}

window.offline = () => {
	const startFrame = parseInt(document.getElementById('startframe').value, 10);

	let endFrame = document.getElementById('startframe').value;
	if (endFrame) endFrame = parseInt(endFrame, 10);

	renderOffline(startFrame, endFrame);
};

window.online = renderOnline;