import { linlin } from './util.js';
import { ChromaticAberrationShader } from './ChromaticAberrationShader.js';
import * as THREE from 'https://unpkg.com/three@0.143.0/build/three.module.js';
import { EffectComposer } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/EffectComposer.js';
import { RenderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/ShaderPass.js';
import { UnrealBloomPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/UnrealBloomPass.js'; 

const DEG120 = 2 * Math.PI / 3;

let camera, scene, group, renderer, pipeline;
let lastFrame = 0;

const objects = {};

const palette = {
	blue: 0x3C4D70,
	red: 0xF63E06,
	purple: 0x977381,
	white: 0xFFFFFF,
}

const ccState = {
	kick: false,
	padfade: 0,
	padmod: 0,
	subfade: 0,
	rndfade: 0,
	rnddetune: 0,
	sidechain: 0,
};

export function initScene(canvasCtx) {
	scene = new THREE.Scene();

	camera = new THREE.OrthographicCamera(0, WIDTH, HEIGHT, 0, -1, 1000);
	scene.add(camera);

	group = new THREE.Group();
	scene.add(group);

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

	const bloomStrength = 1.5;
	const bloomRadius = 0;
	const bloomThreshold = 0;

	const renderPass = new RenderPass(scene, camera);
	const chromaticAberrationPass = new ShaderPass(ChromaticAberrationShader);
	const bloomPass = new UnrealBloomPass(
		new THREE.Vector2(WIDTH, HEIGHT),
		bloomStrength,
		bloomRadius,
		bloomThreshold,
	);

	const composer = new EffectComposer(renderer);
	composer.addPass(renderPass);
	composer.addPass(chromaticAberrationPass);
	composer.addPass(bloomPass);

	pipeline = { composer, renderPass, chromaticAberrationPass, bloomPass };

	objects.mtn = createTriangle({ size: 400, stroke: 50, color: palette.blue, opacity: 0.8 });
	objects.mtn.position.set(WIDTH/2, HEIGHT/3, 0);
	group.add(objects.mtn);

	objects.sidemtn1 = createSlicedTriangle( { size: 250, sliceSize: 125, side: 'right', color: palette.purple, opacity: 0.35 });
	objects.sidemtn1.position.set(WIDTH/2 - 400, HEIGHT/3 - (400/2) + (250/2), 0);
	group.add(objects.sidemtn1);

	objects.sidemtn2 = createSlicedTriangle( { size: 250, sliceSize: 125, side: 'left', color: palette.purple, opacity: 0.35 });
	objects.sidemtn2.position.set(WIDTH/2 + 400, HEIGHT/3 - (400/2) + (250/2), 0);
	group.add(objects.sidemtn2);
	
	objects.core = createTriangle({ size: 150, color: palette.purple, opacity: 0.75 });
	objects.core.position.set(WIDTH/2, HEIGHT/3, 0);
	group.add(objects.core);

	objects.core2 = createTriangle({ size: 275, stroke: 50, color: palette.purple, opacity: 0.5 });
	objects.core2.position.set(WIDTH/2, HEIGHT/3, 0);
	group.add(objects.core2);

	objects.sun = createSun({ size: 250, color: palette.red, opacity: 0.6 });
	objects.sun.position.set(WIDTH/2, HEIGHT/3 + 500, 0);
	group.add(objects.sun);

	objects.base = createSlicedTriangle({ size: 875, sliceSize: 850, side: 'top', color: palette.blue, opacity: 0.8 });
	objects.base.position.set(WIDTH/2, HEIGHT/3, 0);
	group.add(objects.base);

	objects.bg = createTriangle({ size: HEIGHT*2/3 + 4000, stroke: 4000, color: palette.red, opacity: 0.3 });
	objects.bg.position.set(WIDTH/2, HEIGHT/3, 0);
	group.add(objects.bg);

	objects.rnds = [...Array(8)].map((_, i) => {
		const obj = createTriangle({ size: 75, color: palette.blue, opacity: [0.8,0.8,0.7,0.7,0.6,0.6,0.5,0.5][i] });
		obj.position.set(WIDTH/2 + [-0.5, 0.5, -1.5, 1.5, -2.5, 2.5, -3.5, 3.5][i]*183, HEIGHT/3 - 315, 0);
		group.add(obj);
		return obj;
	});
}

function createTriangle({ size, stroke, color, opacity }) {
	const p0 = new THREE.Vector2(size * Math.sin(0), size * Math.cos(0));
	const p1 = new THREE.Vector2(size * Math.sin(DEG120), size * Math.cos(DEG120));
	const p2 = new THREE.Vector2(size * Math.sin(2*DEG120), size * Math.cos(2*DEG120));
	const shape = new THREE.Shape([p0, p1, p2, p0]);

	if (stroke) {
		const inner = size - stroke;
		const i0 = new THREE.Vector2(inner * Math.sin(0), inner * Math.cos(0));
		const i1 = new THREE.Vector2(inner * Math.sin(DEG120), inner * Math.cos(DEG120));
		const i2 = new THREE.Vector2(inner * Math.sin(2*DEG120), inner * Math.cos(2*DEG120));
		shape.holes.push(new THREE.Path([i0, i1, i2, i0]));
	}

	const geometry = new THREE.ShapeGeometry(shape);
	const mesh = new THREE.Mesh( geometry, new THREE.MeshBasicMaterial({
		side: THREE.FrontSide,
		transparent: true,
		color: new THREE.Color(color),
		opacity,
	}));
	return mesh;
}

function createSlicedTriangle({ size, sliceSize, side, stroke, color, opacity }) {
	const p0 = new THREE.Vector2(size * Math.sin(0), size * Math.cos(0));
	const p1 = new THREE.Vector2(size * Math.sin(DEG120), size * Math.cos(DEG120));
	const p2 = new THREE.Vector2(size * Math.sin(2*DEG120), size * Math.cos(2*DEG120));
	
	const sliceDeg = side === 'right' ? DEG120 :
		side === 'left' ? 2*DEG120 :
		0;

	const cx = size * Math.sin(sliceDeg) - sliceSize * Math.sin(sliceDeg);
	const cy = size * Math.cos(sliceDeg) - sliceSize * Math.cos(sliceDeg);

	const q0 = new THREE.Vector2(cx + sliceSize * Math.sin(0), cy + sliceSize * Math.cos(0));
	const q1 = new THREE.Vector2(cx + sliceSize * Math.sin(DEG120), cy + sliceSize * Math.cos(DEG120));
	const q2 = new THREE.Vector2(cx + sliceSize * Math.sin(2*DEG120), cy + sliceSize * Math.cos(2*DEG120));

	const shape = new THREE.Shape(
		side === 'right' ? [p0, q0, q2, p2, p0] :
		side === 'left' ? [p0, p1, q1, q0, p0] :
		[q1, p1, p2, q2, q1]
	);

	const geometry = new THREE.ShapeGeometry(shape);
	const mesh = new THREE.Mesh( geometry, new THREE.MeshBasicMaterial({
		side: THREE.FrontSide,
		transparent: true,
		color: new THREE.Color(color),
		opacity,
	}));
	return mesh;
}

function createSun({ size, color, opacity }) {
	const shape = new THREE.Shape()
		.absarc(0, 0, size, -Math.PI/3, 4*Math.PI/3, false)
		.lineTo(0, 0)

	const geometry = new THREE.ShapeGeometry(shape, 36);
	const mesh = new THREE.Mesh( geometry, new THREE.MeshBasicMaterial({
		side: THREE.FrontSide,
		transparent: true,
		color: new THREE.Color(color),
		opacity,
	}));
	return mesh;
}

export function drawFrame(frame) {
	pipeline.composer.render((frame - lastFrame)/FPS);
	lastFrame = frame;
	// tick();
}

function tick() {
	ccState.sidechain *= 0.98;

	objects.mtn.material.opacity = decay(objects.mtn.material.opacity);
	objects.sidemtn1.material.opacity = decay(objects.sidemtn1.material.opacity);
	objects.sidemtn2.material.opacity = decay(objects.sidemtn2.material.opacity);
	objects.core.material.opacity = decay(objects.core.material.opacity);
	objects.core2.material.opacity = decay(objects.core2.material.opacity);
	objects.bg.material.opacity = decay2(objects.bg.material.opacity);
	objects.sun.material.opacity = Math.max(0, linlin(ccState.padfade, 40, 127, 0, 1, true) * linlin(ccState.sidechain, 0, 0.9, 1, 0, true));
	objects.base.material.opacity = Math.max(0, linlin(ccState.subfade, 40, 127, 0, 1, true) * linlin(ccState.sidechain, 0.25, 1, 1, 0, true));
	objects.rnds.forEach(obj => obj.material.opacity = decay3(obj.material.opacity));

	pipeline.bloomPass.strength = linlin(ccState.rnddetune, 0, 127, 1.25, 2.0, true);
	pipeline.chromaticAberrationPass.uniforms.spread.value = linlin(ccState.padmod * linlin(ccState.sidechain, 0, 0.9, 1, 0, true), 0, 127, 0, 0.06, true);

	function decay(t) {
		if (t > 0.25) {
			return t * 0.92;
		} else {
			return t * 0.5;
		}
	}

	function decay2(t) {
		return t * 0.94;
	}

	function decay3(t) {
		if (t > 0.8) {
			return t * 0.9;
		}
		return t * 0.99;
	}
}

export function handleSongEvent(e) {
	switch (e.type) {
		case 'mtn':
			objects.mtn.material.opacity = 1;
			if (e.note > 29) {
				objects.mtn.material.color.set(palette.purple);
			} else {
				objects.mtn.material.color.set(palette.blue);
			}
			break;

		case 'rnd': {
			const index = e.note % 8;
			const high = e.note >= 8;

			objects.rnds[index].material.opacity = linlin(ccState.rndfade, 20, 127, 0, 1);
			if (high) {
				objects.rnds[index].material.color.set(palette.red);
			} else {
				objects.rnds[index].material.color.set(palette.blue);
			}
			break;
		}

		case 'base':
			objects.base.material.color.lerpColors(new THREE.Color(palette.blue), new THREE.Color(palette.purple), linlin(e.note, 29, 34, 0, 1, true));
			break;

		case 'snare':
			objects.core2.material.opacity = linlin(e.velocity, 31, 119, 0.4, 1.0, true);
			break;

		case 'hat':
			objects.core.material.opacity = linlin(e.velocity, 31, 119, 0.4, 1.0, true);
			break;

		case 'stones':
			objects.sidemtn1.material.opacity = linlin(e.velocity, 31, 119, 0.4, 1.0, true);
			objects.sidemtn2.material.opacity = linlin(e.velocity, 31, 119, 0.4, 1.0, true);
			break;

		case 'kick':
			if (ccState.kick) {
				objects.bg.material.opacity = linlin(e.velocity, 31, 119, 0.4, 1, true);
			}
			ccState.sidechain = 1;
			break;

		case 'cc_kick':
			ccState.kick = e.value;
			break;

		case 'cc_rnddetune':
			ccState.rnddetune = e.value;
			break;

		case 'cc_rndfade':
			ccState.rndfade = e.value;
			break;

		case 'cc_padfade':
			ccState.padfade = e.value;
			break;

		case 'cc_padmod':
			ccState.padmod = e.value;
			break;

		case 'cc_subfade':
			ccState.subfade = e.value;
			break;
	}
}
