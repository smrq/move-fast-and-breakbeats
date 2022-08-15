import * as THREE from 'https://unpkg.com/three@0.143.0/build/three.module.js';
import { EffectComposer } from 'https://unpkg.com/three@0.143.0/examples/jsm/postprocessing/EffectComposer.js';

export class Vis {
	constructor(context) {
		if (this.constructor === Vis) {
			throw new Error('abstract class');
		}

		this.renderer = new THREE.WebGLRenderer({
			context: context,
			antialias: true,
		});
		this.renderer.setPixelRatio(1);
		this.renderer.setSize(WIDTH, HEIGHT);

		this.composer = new EffectComposer(this.renderer);

		this.lastFrame = 0;

		this.refs = null;
		this.pipeline = null;
		
		this.state = {};
	}

	init() {
		this.refs = this.createScene();
		this.pipeline = this.createRenderPipeline();
	}

	createScene() {
		throw new Error('unimplemented abstract method');
	}

	createRenderPipeline() {
		throw new Error('unimplemented abstract method');
	}

	drawFrame(frame) {
		this.beforeFrame(frame);
		this.composer.render((frame - this.lastFrame)/FPS);
		this.afterFrame(frame);
		this.lastFrame = frame;
	}

	beforeFrame() {
		throw new Error('unimplemented abstract method');
	}

	afterFrame() {
		throw new Error('unimplemented abstract method');
	}
}
