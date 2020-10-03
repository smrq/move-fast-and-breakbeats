function rot(n, x, y, rx, ry) {
	if (ry == 0) {
		if (rx == 1) {
			x = n - 1 - x;
			y = n - 1 - y;
		}
		return [y, x];
	} else {
		return [x, y];
	}
}

export function coordsToIndex(n, x, y) {
	let d = 0;
	for (let s = n >> 1; s > 0; s >>= 1) {
		const rx = (x & s) ? 1 : 0;
		const ry = (y & s) ? 1 : 0;
		d += s * s * ((3 * rx) ^ ry);
		[x, y] = rot(n, x, y, rx, ry);
	}
	return d;
}

export function indexToCoords(n, d) {
	let x = 0, y = 0, t = d;
	for (let s = 1; s < n; s <<= 1) {
		const rx = 1 & (t >> 1);
		const ry = 1 & (t ^ rx);
		[x, y] = rot(s, x, y, rx, ry);
		x += s * rx;
		y += s * ry;
		t = t >> 2;
	}
	return [x, y];
}
