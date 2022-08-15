import { WebMidi } from 'https://cdn.jsdelivr.net/npm/webmidi@3.0.21/dist/esm/webmidi.esm.min.js';

export async function initMidi(emitMidiEvent) {
	let startTimestamp = 0;

	await WebMidi.enable();

	console.log('MIDI enabled');

	const midiInput = WebMidi.getInputByName('Sylphid');
	midiInput.addListener('midimessage', handleMidiMessage);

	return () => {
		midiInput.removeListener('midimessage', handleMidiMessage);
	};

	function handleMidiMessage({ message, timestamp }) {
		switch (message.type) {
			case 'start':
				startTimestamp = timestamp;
				break;

			case 'noteon':
			case 'noteoff':
			case 'controlchange': {
				const event = {
					timestamp: (timestamp - startTimestamp) | 0,
					type: message.type,
					channel: message.channel,
					dataBytes: message.dataBytes,
				};
				emitMidiEvent(event);
				break;
			}

		}
	}
}
