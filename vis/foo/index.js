import * as THREE from 'https://unpkg.com/three@0.121.1/build/three.module.js';
import { EffectComposer } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/ShaderPass.js';
import { UnrealBloomPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/UnrealBloomPass.js'; 
import { FilmPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/FilmPass.js'; 
import { VerticalTiltShiftShader } from './VerticalTiltShiftShader.js';
import * as hilbert from './hilbert.js';

const sampleRate = 44100;
const channels = 2;
const fftBuckets = 2**12;
const fftSize = fftBuckets * 2;
const smoothing = 0.2;
const minDb = -100;
const maxDb = 0;
const maxLength = sampleRate * 60 * 10;
const fps = 60;

const filepath = 'tetrik.flac';
const audioArrayBuffer = fetch(filepath).then(res => res.arrayBuffer());

const width = 1920;
const height = 1080;
const resolution = new THREE.Vector2(width, height);
const particleCount = 2**12;
const particlePositions = new Float32Array(particleCount * 3);
const particleScales = new Float32Array(particleCount);
const particleMinFreq = 20;
const particleMaxFreq = 20000;

const postprocessing = {};
let parent, particles;

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

function init() {
	const camera = new THREE.PerspectiveCamera(50, width / height, 0.01, 10000);
	window.camera = camera;
	camera.position.set(0, 400, 600);
	camera.lookAt(0, -150, 0);

	const scene = new THREE.Scene();

	const light = new THREE.AmbientLight(0xffffff, 1);
	scene.add(light);

	parent = new THREE.Object3D();
	parent.position.set(0, 0, 300);
	scene.add(parent);

	for (let n = 0; n < particleCount; ++n) {
		const [x, y] = hilbert.indexToCoords(particleCount, n);
		particlePositions[n*3] = linlin(x, 0, Math.sqrt(particleCount), -250, 250);
		particlePositions[n*3 + 1] = 0;
		particlePositions[n*3 + 2] = linlin(y, 0, Math.sqrt(particleCount), -250, 250);
		particleScales[n] = 1;
	}
	const particleGeometry = new THREE.BufferGeometry();
	particleGeometry.setAttribute('position', new THREE.BufferAttribute(particlePositions, 3));
	particleGeometry.setAttribute('scale', new THREE.BufferAttribute(particleScales, 1));
	const particleMaterial = new THREE.ShaderMaterial({
		uniforms: {
			color: { value: new THREE.Color(0xffffff) }
		},
		vertexShader: `
attribute float scale;
void main() {
	vec4 mvPosition = modelViewMatrix * vec4( position, 1.0 );
	gl_PointSize = scale * ( 300.0 / - mvPosition.z );
	gl_Position = projectionMatrix * mvPosition;
}
		`,
		fragmentShader: `
uniform vec3 color;
void main() {
	if ( length( gl_PointCoord - vec2( 0.5, 0.5 ) ) > 0.475 ) discard;
	gl_FragColor = vec4( color, 1.0 );
}
		`,
	});
	particles = new THREE.Points(particleGeometry, particleMaterial);
	scene.add(particles);

	const renderer = new THREE.WebGLRenderer({
		canvas: canvas,
		context: canvasCtx,
		antialias: true,
	});
	renderer.setPixelRatio(1);
	renderer.setSize(width, height);

	postprocessing.renderPass = new RenderPass(scene, camera);
	postprocessing.tiltShiftPass = new ShaderPass(VerticalTiltShiftShader);
	postprocessing.tiltShiftPass.uniforms.focusPos.value = 0.35;
	postprocessing.tiltShiftPass.uniforms.amount.value = 0.002;
	postprocessing.tiltShiftPass.uniforms.brightness.value = 0.8;
	postprocessing.bloomPass = new UnrealBloomPass(resolution, 1.5, 0.4, 0.85);
	postprocessing.filmPass = new FilmPass(0.35, 0.025, 648, false);
	postprocessing.filmPass.renderToScreen = true;

	postprocessing.composer = new EffectComposer(renderer);
	postprocessing.composer.addPass(postprocessing.renderPass);
	postprocessing.composer.addPass(postprocessing.tiltShiftPass);
	postprocessing.composer.addPass(postprocessing.bloomPass);
	postprocessing.composer.addPass(postprocessing.filmPass);
}

function drawFrame(frequencyData, timeData, frame) {
	let oscPoints = [];
	const downsampling = 8;
	for (let i = 0; i < timeData.length; i += downsampling) {
		oscPoints.push(new THREE.Vector3(
			linlin(i, 0, timeData.length - downsampling, -500, 500),
			linlin(timeData[i], -1, 1, -100, 100),
			0));
	}
	const oscCurve = new THREE.CatmullRomCurve3(oscPoints);
	const oscGeometry = new THREE.TubeBufferGeometry(oscCurve, 1024, 2, 4, false);
	const oscMaterial = new THREE.MeshStandardMaterial({ color: 0xffffff, flatShading: true });
	const oscMesh = new THREE.Mesh(oscGeometry, oscMaterial);
	parent.add(oscMesh);

	const logFrequencyData = rebinFft(frequencyData, particleCount, 40, 16000);

	const positions = particles.geometry.attributes.position.array;
	const scales = particles.geometry.attributes.scale.array;
	for (let i = 0; i < particleCount; ++i) {
		positions[i*3 + 1] = linlin(logFrequencyData[i], -100, -40, -100, 100, true);
		scales[i] = linlin(logFrequencyData[i], -100, -40, 1, 20, true);
	}
	particles.geometry.attributes.position.needsUpdate = true;
	particles.geometry.attributes.scale.needsUpdate = true;

	particles.rotation.y = (frame / fps) * 0.1;

	const powerLow = getBandPower(frequencyData, 20, 40);
	const powerHigh = getBandPower(frequencyData, 2000, 16000);
	postprocessing.filmPass.uniforms.nIntensity.value = linlin(powerHigh, -100, -40, 0, 0.75, true);
	postprocessing.filmPass.uniforms.sIntensity.value = linlin(powerLow, -60, -30, 0, 0.25, true);

	postprocessing.composer.render();

	parent.remove(oscMesh);

}

function rebinFft(frequencyData, binCount, minFreq, maxFreq) {
	const minFreqLn = Math.log(minFreq);
	const maxFreqLn = Math.log(maxFreq);
	const bandSizeLn = (maxFreqLn - minFreqLn) / binCount;

	const bins = [];
	for (let i = 0; i < binCount; ++i) {
		const bandLowLn = minFreqLn + i * bandSizeLn;
		const bandHighLn = minFreqLn + (i + 1) * bandSizeLn;
		const bandLow = Math.exp(bandLowLn);
		const bandHigh = Math.exp(bandHighLn);
		const power = getBandPower(frequencyData, bandLow, bandHigh);
		bins.push(power);
	}
	return bins;
}

function getBandPower(frequencyData, bandLow, bandHigh) {
	const binCount = frequencyData.length;
	const hzPerBin = (sampleRate / 2) / binCount;

	const [indexLow, tLow] = frequencyToBinIndex(bandLow);
	const [indexHigh, tHigh] = frequencyToBinIndex(bandHigh);

	let power = 0;
	for (let i = indexLow; i <= indexHigh; ++i) {
		const dataExp = 10**(frequencyData[i] / 20);
		power += dataExp;
		if (i === indexLow) {
			power -= dataExp * tLow;
		}
		if (i === indexHigh) {
			power -= dataExp * (1 - tHigh);
		}
	}
	power = 20*Math.log10(power);
	return power;

	function frequencyToBinIndex(frequency) {
		const fractional = frequency / hzPerBin;
		const index = fractional | 0;
		const t = fractional % 1;
		return [index, t];
	}
}

function linlin(value, in1, in2, out1, out2, clamp = false) {
	if (clamp && value < in1) value = in1;
	if (clamp && value > in2) value = in2;
	const t = (value - in1) / (in2 - in1);
	return out1 + (out2 - out1) * t;
}

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

async function renderOffline(inFrames = 0, outFrames = 0) {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: sampleRate,
		numberOfChannels: channels,
	});

	const { buffer, source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, false);
	source.start();

	const pixels = new Uint8Array(width * height * 4);

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
		console.log(`rendered in ${Date.now() - t0}ms`);
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

		const t = (frame - inFrames) / fps;
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

async function renderSingleFrame(frame, inFrames = 0) {
	const audioCtx = new OfflineAudioContext({
		length: maxLength,
		sampleRate: sampleRate,
		numberOfChannels: channels,
	});

	const { source, analyzer, frequencyData, timeData } = await setupAudioPipeline(audioCtx, false);
	source.start();

	if (frame >= inFrames) {
		const suspension = audioCtx.suspend((frame - inFrames) / fps);
		audioCtx.startRendering();
		await suspension;
		analyzer.getFloatFrequencyData(frequencyData);
		analyzer.getFloatTimeDomainData(timeData);
	}

	drawFrame(frequencyData, timeData, frame);
}

async function renderOnline(inFrames = 0) {
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

window.online = () => { renderOnline(90); };
window.offline = () => { renderOffline(90, 0); };

init();
renderSingleFrame(1000, 90);
