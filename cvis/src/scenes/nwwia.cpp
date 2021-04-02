#include "Nwwia.h"

NwwiaScene::NwwiaScene(std::shared_ptr<GlContext> glContext, std::vector<std::string> &automationFilenames)
: glContext(glContext) {
	axisShader = std::make_unique<gl::Shader>("shaders/axisShader.vert", "shaders/axisShader.geom", "shaders/axisShader.frag");
	lineShader = std::make_unique<gl::Shader>("shaders/lineShader.vert", "shaders/lineShader.geom", "shaders/lineShader.frag");
	gridShader = std::make_unique<gl::Shader>("shaders/gridShader.vert", "shaders/gridShader.frag");
	
	tiltShiftPass = std::make_unique<TiltShiftPass>(glContext);
	bloomPass = std::make_unique<BloomPass>(glContext);
	filmPass = std::make_unique<FilmPass>(glContext);

	projectionMatrix = glm::perspective(glm::radians(50.0f), (float)glContext->width / (float)glContext->height, 0.1f, 10000.0f);
	viewMatrix = glm::lookAt(
		glm::vec3(0.0f, 400.0f, 600.0f),
		glm::vec3(0.0f, -150.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	readAutomationFiles(automationFilenames);

	initAxisVertices();
	initOscVertices();
	initGridVertices();
}

void NwwiaScene::readAutomationFiles(std::vector<std::string> &automationFilenames) {
	for (auto &filename: automationFilenames) {
		std::multimap<double, double> values;
		std::ifstream inputStream(filename);
		if (inputStream.fail()) {
			fprintf(stderr, "Failed to open automation track file %s", filename.c_str());
			exit(1);
		}
		std::string line;
		while (getline(inputStream, line)) {
			std::stringstream stringStream(line);
			double key, value;
			stringStream >> key;
			stringStream >> value;
			values.insert(std::multimap<double, double>::value_type(key, value));
		}
		automation.push_back(values);
	}
}

double NwwiaScene::getAutomationLevel(int trackIndex, double time) {
	auto &track = automation[trackIndex];

	const double bpm = 120;
	double beat = time * (bpm / 60.0);

	auto itHigh = track.upper_bound(beat);
	if (itHigh == track.begin()) {
		return itHigh->second;
	}
	auto itLow = std::prev(itHigh);
	if (itHigh == track.end()) {
		return itLow->second;
	}
	return lerp(beat, itLow->first, itHigh->first, itLow->second, itHigh->second);
}

void NwwiaScene::initAxisVertices() {
	float axisData[] = {
		  0.0f,   0.0f,   0.0f, 1.0f, 1.0f, 0.0f,
		100.0f,   0.0f,   0.0f, 1.0f, 1.0f, 0.0f,
		  0.0f,   0.0f,   0.0f, 0.0f, 1.0f, 1.0f,
		  0.0f, 100.0f,   0.0f, 0.0f, 1.0f, 1.0f,
		  0.0f,   0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
		  0.0f,   0.0f, 100.0f, 1.0f, 0.0f, 1.0f,
	};

	glBindVertexArray(axisArray);
	glBindBuffer(GL_ARRAY_BUFFER, axisBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(axisData), axisData, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);		
}

void NwwiaScene::initOscVertices() {
	for (int i = 0; i < NWWIA_OSC_COUNT; ++i) {
		oscDataX[i+1] = (float)lerpClamped(i, 0, NWWIA_OSC_COUNT, -500, 500);
		oscDataY[i+1] = 0.0f;
	}
	oscDataX[0] = oscDataX[1];
	oscDataY[0] = oscDataY[1];
	oscDataX[NWWIA_OSC_COUNT+1] = oscDataX[NWWIA_OSC_COUNT];
	oscDataY[NWWIA_OSC_COUNT+1] = oscDataY[NWWIA_OSC_COUNT];

	glBindVertexArray(oscArray);
	glBindBuffer(GL_ARRAY_BUFFER, oscBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(oscDataX) + sizeof(oscDataY), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(oscDataX), oscDataX);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(oscDataX), sizeof(oscDataY), oscDataY);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)(sizeof(oscDataX)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);	
}

