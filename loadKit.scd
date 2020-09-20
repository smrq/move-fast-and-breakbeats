{|kitname|
	var kit = Dictionary.new;
	var folder = PathName.new(thisProcess.nowExecutingPath.dirname +/+ "media" +/+ kitname);
	folder.entries.do({|path, i|
		var buf = Buffer.read(s, path.fullPath);
		kit = kit.put(path.fileNameWithoutExtension, buf);
	});
	kit
}
