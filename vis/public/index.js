// import { VisSummit as Vis } from './vis/VisSummit.js'
import { VisNwwia as Vis } from './vis/VisNwwia.js'
import { initMidi } from './midi/webmidi.js';
import { MidiState } from './midi/MidiState.js';
import { downloadJson } from './util.js';

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

window.WIDTH = canvas.width;
window.HEIGHT = canvas.height;
window.FPS = 60;

window.play = async () => {
	document.getElementById('controls-init').style.display = 'none';

	const midiState = new MidiState();
	const vis = new Vis(canvasCtx, midiState);
	vis.init();

	await initMidi(midiState.handleMidiEvent.bind(midiState));

	let frame = 0;
	requestAnimationFrame(handleRaf);

	function handleRaf() {
		vis.drawFrame(frame++);
		requestAnimationFrame(handleRaf);
	}
};

window.record = async () => {
	document.getElementById('controls-init').style.display = 'none';
	document.getElementById('controls-recording').style.display = '';

	const events = [];
	await initMidi(event => {
		events.push(event);
		document.getElementById('recording-count').innerHTML = events.length;
	});
	window.save_recording = () => {
		downloadJson(events, 'events.json')
	};
}

window.render = async () => {
	document.getElementById('controls-init').style.display = 'none';
	document.getElementById('controls-rendering').style.display = '';

	const events = await fetch('events.json').then(res => res.json());

	const midiState = new MidiState();
	const vis = new Vis(canvasCtx, midiState);
	vis.init();

	let frame = 0;
	const outFrames = 0;
	const endFrame = Math.floor(correctedTimestamp(events[events.length - 1].timestamp) / 1000 * FPS) + outFrames;

	while (frame < endFrame) {
		const t0 = Date.now();

		while (events.length && correctedTimestamp(events[0].timestamp) <= frame * 1000 / FPS) {
			midiState.handleMidiEvent(events.shift());
		}

		vis.drawFrame(frame);
		cvg.addFrame(canvas);

		console.log(`frame ${frame} / ${endFrame-1} done in ${Date.now() - t0}ms`);
		document.getElementById('rendering-count').innerHTML = `${frame}/${endFrame-1}`;

		await new Promise((resolve) => setTimeout(resolve));

		frame++;
	}

	cvg.render("out");

	function correctedTimestamp(t) {
		return t + 2000;
	}
}