void NwwiaScene::initGridVertices() {
	for (int i = 0; i < NWWIA_GRID_COUNT; ++i) {
		int x, z;
		hilbert(NWWIA_GRID_COUNT, i, &x, &z);
		gridDataXZ[i][0] = (float)lerpClamped(x, 0, NWWIA_GRID_SIZE, -250, 250);
		gridDataXZ[i][1] = (float)lerpClamped(z, 0, NWWIA_GRID_SIZE, -250, 250);
		gridDataYScale[i][0] = 0.0f;
		gridDataYScale[i][1] = 1.0f;
	}

	glBindVertexArray(gridArray);
	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(gridDataXZ) + sizeof(gridDataYScale), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(gridDataXZ), gridDataXZ);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(gridDataXZ), sizeof(gridDataYScale), gridDataYScale);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(sizeof(gridDataXZ)));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void NwwiaScene::populateOscData(double *timeData, size_t timeDataSize) {
	int downsample = timeDataSize / NWWIA_OSC_COUNT;
	for (int i = 0; i < NWWIA_OSC_COUNT; ++i) {
		oscDataY[i+1] = (float)timeData[i * downsample] * 100.0f;
	}
	oscDataY[0] = oscDataY[1];
	oscDataY[NWWIA_OSC_COUNT+1] = oscDataY[NWWIA_OSC_COUNT];

	glBindBuffer(GL_ARRAY_BUFFER, oscBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(oscDataX), sizeof(oscDataY), oscDataY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void NwwiaScene::populateGridData(double *freqData, size_t freqDataSize, int sampleRate) {	
	double minPitch = log2(NWWIA_MIN_FREQ);
	double maxPitch = log2(NWWIA_MAX_FREQ);
	double bandSizePitch = (maxPitch - minPitch) / NWWIA_GRID_COUNT;

	for (int i = 0; i < NWWIA_GRID_COUNT; ++i) {
		double bandLowPitch = minPitch + i * bandSizePitch;
		double bandHighPitch = minPitch + (i+1) * bandSizePitch;

		double bandLowFreq = pow(2, bandLowPitch);
		double bandHighFreq = pow(2, bandHighPitch);

		double db = getBandDb(freqData, freqDataSize, sampleRate, bandLowFreq, bandHighFreq);
		gridDataYScale[i][0] = (float)lerpClamped(db, -100, -40, 0, 200);
		gridDataYScale[i][1] = (float)lerpClamped(db, -100, -20, 1, 5);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(gridDataXZ), sizeof(gridDataYScale), gridDataYScale);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void NwwiaScene::drawAxes() {
	axisShader->use();
	axisShader->setMat4("projection", projectionMatrix);
	axisShader->setMat4("view", viewMatrix);
	axisShader->setVec3("normal", 0.0f, 0.0f, -1.0f);
	axisShader->setFloat("lineWidth", 4.0f);
	glBindVertexArray(axisArray);
	glDrawArrays(GL_LINES, 0, 12);
	glBindVertexArray(0);
}

void NwwiaScene::drawOsc() {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 300.0f));
	lineShader->use();
	lineShader->setMat4("projection", projectionMatrix);
	lineShader->setMat4("view", viewMatrix);
	lineShader->setMat4("model", modelMatrix);
	lineShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	lineShader->setFloat("lineWidth", 2.0f);
	glBindVertexArray(oscArray);
	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, NWWIA_OSC_COUNT + 2);
	glBindVertexArray(0);
}

void NwwiaScene::drawGrid(double time) {
	glEnable(GL_PROGRAM_POINT_SIZE);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::rotate(modelMatrix, (float)(0.1 * time), glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
	gridShader->use();
	gridShader->setMat4("projection", projectionMatrix);
	gridShader->setMat4("view", viewMatrix);
	gridShader->setMat4("model", modelMatrix);
	gridShader->setVec2("resolution", (float)glContext->width, (float)glContext->height);
	gridShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	glBindVertexArray(gridArray);
	glDrawArrays(GL_POINTS, 0, NWWIA_GRID_COUNT);
	glBindVertexArray(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void NwwiaScene::draw(double time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize, int sampleRate) {	
	populateOscData(timeData, timeDataSize);
	populateGridData(freqData, freqDataSize, sampleRate);

	glContext->framebufferFront->activate();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawAxes();
	drawOsc();
	drawGrid(time);

	glDisable(GL_DEPTH_TEST);

	tiltShiftPass->draw(0.35, 0.002, 0.8);
	bloomPass->draw(1.5, 0.4, 0.85);

	double dbLow = getBandDb(freqData, freqDataSize, sampleRate, 20, 40);
	double scanlineLevel = lerpClamped(dbLow, -60, -30, 0, 0.25);
	double dbHigh = getBandDb(freqData, freqDataSize, sampleRate, 2000, 16000);
	double noiseLevel = lerpClamped(dbHigh, -100, -30, 0, 0.75);
	filmPass->draw(time, noiseLevel, scanlineLevel, 648.0);
}
