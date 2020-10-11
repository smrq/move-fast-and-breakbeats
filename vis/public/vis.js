import * as THREE from 'https://unpkg.com/three@0.121.1/build/three.module.js';
import { EffectComposer } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/ShaderPass.js';
import { UnrealBloomPass } from 'https://unpkg.com/three@0.121.1/examples/jsm/postprocessing/UnrealBloomPass.js'; 
import { ParticleShader } from './ParticleShader.js';
import { FilmPass } from './FilmPass.js'; 
import { VerticalTiltShiftShader } from './VerticalTiltShiftShader.js';
import { linlin, rebinFft, getBandPower, powerToDb } from './util.js';
import * as hilbert from './hilbert.js';

const particleCount = 2**12;
const particlePositions = new Float32Array(particleCount * 3);
const particleScales = new Float32Array(particleCount);
const particleMinFreq = 20;
const particleMaxFreq = 18000;

let camera, scene, light, oscContainer, particles, renderer, pipeline;
let lastFrame = 0;

export function initScene(canvasCtx) {
	camera = new THREE.PerspectiveCamera(50, WIDTH / HEIGHT, 0.01, 10000);
	camera.position.set(0, 400, 600);
	camera.lookAt(0, -150, 0);

	scene = new THREE.Scene();

	light = new THREE.AmbientLight(0xffffff, 1);
	scene.add(light);

	oscContainer = new THREE.Object3D();
	oscContainer.position.set(0, 0, 300);
	scene.add(oscContainer);

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
	const particleMaterial = new THREE.ShaderMaterial(ParticleShader);
	particleMaterial.uniforms.color.value = new THREE.Color(0xffffff);
	particleMaterial.uniforms.resolution.value = new THREE.Vector2(WIDTH, HEIGHT);
	particles = new THREE.Points(particleGeometry, particleMaterial);
	scene.add(particles);

	renderer = new THREE.WebGLRenderer({
		context: canvasCtx,
		antialias: true,
	});
	renderer.setPixelRatio(1);
	renderer.setSize(WIDTH, HEIGHT);

	const bufferTexture = new THREE.WebGLRenderTarget(WIDTH, HEIGHT, {
		minFilter: THREE.LinearFilter,
		magFilter: THREE.LinearFilter,
		stencilBuffer: false,
	});

	const renderPass = new RenderPass(scene, camera);
	const tiltShiftPass = new ShaderPass(VerticalTiltShiftShader);
	tiltShiftPass.uniforms.focusPos.value = 0.35;
	tiltShiftPass.uniforms.amount.value = 0.002;
	tiltShiftPass.uniforms.brightness.value = 0.8;
	const bloomPass = new UnrealBloomPass(
		new THREE.Vector2(WIDTH, HEIGHT),
		1.5,
		0.4,
		0.85);
	const filmPass = new FilmPass(0, 0, 648, false);

	const composer = new EffectComposer(renderer);
	composer.addPass(renderPass);
	composer.addPass(tiltShiftPass);
	composer.addPass(bloomPass);
	composer.addPass(filmPass);

	pipeline = { composer, renderPass, tiltShiftPass, bloomPass, filmPass };
}

export function drawFrame(frequencyData, timeData, frame) {
	const oscMesh = createOscMesh(timeData);
	oscContainer.add(oscMesh);
	updateSpectrumPoints(frequencyData, frame);
	updateShaders(frequencyData);
	pipeline.composer.render((frame - lastFrame)/FPS);
	oscContainer.remove(oscMesh);
	lastFrame = frame;
}

function createOscMesh(timeData) {
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
	return oscMesh;
}

function updateSpectrumPoints(frequencyData, frame) {
	const rebinned = rebinFft(frequencyData, particleCount, 40, 16000, SAMPLE_RATE);
	const positions = particles.geometry.attributes.position.array;
	const scales = particles.geometry.attributes.scale.array;
	for (let i = 0; i < particleCount; ++i) {
		positions[i*3 + 1] = linlin(powerToDb(rebinned[i]), -100, -40, -100, 100, true);
		scales[i] = linlin(powerToDb(rebinned[i]), -100, -40, 1, 4, true);
	}
	particles.geometry.attributes.position.needsUpdate = true;
	particles.geometry.attributes.scale.needsUpdate = true;
	particles.rotation.y = (frame / FPS) * 0.1;
}

function updateShaders(frequencyData) {
	const powerLow = getBandPower(frequencyData, 20, 40, SAMPLE_RATE);
	const powerHigh = getBandPower(frequencyData, 2000, 16000, SAMPLE_RATE);
	pipeline.filmPass.uniforms.sIntensity.value = linlin(powerToDb(powerLow), -60, -30, 0, 0.25, true);
	pipeline.filmPass.uniforms.nIntensity.value = linlin(powerToDb(powerHigh), -100, -30, 0, 0.75, true);
}
