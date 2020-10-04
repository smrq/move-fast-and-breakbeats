var fs = require('fs');
var path = "/Users/gregsm/Library/Application Support/Google/Chrome/Default/File System/022/p/";
var first = 0;
var last = 6458;
var j = 0;

for (var i = first; i <= last; ++i) {
  var subdir = ("00" + Math.floor(i / 100)).slice(-2);
  var file = path + subdir + '/' + ("000000000" + i).slice(-8);
  var target = './file' + ("00000000" + (j++)).slice(-8);
  console.log(file, target + '.png');
  fs.renameSync(file, target + '.png');
}