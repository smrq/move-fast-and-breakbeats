import * as THREE from 'https://unpkg.com/three@0.143.0/build/three.module.js';
import { RenderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/RenderPass.js';
import { ShaderPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/ShaderPass.js';
import { UnrealBloomPass } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/UnrealBloomPass.js'; 
import { OrbitControls } from 'https://unpkg.com/three@0.143.0/examples/jsm/controls/OrbitControls.js';
import { Vis } from './Vis.js';
import { RgbGlitchShader } from '../shaders/RgbGlitchShader.js';
import { RippleShader } from '../shaders/RippleShader.js';
import { ScanlineGlitchShader } from '../shaders/ScanlineGlitchShader.js';
import { linlin } from '../util.js';

const palette = [
	0xFFFF00,
	0xFF0000,
	0x00FFFF,
	0xFF00FF,
	// 0xFFA600,
	// 0xFF6361,
	// 0x003F5C,
	// 0xBC5090,
];

export class VisNwwia extends Vis {
	constructor(context, midiState) {
		super(context);
		this.midiState = midiState;

		this.midiState.ch[1].cc.set(100, 0);
		this.midiState.ch[1].cc.set(101, 0);
		this.midiState.ch[1].cc.set(102, 0);
		this.midiState.ch[1].cc.set(103, 0);
		this.midiState.ch[1].cc.set(104, 99);
		this.midiState.ch[1].cc.set(105, 0);
		this.midiState.ch[1].cc.set(106, 0);
		this.midiState.ch[1].cc.set(107, 0);

		this.midiState.onNoteOn(1, this.handleBellNote.bind(this));
		this.midiState.onNoteOn(2, this.handleBassNote.bind(this));

		this.state.light0 = 0;
		this.state.light1 = 0;
		this.state.light2 = 0;
		this.state.light3 = 0;
	}

	createScene() { 
		const scene = new THREE.Scene();
		scene.background = new THREE.Color(0xffffff);

		const fov = 45;
		const aspect = WIDTH / HEIGHT;
		const near = 0.1;
		const far = 1000;
		const camera = new THREE.PerspectiveCamera(fov, aspect, near, far);
		camera.position.set(0, 0, 5);

		const cube = (() => {
			const geometry = new THREE.BoxGeometry(2, 2, 2);
			const material = new THREE.MeshPhongMaterial({
				color: 0xffffff,
				opacity: 0.5,
				transparent: true,
				side: THREE.DoubleSide,
			});
			return new THREE.Mesh(geometry, material);
		})();
		cube.rotation.z = Math.PI / 4;
		cube.rotation.x = Math.atan2(1, Math.sqrt(2));
		scene.add(cube);

		const cubeSilhouette = (() => {
			const geometry = new THREE.BoxGeometry(2, 2, 2);
			const material = new THREE.MeshPhongMaterial({
				color: 0x000000,
				side: THREE.BackSide,
			});
			return new THREE.Mesh(geometry, material);
		})();
		cubeSilhouette.rotation.z = Math.PI / 4;
		cubeSilhouette.rotation.x = Math.atan2(1, Math.sqrt(2));
		scene.add(cubeSilhouette);

		const light0 = new THREE.PointLight(palette[0], 1);
		light0.position.set(-5, 5, 2.5);
		scene.add(light0);

		const light1 = new THREE.PointLight(palette[1], 0.85);
		light1.position.set(5, 5, 2.5);
		scene.add(light1);

		const light2 = new THREE.PointLight(palette[2], 1);
		light2.position.set(5, -5, 2.5);
		scene.add(light2);

		const light3 = new THREE.PointLight(palette[3], 2);
		light3.position.set(-5, -5, 2.5);
		scene.add(light3);

		return {
			scene, camera,
			cube, cubeSilhouette,
			light0, light1, light2, light3,
		};
	}

	createRenderPipeline() {
		const renderPass = new RenderPass(this.refs.scene, this.refs.camera);

		const ripplePass1 = new ShaderPass(RippleShader);
		ripplePass1.uniforms.aspectRatio.value = WIDTH / HEIGHT;
		ripplePass1.uniforms.speed.value = 0.6;

		const rgbGlitchPass = new ShaderPass(RgbGlitchShader);
		rgbGlitchPass.uniforms.amount.value = 0;

		const ripplePass2 = new ShaderPass(RippleShader);
		ripplePass2.uniforms.aspectRatio.value = WIDTH / HEIGHT;
		ripplePass2.uniforms.speed.value = 0.3;

		const bloomStrength = 1.5;
		const bloomRadius = 0;
		const bloomThreshold = 0.5;
		const bloomPass = new UnrealBloomPass(
			new THREE.Vector2(WIDTH, HEIGHT),
			bloomStrength,
			bloomRadius,
			bloomThreshold,
		);

		const scanlineGlitchPass = new ShaderPass(ScanlineGlitchShader);

		this.composer.addPass(renderPass);
		this.composer.addPass(ripplePass1);
		this.composer.addPass(rgbGlitchPass);
		this.composer.addPass(ripplePass2);
		this.composer.addPass(bloomPass);
		this.composer.addPass(scanlineGlitchPass);

		return { renderPass, ripplePass1, ripplePass2, rgbGlitchPass, bloomPass, scanlineGlitchPass };
	}

	beforeFrame(frame) {
		this.refs.cube.rotateOnWorldAxis(new THREE.Vector3(0, 1, 0), 0.01);
		this.refs.cubeSilhouette.setRotationFromQuaternion(this.refs.cube.quaternion);

		this.refs.light0.intensity = linlin(this.state.light0, 0, 1, 0, 2);
		this.refs.light1.intensity = linlin(this.state.light1, 0, 1, 0, 2);
		this.refs.light2.intensity = linlin(this.state.light2, 0, 1, 0, 2);
		this.refs.light3.intensity = linlin(this.state.light3, 0, 1, 0, 2);

		this.pipeline.scanlineGlitchPass.uniforms.time.value = frame / FPS;
		this.pipeline.scanlineGlitchPass.uniforms.lines.value = linlin(this.midiState.ch[1].cc.get(105), 0, 127, 0, 0.2);
		this.pipeline.scanlineGlitchPass.uniforms.displacement.value = linlin(this.midiState.ch[1].cc.get(105), 0, 127, 0, 0.05);

		this.pipeline.rgbGlitchPass.uniforms.time.value = frame / FPS;
		this.pipeline.rgbGlitchPass.uniforms.amount.value =
			linlin(this.midiState.ch[1].cc.get(100), 0, 127, 0, 0.5) *
			linlin(this.midiState.ch[1].cc.get(105) + this.midiState.ch[1].cc.get(106), 0, 35, 0, 1, true);

		if (!this.state.ripple2 && this.midiState.ch[1].cc.get(103) === 127) {
			this.state.ripple2 = 1;
		}

		this.pipeline.ripplePass1.uniforms.time.value = this.state.ripple1 / FPS;
		this.pipeline.ripplePass2.uniforms.time.value = this.state.ripple2 / FPS;
	}

	afterFrame(frame) {
		this.state.light0 *= 0.99;
		this.state.light1 *= 0.99;
		this.state.light2 *= 0.99;
		this.state.light3 *= 0.99;

		if (this.state.ripple1) {
			++this.state.ripple1;
		}

		if (this.state.ripple2) {
			++this.state.ripple2;
		}
	}

	handleBellNote({ note }) {
		const amount = linlin(this.midiState.ch[1].cc.get(106), 20, 127, 0, 1);
		const decay = linlin(this.midiState.ch[1].cc.get(106), 20, 127, 1, 0.5);

		switch (note) {
			case 67:
				this.state.light0 = amount;
				this.state.light1 *= decay;
				this.state.light2 *= decay;
				this.state.light3 *= decay;
				break;
			case 65:
				this.state.light1 = amount;
				this.state.light0 *= decay;
				this.state.light2 *= decay;
				this.state.light3 *= decay;
				break;
			case 60:
				this.state.light2 = amount;
				this.state.light0 *= decay;
				this.state.light1 *= decay;
				this.state.light3 *= decay;
				break;
			case 53:
				this.state.light3 = amount;
				this.state.light0 *= decay;
				this.state.light1 *= decay;
				this.state.light2 *= decay;
				break;
		}
	}

	handleBassNote() {
		this.state.ripple1 = 1;
	}
}
