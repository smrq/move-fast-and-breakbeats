const fs = require('fs');

const data = JSON.parse(fs.readFileSync(process.argv[2], 'utf8'));
for (const key of Object.keys(data)) {
	const clip = data[key];
	data[key] = { events: expandClip(data[key]) };
}

console.log(JSON.stringify(data, null, '  '));

// const x = [].concat(...data.map(expandClip));
// require('fs').writeFileSync('nwwia2b.json', JSON.stringify(x, null, '  '));

function lerp(value, in1, in2, out1, out2) {
	return out1 + (out2 - out1) * ((value - in1) / (in2 - in1));
}

function cropEvents(events, t0, t1) {
	let i, j;
	for (i = 0; i < events.length && events[i].time <= t0; ++i) {}
	for (j = 0; j < events.length && events[j].time < t1; ++j) {}
	--i; // last index that is <= t0
	++j; // first index that is >= t1, +1

	const croppedEvents = events.slice(i, j);

	if (croppedEvents[0].time < t0) {
		croppedEvents[0] = {
			time: t0,
			value: lerp(
				t0,
				croppedEvents[0].time,
				croppedEvents[1].time,
				croppedEvents[0].value,
				croppedEvents[1].value)
		};
	}
	if (croppedEvents[croppedEvents.length-1].time > t1) {
		croppedEvents[croppedEvents.length-1] = {
			time: t1,
			value: lerp(
				t1,
				croppedEvents[croppedEvents.length-2].time,
				croppedEvents[croppedEvents.length-1].time,
				croppedEvents[croppedEvents.length-2].value,
				croppedEvents[croppedEvents.length-1].value)
		};
	}

	return croppedEvents;	
}

function expandClip({ events, start, end, loopStart, loopEnd }) {
	if (events[0].time > loopStart) {
		events.unshift({ time: loopStart, value: events[0].value });
	}
	if (events[events.length-1].time < loopEnd) {
		events.push({ time: loopEnd, value: events[events.length-1].value });
	}

	const loopLength = loopEnd - loopStart;
	const croppedEvents = cropEvents(events, loopStart, loopEnd);

	const clipLength = end - start;
	const loopCount = Math.ceil(clipLength / loopLength);
	const expandedEvents = [].concat(...Array(loopCount).fill().map((_, i) => (
		croppedEvents.map(e => ({
			time: e.time - loopStart + i * loopLength,
			value: e.value
		}))
	)));

	return cropEvents(expandedEvents, 0, clipLength)
		.map(e => ({ time: e.time + start, value: e.value }));
}
