const fs = require('fs');

const outputData = {};
const datasets = process.argv.slice(2).map(f => JSON.parse(fs.readFileSync(f, 'utf8')));

for (const data of datasets) {
	for (const [path, value] of Object.entries(data)) {
		const shortpath = shortenPath(path);
		outputData[shortpath] = (outputData[shortpath] || '') +
			value.events.map(event => `${event.time}\t${event.value}\n`).join('');
	}
}

for (const [f, data] of Object.entries(outputData)) {
	fs.writeFileSync(f + '.tsv', data, 'utf8');
}

function shortenPath(path) {
	return path.replace(/^Ableton\.LiveSet\.Tracks\./, '')
		.replace(/^(Midi|Audio)Track(?:\.(\d+))?/, '$1$2')
		.replace(/\.DeviceChain/, '')
		.replace(/\.DeviceChain\.Devices/, '')
		.replace(/(?:Instrument|AudioEffect)GroupDevice\.(Macro)Controls\.(\d)/, '$1$2')
		.replace(/\.MainSequencer\.Sample\.ArrangerAutomation\.Events/, '')
		.replace(/\.AudioClip\.(\d+)\.ModulationList\.Modulations/, '')
		.replace(/\.Mixer/, '')
		.replace(/\.(Arranger)?Automation\.Events\.FloatEvent$/, '');
}
