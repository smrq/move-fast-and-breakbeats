(
~midipelog = {|note|
	// A4=459
	// Octave 5: 72, 74, 76, 77, 79, 81
	var tuning = [585, 626, 700, 782, 856, 918, 1034];
	var octave = div(note, 12) - 1;
	var tone = note % 12;
	var hz = tuning[[0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6][tone]];
	hz * (2 ** (octave - 5));
};
)
