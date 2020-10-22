#!/bin/sh
xq < nwwia.xml '[paths as $p | ($p|join(".")) as $q | select (($q|endswith(".ArrangerAutomation.Events.FloatEvent")) and (getpath($p)|type == "array")) | { path: $q, events: getpath($p)|map(.|{time: .["@Time"]|tonumber, value: .["@Value"]|tonumber}) }] | reduce .[] as $i ({}; .[$i.path] = ($i|del(.path)))' > nwwia1.json
xq < nwwia.xml '[paths as $p | ($p|join(".")) as $q | select (($q|endswith(".ModulationList.Modulations.Modulation.Automation.Events.FloatEvent")) and ($q|contains(".MainSequencer.Sample.ArrangerAutomation.")) and (getpath($p)|type == "array")) | { path: $q, events: getpath($p)|map(.|{time: .["@Time"]|tonumber, value: .["@Value"]|tonumber}), start: getpath($p[:-6]).CurrentStart["@Value"]|tonumber, end: getpath($p[:-6]).CurrentEnd["@Value"]|tonumber, loopStart: getpath($p[:-3]).LoopSlot.Value.Loop.LoopStart["@Value"]|tonumber, loopEnd: getpath($p[:-3]).LoopSlot.Value.Loop.LoopEnd["@Value"]|tonumber }] | reduce .[] as $i ({}; .[$i.path] = ($i|del(.path)))' > nwwia2.json
node expandautomation.js nwwia2.json > nwwia2b.json
node tsvconvert.js nwwia1.json nwwia2b.json
rm *.json
