#include "tetrik.h"

TetrikScene::TetrikScene(std::shared_ptr<GlContext> glContext)
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

	initAxisVertices();
	initOscVertices();
	initGridVertices();
}

void TetrikScene::initAxisVertices() {
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

void TetrikScene::initOscVertices() {
	for (int i = 0; i < TETRIK_OSC_COUNT; ++i) {
		oscDataX[i+1] = lerpClamped(i, 0, TETRIK_OSC_COUNT, -500.0f, 500.0f);
		oscDataY[i+1] = 0.0f;
	}
	oscDataX[0] = oscDataX[1];
	oscDataY[0] = oscDataY[1];
	oscDataX[TETRIK_OSC_COUNT+1] = oscDataX[TETRIK_OSC_COUNT];
	oscDataY[TETRIK_OSC_COUNT+1] = oscDataY[TETRIK_OSC_COUNT];

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

void TetrikScene::initGridVertices() {
	for (int i = 0; i < TETRIK_GRID_COUNT; ++i) {
		int x, z;
		hilbert(TETRIK_GRID_COUNT, i, &x, &z);
		gridDataXZ[i][0] = lerpClamped(x, 0, TETRIK_GRID_SIZE, -250.0f, 250.0f);
		gridDataXZ[i][1] = lerpClamped(z, 0, TETRIK_GRID_SIZE, -250.0f, 250.0f);
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

void TetrikScene::populateOscData(double *timeData, size_t timeDataSize) {
	int downsample = timeDataSize / TETRIK_OSC_COUNT;
	for (int i = 0; i < TETRIK_OSC_COUNT; ++i) {
		oscDataY[i+1] = (float)timeData[i * downsample] * 100.0f;
	}
	oscDataY[0] = oscDataY[1];
	oscDataY[TETRIK_OSC_COUNT+1] = oscDataY[TETRIK_OSC_COUNT];

	glBindBuffer(GL_ARRAY_BUFFER, oscBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(oscDataX), sizeof(oscDataY), oscDataY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TetrikScene::populateGridData(double *freqData, size_t freqDataSize, int sampleRate) {	
	double minPitch = log2(TETRIK_MIN_FREQ);
	double maxPitch = log2(TETRIK_MAX_FREQ);
	double bandSizePitch = (maxPitch - minPitch) / TETRIK_GRID_COUNT;

	for (int i = 0; i < TETRIK_GRID_COUNT; ++i) {
		double bandLowPitch = minPitch + i * bandSizePitch;
		double bandHighPitch = minPitch + (i+1) * bandSizePitch;

		double bandLowFreq = pow(2, bandLowPitch);
		double bandHighFreq = pow(2, bandHighPitch);

		double db = getBandDb(freqData, freqDataSize, sampleRate, bandLowFreq, bandHighFreq);
		gridDataYScale[i][0] = lerpClamped((float)db, -100.0f, -40.0f, 0.0f, 200.0f);
		gridDataYScale[i][1] = lerpClamped((float)db, -100.0f, -20.0f, 1.0f, 5.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(gridDataXZ), sizeof(gridDataYScale), gridDataYScale);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void TetrikScene::drawAxes() {
	axisShader->use();
	axisShader->setMat4("projection", projectionMatrix);
	axisShader->setMat4("view", viewMatrix);
	axisShader->setVec3("normal", 0.0f, 0.0f, -1.0f);
	axisShader->setFloat("lineWidth", 4.0f);
	glBindVertexArray(axisArray);
	glDrawArrays(GL_LINES, 0, 12);
	glBindVertexArray(0);
}

void TetrikScene::drawOsc() {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 300.0f));
	lineShader->use();
	lineShader->setMat4("projection", projectionMatrix);
	lineShader->setMat4("view", viewMatrix);
	lineShader->setMat4("model", modelMatrix);
	lineShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	lineShader->setFloat("lineWidth", 2.0f);
	glBindVertexArray(oscArray);
	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, TETRIK_OSC_COUNT + 2);
	glBindVertexArray(0);
}

void TetrikScene::drawGrid(float time) {
	glEnable(GL_PROGRAM_POINT_SIZE);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::rotate(modelMatrix, 0.1f * time, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
	gridShader->use();
	gridShader->setMat4("projection", projectionMatrix);
	gridShader->setMat4("view", viewMatrix);
	gridShader->setMat4("model", modelMatrix);
	gridShader->setVec2("resolution", (float)glContext->width, (float)glContext->height);
	gridShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	glBindVertexArray(gridArray);
	glDrawArrays(GL_POINTS, 0, TETRIK_GRID_COUNT);
	glBindVertexArray(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void TetrikScene::draw(float time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize, int sampleRate) {	
	populateOscData(timeData, timeDataSize);
	populateGridData(freqData, freqDataSize, sampleRate);

	glContext->framebufferFront->activate();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// drawAxes();
	drawOsc();
	drawGrid(time);

	glDisable(GL_DEPTH_TEST);

	tiltShiftPass->draw(0.35f, 0.002f, 0.8f);
	bloomPass->draw(1.5f, 0.4f, 0.85f);

	float dbLow = getBandDb(freqData, freqDataSize, sampleRate, 20, 40);
	float scanlineLevel = lerpClamped(dbLow, -60, -30, 0, 0.25);
	float dbHigh = getBandDb(freqData, freqDataSize, sampleRate, 2000, 16000);
	float noiseLevel = lerpClamped(dbHigh, -100, -30, 0, 0.75);
	filmPass->draw(time, noiseLevel, scanlineLevel, 648.0);
}
