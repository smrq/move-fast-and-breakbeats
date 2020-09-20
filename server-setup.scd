s.options.device = "Fireface 802 (23913873)";
s.options.numOutputBusChannels = 12;
s.options.numInputBusChannels = 12;
s.options.memSize = 2.pow(20);
s.reboot;
MIDIClient.init;
MIDIIn.connectAll;
VSTPlugin.search;
~midiSrc = Dictionary.newFrom([\apc, 1082953365, \fireface, -656226983, \iac, 571065816]);
