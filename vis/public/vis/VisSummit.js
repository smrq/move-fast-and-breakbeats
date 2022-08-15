import * as THREE from 'https://unpkg.com/three@0.143.0/build/three.module.js';
import { RenderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/ShaderPass.js';
import { UnrealBloomPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/UnrealBloomPass.js'; 
import { Vis } from './Vis.js';
import { ChromaticAberrationShader } from '../shaders/ChromaticAberrationShader.js';
import { linlin } from '../util.js';

const DEG120 = 2 * Math.PI / 3;

const palette = {
	blue: 0x3C4D70,
	red: 0xF63E06,
	purple: 0x977381,
	white: 0xFFFFFF,
}

export class VisSummit extends Vis {
	constructor(context, midiState) {
		super(context);

		this.midiState = midiState;
		this.midiState.ch[1].cc.set(100, 0);
		this.midiState.ch[3].cc.set(101, 0);
		this.midiState.ch[3].cc.set(102, 0);
		this.midiState.ch[4].cc.set(104, 0);
		this.midiState.ch[4].cc.set(105, 0);
		this.midiState.ch[5].cc.set(106, 0);

		this.midiState.onNoteOn(1, this.handleDrumNote.bind(this));
		this.midiState.onNoteOn(2, this.handlePluckNote.bind(this));
		this.midiState.onNoteOn(3, this.handleRndNote.bind(this));
		this.midiState.onNoteOn(5, this.handleSubNote.bind(this));

		this.state.sidechain = 0;
	}

	createScene() {
		const scene = new THREE.Scene();

		const camera = new THREE.OrthographicCamera(0, WIDTH, HEIGHT, 0, -1, 1000);
		scene.add(camera);

		const group = new THREE.Group();
		scene.add(group);

		const mtn = createTriangle({ size: 400, stroke: 50, color: palette.blue, opacity: 0.8 });
		mtn.position.set(WIDTH/2, HEIGHT/3, 0);
		group.add(mtn);

		const sidemtn1 = createSlicedTriangle({ size: 250, sliceSize: 125, side: 'right', color: palette.purple, opacity: 0.35 });
		sidemtn1.position.set(WIDTH/2 - 400, HEIGHT/3 - (400/2) + (250/2), 0);
		group.add(sidemtn1);

		const sidemtn2 = createSlicedTriangle({ size: 250, sliceSize: 125, side: 'left', color: palette.purple, opacity: 0.35 });
		sidemtn2.position.set(WIDTH/2 + 400, HEIGHT/3 - (400/2) + (250/2), 0);
		group.add(sidemtn2);
		
		const core = createTriangle({ size: 150, color: palette.purple, opacity: 0.75 });
		core.position.set(WIDTH/2, HEIGHT/3, 0);
		group.add(core);

		const core2 = createTriangle({ size: 275, stroke: 50, color: palette.purple, opacity: 0.5 });
		core2.position.set(WIDTH/2, HEIGHT/3, 0);
		group.add(core2);

		const sun = createSun({ size: 250, color: palette.red, opacity: 0.6 });
		sun.position.set(WIDTH/2, HEIGHT/3 + 500, 0);
		group.add(sun);

		const base = createSlicedTriangle({ size: 875, sliceSize: 850, side: 'top', color: palette.blue, opacity: 0.8 });
		base.position.set(WIDTH/2, HEIGHT/3, 0);
		group.add(base);

		const bg = createTriangle({ size: HEIGHT*2/3 + 4000, stroke: 4000, color: palette.red, opacity: 0.3 });
		bg.position.set(WIDTH/2, HEIGHT/3, 0);
		group.add(bg);

		const rnds = [...Array(8)].map((_, i) => {
			const obj = createTriangle({ size: 75, color: palette.blue, opacity: [0.8,0.8,0.7,0.7,0.6,0.6,0.5,0.5][i] });
			obj.position.set(WIDTH/2 + [-0.5, 0.5, -1.5, 1.5, -2.5, 2.5, -3.5, 3.5][i]*183, HEIGHT/3 - 315, 0);
			group.add(obj);
			return obj;
		});

		return {
			scene, camera, group,
			mtn, sidemtn1, sidemtn2, core, core2, sun, base, bg, rnds
		};
	}

	createRenderPipeline() {
		const renderPass = new RenderPass(this.refs.scene, this.refs.camera);
		const chromaticAberrationPass = new ShaderPass(ChromaticAberrationShader);

		const bloomStrength = 1.5;
		const bloomRadius = 0;
		const bloomThreshold = 0;
		const bloomPass = new UnrealBloomPass(
			new THREE.Vector2(WIDTH, HEIGHT),
			bloomStrength,
			bloomRadius,
			bloomThreshold,
		);

		this.composer.addPass(renderPass);
		this.composer.addPass(chromaticAberrationPass);
		this.composer.addPass(bloomPass);

		return { renderPass, chromaticAberrationPass, bloomPass };
	}

	beforeFrame() {
		const rnddetune = this.midiState.ch[3].cc.get(101);
		const padfade = this.midiState.ch[4].cc.get(104);
		const padmod = this.midiState.ch[4].cc.get(105);
		const subfade = this.midiState.ch[5].cc.get(106);

		this.refs.sun.material.opacity = Math.max(0, linlin(padfade, 40, 127, 0, 1, true) * linlin(this.state.sidechain, 0, 0.9, 1, 0, true));
		this.refs.base.material.opacity = Math.max(0, linlin(subfade, 40, 127, 0, 1, true) * linlin(this.state.sidechain, 0.25, 1, 1, 0, true));
		this.pipeline.bloomPass.strength = linlin(rnddetune, 0, 127, 1.25, 2.0, true);
		this.pipeline.chromaticAberrationPass.uniforms.spread.value = linlin(padmod * linlin(this.state.sidechain, 0, 0.9, 1, 0, true), 0, 127, 0, 0.06, true);
	}

	afterFrame() {
		this.state.sidechain *= 0.98;

		this.refs.mtn.material.opacity = decay(this.refs.mtn.material.opacity);
		this.refs.sidemtn1.material.opacity = decay(this.refs.sidemtn1.material.opacity);
		this.refs.sidemtn2.material.opacity = decay(this.refs.sidemtn2.material.opacity);
		this.refs.core.material.opacity = decay(this.refs.core.material.opacity);
		this.refs.core2.material.opacity = decay(this.refs.core2.material.opacity);
		this.refs.bg.material.opacity = decay2(this.refs.bg.material.opacity);
		this.refs.rnds.forEach(obj => obj.material.opacity = decay3(obj.material.opacity));

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

	handleDrumNote({ note, velocity }) {
		switch (note) {
			case 36:
				if (this.midiState.ch[1].cc.get(100) > 0) {
					this.refs.bg.material.opacity = linlin(velocity, 31, 119, 0.4, 1, true);
				}
				this.state.sidechain = 1;
				break;

			case 41:
				this.refs.core.material.opacity = linlin(velocity, 31, 119, 0.4, 1.0, true);
				break;

			case 43:
				this.refs.core2.material.opacity = linlin(velocity, 31, 119, 0.4, 1.0, true);
				break;

			case 48:
			case 50:
			case 52:
			case 53:
				this.refs.sidemtn1.material.opacity = linlin(velocity, 31, 119, 0.4, 1.0, true);
				this.refs.sidemtn2.material.opacity = linlin(velocity, 31, 119, 0.4, 1.0, true);
				break;
		}
	}

	handlePluckNote({ note }) {
		this.refs.mtn.material.opacity = 1;
		if (note > 29) {
			this.refs.mtn.material.color.set(palette.purple);
		} else {
			this.refs.mtn.material.color.set(palette.blue);
		}			
	}
	
	handleRndNote({ note }) {
		const i = [0, 3, 5, 7, 10, 12].findIndex(x => x >= note % 12);
		const o = ((note - 36) / 12) | 0;
		const n = i + 5*o;

		const index = n % 8;
		const high = n >= 8;

		const rndfade = this.midiState.ch[3].cc.get(102);
		this.refs.rnds[index].material.opacity = linlin(rndfade, 20, 127, 0, 1);
		if (high) {
			this.refs.rnds[index].material.color.set(palette.red);
		} else {
			this.refs.rnds[index].material.color.set(palette.blue);
		}
	}

	handleSubNote({ note }) {
		this.refs.base.material.color.lerpColors(
			new THREE.Color(palette.blue),
			new THREE.Color(palette.purple),
			linlin(note, 29, 34, 0, 1, true));
	}
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
