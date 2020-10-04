import { linlin } from './util.js';
import { initScene, drawFrame } from './vis.js';

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

window.WIDTH = canvas.width;
window.HEIGHT = canvas.height;
window.SAMPLE_RATE = 44100;
window.FPS = 60;

const inFrames = 90;
const outFrames = 0;

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
renderSingleFrame(1000, 90);

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

window.offline = async function renderOffline() {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: SAMPLE_RATE,
		numberOfChannels: channels,
	});

	const { buffer, source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, false);
	source.start();

	const pixels = new Uint8Array(WIDTH * HEIGHT * 4);

	const requestedBytes = 1024*1024*1024*100;
	const grantedBytes = await new Promise((resolve, reject) => {
		navigator.webkitPersistentStorage.requestQuota(requestedBytes, grantedBytes => resolve(grantedBytes), error => reject(error));
	});
	const fs = await new Promise((resolve, reject) => {
		window.webkitRequestFileSystem(PERSISTENT, grantedBytes, fs => resolve(fs), error => reject(error));
	});

	let frame = 0;
	let t0;

	for (let i = 0; i < inFrames; ++i) {
		console.log(`drawing frame ${frame} (inframe)`);
		t0 = Date.now();
		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		++frame;
	}

	const suspension0 = audioCtx.suspend(0);
	audioCtx.startRendering();
	await suspension0;

	while (true) {
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);
		console.log(`drawing frame ${frame}`);
		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		++frame;

		const t = (frame - inFrames) / FPS;
		if (t < buffer.duration) {
			const suspension = audioCtx.suspend(t);
			audioCtx.resume();
			await suspension;
		} else {
			frequencyData.fill(minDb);
			timeData.fill(0);
			break;
		}
	}

	for (let i = 0; i < outFrames; ++i) {
		console.log(`drawing frame ${frame} (outframe)`);
		drawFrame(frequencyData, timeData, frame);
		await saveFrame(frame);
		++frame;
	}

	async function saveFrame(frame) {
		t0 = Date.now();
		const filename = `frame_${String(frame).padStart(6, '0')}.png`;
		return new Promise(resolve => {
			canvas.toBlob(blob => {
				console.log(`blobbed ${blob.size} bytes in ${Date.now() - t0}ms`);
				fs.root.getFile(filename, { create: true }, entry => {
					entry.createWriter(writer => {
						writer.seek(0);
						writer.write(blob);
						console.log(`saved frame ${frame} in ${Date.now() - t0}ms`);
						resolve();
					});
				});
			}, 'image/png');
		});
	}
}

window.online = async function renderOnline() {
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
