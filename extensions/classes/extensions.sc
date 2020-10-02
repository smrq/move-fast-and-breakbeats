+ Integer {
	splay {|min=(-1.0), max=1.0|
		^if (this<2,
			{[(min+max)/2]},
			{(max-min) * (0..(this-1)) / (this-1) + min})
	}
}
