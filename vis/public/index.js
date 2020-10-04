import { linlin } from './util.js';
import { initScene, drawFrame } from './vis.js';
import { TarWriter } from './tarball.js';

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

window.WIDTH = canvas.width;
window.HEIGHT = canvas.height;
window.SAMPLE_RATE = 44100;
window.FPS = 60;

const inFrames = 90;

const channels = 2;
const fftBuckets = 2**12;
const fftSize = fftBuckets * 2;
const smoothing = 0.2;
const minDb = -100;
const maxDb = 0;
const maxLength = SAMPLE_RATE * 60 * 10;

const filepath = 'media/tetrik.flac';
const audioArrayBuffer = fetch(filepath).then(res => res.arrayBuffer());

initScene(canvasCtx);
// renderSingleFrame(20616, 90);

async function setupAudioPipeline(audioCtx, interactive) {
	const [buffer] = await Promise.all([
		audioArrayBuffer.then(b => b.slice()).then(audioCtx.decodeAudioData.bind(audioCtx)),
		interactive ? StartAudioContext(audioCtx) : Promise.resolve()
	]);
	const source = audioCtx.createBufferSource();
	source.buffer = buffer;

	const analyzer = new AnalyserNode(audioCtx);
	analyzer.fftSize = fftSize;
	analyzer.smoothingTimeConstant = smoothing;
	analyzer.minDecibels = minDb;
	analyzer.maxDecibels = maxDb;
	const frequencyData = new Float32Array(analyzer.frequencyBinCount).fill(minDb);
	const timeData = new Float32Array(fftSize).fill(0);

	source.connect(analyzer);
	if (interactive) {
		analyzer.connect(audioCtx.destination);
	}

	return { buffer, source, analyzer, frequencyData, timeData };
}

async function renderSingleFrame(frame) {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: SAMPLE_RATE,
		numberOfChannels: channels,
	});

	const { source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, false);
	source.start();

	if (frame >= inFrames) {
		const suspension = audioCtx.suspend((frame - inFrames) / FPS);
		audioCtx.startRendering();
		await suspension;
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);
	}

	drawFrame(frequencyData, timeData, frame);
}

async function renderOffline(startFrame = 0) {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: SAMPLE_RATE,
		numberOfChannels: channels,
	});

	const { buffer, source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, false);
	const audioFrames = Math.floor(buffer.duration * FPS);
	const totalFrames = inFrames + audioFrames;
	source.start();

	const archiveSize = 100;
	let archive = new TarWriter();
	let frame = startFrame;
	let t0;

	while (frame < inFrames) {
		t0 = Date.now();

		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		++frame;

		console.log(`inframe ${frame} / ${totalFrames} done in ${Date.now() - t0}ms`);
	}

	const suspension0 = audioCtx.suspend(0);
	audioCtx.startRendering();
	await suspension0;

	while (frame < inFrames + audioFrames) {
		t0 = Date.now();

		const suspension = audioCtx.suspend((frame - inFrames) / FPS);
		audioCtx.resume();
		await suspension;
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);

		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		++frame;

		console.log(`audio frame ${frame} / ${totalFrames} done in ${Date.now() - t0}ms`);
	}

	saveArchive();

	async function saveFrame(frame) {
		const filename = `frame_${String(frame).padStart(6, '0')}.png`;
		const blob = await new Promise(resolve => {
			canvas.toBlob(blob => resolve(blob), 'image/png');
		});
		archive.addFile(filename, blob);

		if ((frame + 1) % archiveSize === 0) {
			await saveArchive();
			archive = new TarWriter();
		}
	}

	async function saveArchive() {
		const t0 = Date.now();
		const archiveIndex = frame / archiveSize | 0;
		const filename = `frames_${String(archiveIndex).padStart(4, '0')}.tar`;
		await archive.download(filename);
		console.log(`saved ${filename} in ${Date.now() - t0}ms`);
	}
}

async function renderOnline() {
	const audioCtx = new AudioContext();
	const { source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, true);

	let frame = 0;
	requestAnimationFrame(handleRaf);

	function handleRaf() {
		if (frame == inFrames) {
			source.start();
		}

		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);
		drawFrame(frequencyData, timeData, frame++);
		requestAnimationFrame(handleRaf);
	}
}

window.offline = () => {
	const startFrame = parseInt(document.getElementById('startframe').value, 10);
	renderOffline(startFrame);
};

window.online = renderOnline;