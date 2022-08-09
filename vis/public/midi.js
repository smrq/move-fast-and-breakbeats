import { WebMidi } from 'https://cdn.jsdelivr.net/npm/webmidi@3.0.21/dist/esm/webmidi.esm.min.js';

export async function initMidi(emitSongEvent) {
	let startTimestamp = 0;

	await WebMidi.enable();
	const midiInput = WebMidi.getInputByName('Sylphid');
	midiInput.addListener('midimessage', handleMidiMessage);

	return () => {
		midiInput.removeListener('midimessage', handleMidiMessage);
	};

	function handleMidiMessage({ message, timestamp }) {
		const t = timestamp - startTimestamp;

		switch (message.type) {
			case 'start':
				startTimestamp = timestamp;
				break;

			case 'noteon':
				switch (message.channel) {
					case 1:
						switch (message.dataBytes[0]) {
							case 36:
								emitSongEvent({ t, type: 'kick', velocity: message.dataBytes[1] });
								break;

							case 41:
								emitSongEvent({ t, type: 'hat', velocity: message.dataBytes[1] });
								break;

							case 43:
								emitSongEvent({ t, type: 'snare', velocity: message.dataBytes[1] });
								break;

							case 48:
							case 50:
							case 52:
							case 53:
								emitSongEvent({ t, type: 'stones', velocity: message.dataBytes[1] });
								break;
						}
						break;

					case 2:
						emitSongEvent({ t, type: 'mtn', note: message.dataBytes[0] });
						break;

					case 3: {
						const i = [0, 3, 5, 7, 10, 12].findIndex(x => x >= message.dataBytes[0] % 12);
						const o = ((message.dataBytes[0] - 36) / 12) | 0;
						const n = i + 5*o;
						emitSongEvent({ t, type: 'rnd', note: n });
						break;
					}

					case 5:
						emitSongEvent({ t, type: 'base', note: message.dataBytes[0] });
						break;
				}
				break;

			case 'controlchange':
				if (message.channel === 1 && message.dataBytes[0] === 100) {
					emitSongEvent({ t, type: 'cc_kick', value: message.dataBytes[1] > 0 });
				}

				if (message.channel === 3 && message.dataBytes[0] === 101) {
					emitSongEvent({ t, type: 'cc_rnddetune', value: message.dataBytes[1] });
				}

				if (message.channel === 3 && message.dataBytes[0] === 102) {
					emitSongEvent({ t, type: 'cc_rndfade', value: message.dataBytes[1] });
				}

				if (message.channel === 4 && message.dataBytes[0] === 104) {
					emitSongEvent({ t, type: 'cc_padfade', value: message.dataBytes[1] });
				}

				if (message.channel === 4 && message.dataBytes[0] === 105) {
					emitSongEvent({ t, type: 'cc_padmod', value: message.dataBytes[1] });
				}

				if (message.channel === 5 && message.dataBytes[0] === 106) {
					emitSongEvent({ t, type: 'cc_subfade', value: message.dataBytes[1] });
				}
				break;

			default:
				break;
		}
	}
}
