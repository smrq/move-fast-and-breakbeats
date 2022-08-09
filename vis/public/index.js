import { initScene, drawFrame, handleSongEvent } from './vis.js';
import { initMidi } from './midi.js';

const canvas = document.getElementById('canvas');
const canvasCtx = canvas.getContext('webgl');

window.WIDTH = canvas.width;
window.HEIGHT = canvas.height;
window.FPS = 60;

init();

async function init() {
	initScene(canvasCtx);
	renderOnline();
}

async function renderOnline() {
	let frame = 0;
	requestAnimationFrame(handleRaf);

	function handleRaf() {
		drawFrame(frame++);
		requestAnimationFrame(handleRaf);
	}
}

window.recordedEvents = [];

fetch('events.json').then(res => res.json()).then(data => {
	window.recordedEvents = data;
	document.getElementById('record').innerHTML = 'Record (' + recordedEvents.length + ' events recorded)';
});

function recordSongEvent(event) {
	recordedEvents.push(event);
	document.getElementById('record').innerHTML = 'Stop (' + recordedEvents.length + ' events recorded)';
}

async function renderOffline() {
	let frame = 0;
	const outframes = 0;
	const endFrame = Math.floor(recordedEvents[recordedEvents.length - 1].t / 1000 * FPS) + outframes;

	while (frame < endFrame) {
		const t0 = Date.now();

		while (recordedEvents.length && recordedEvents[0].t <= frame * 1000 / FPS) {
			handleSongEvent(recordedEvents.shift());
		}

		drawFrame(frame);
		cvg.addFrame(canvas);
		console.log(`frame ${frame} / ${endFrame-1} done in ${Date.now() - t0}ms`);
		await new Promise((resolve) => setTimeout(resolve));
		frame++;
	}

	cvg.render("out");
}

window.play = async () => {
	await initMidi(handleSongEvent);
	renderOnline();
};

function downloadJson(obj, filename) {
	const blob = new Blob([JSON.stringify(obj)], { type: "text/json" });
	const link = document.createElement("a");
	link.download = filename;
	link.href = window.URL.createObjectURL(blob);
	link.dataset.downloadurl = ["text/json", link.download, link.href].join(":");
	const evt = new MouseEvent('click', {
		view: window,
		bubbles: true,
		cancelable: true,
	});
	link.dispatchEvent(evt);
	link.remove();
}

let closeMidi = null;
window.record = async () => {
	if (!closeMidi) {
		recordedEvents = [];
		closeMidi = await initMidi(recordSongEvent);
		document.getElementById('record').innerHTML = 'Stop (0 events recorded)';
	} else {
		closeMidi();
		closeMidi = null;
		document.getElementById('record').innerHTML = 'Record (' + recordedEvents.length + ' events recorded)';
		downloadJson(recordedEvents, 'events.json');
	}
};

window.render = () => {
	renderOffline();
};
