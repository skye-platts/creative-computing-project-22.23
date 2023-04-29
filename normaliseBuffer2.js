var srcBuffer = new Buffer("srcLiveRaw");
var srcNormal = new Buffer("srcLiveNorm");
outlets = 1;

function bang() {
	
	var bufferSize = srcBuffer.framecount();
	
	var maxSampleValue = 0;
	
	var normalFactor = 0;
	
	var buffer = [];
	
	for (var i = 0; i < bufferSize; i++) {
		var currentValue = srcBuffer.peek(1, i, 1);
		var currentValueABS = Math.abs(currentValue);
		buffer[i] = currentValue
		if (currentValueABS > maxSampleValue) {
			maxSampleValue = currentValue;
		}
	}
	
	normalFactor = 0.9 / maxSampleValue;
	
	for (var i = 0; i < bufferSize; i++) {
		buffer[i] *= normalFactor;
	}
	
	srcNormal.poke(1, 0, buffer);
	
	outlet(0, bang);
	
}