export class MidiState {
	constructor() {
		this.ch = [...Array(16)].map(() => ({
			notes: new Map(),
			cc: new Map(),
			listeners: { noteon: [] }
		}));
		this.ch.unshift(null);
	}

	onNoteOn(channel, fn) {
		this.ch[channel].listeners.noteon.push(fn);
	}

	handleMidiEvent({ timestamp, type, channel, dataBytes }) {
		switch (type) {
			case 'noteon':
				this.ch[channel].notes.set(dataBytes[0], dataBytes[1]);
				for (let listener of this.ch[channel].listeners.noteon) {
					listener({ note: dataBytes[0], velocity: dataBytes[1] });
				}
				break;

			case 'noteoff':
				this.ch[channel].notes.delete(dataBytes[0]);
				break;

			case 'controlchange':
				// TODO: all notes off CC (#123)
				this.ch[channel].cc.set(dataBytes[0], dataBytes[1]);
				break;

			default:
				break;
		}
	}
}
