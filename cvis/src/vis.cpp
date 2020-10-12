#include "vis.h"

static void hilbert(int n, int d, int *x, int *y) {
	*x = 0;
	*y = 0;
	int t = d;

	for (int s = 1; s < n; s <<= 1) {
		int rx = 1 & (t >> 1);
		int ry = 1 & (t ^ rx);

		if (ry == 0) {
			if (rx == 1) {
				*x = s - 1 - *x;
				*y = s - 1 - *y;
			}
			int tmp = *x;
			*x = *y;
			*y = tmp;
		}

		*x += s * rx;
		*y += s * ry;
		t = t >> 2;
	}
}

static float lerp(float value, float in1, float in2, float out1, float out2) {
	return ((value - in1) / (in2 - in1)) * (out2 - out1) + out1;
}

static float lerpClamped(float value, float in1, float in2, float out1, float out2) {
	return lerp((value < in1) ? in1 : (value > in2) ? in2 : value, in1, in2, out1, out2);
}

Vis::Vis(std::shared_ptr<gl::Window> window, const char *endFrameFilename)
: window(std::move(window)), endFrame(endFrameFilename) {
	framebufferFront = std::make_unique<gl::Framebuffer>(GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT, true, true);
	framebufferBack = std::make_unique<gl::Framebuffer>(GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT, true, true);

	axisShader = std::make_unique<gl::Shader>(
#include "shaders/axisShader.glsl"
	);
	lineShader = std::make_unique<gl::Shader>(
#include "shaders/lineShader.glsl"
	);
	gridShader = std::make_unique<gl::Shader>(
#include "shaders/gridShader.glsl"
	);
	tiltShiftShader = std::make_unique<gl::Shader>(
#include "shaders/tiltShiftShader.glsl"
	);
	filmShader = std::make_unique<gl::Shader>(
#include "shaders/filmShader.glsl"
	);
	copyShader = std::make_unique<gl::Shader>(
#include "shaders/copyShader.glsl"
	);

	bloomPass = std::make_unique<BloomPass>(GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT);

	projectionMatrix = glm::perspective(glm::radians(50.0f), (float)GEN_VIDEO_WIDTH / (float)GEN_VIDEO_HEIGHT, 0.1f, 10000.0f);
	viewMatrix = glm::lookAt(
		glm::vec3(0.0f, 400.0f, 600.0f),
		glm::vec3(0.0f, -150.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	initAxisVertices();
	initOscVertices();
	initGridVertices();
	initScreenQuadVertices();
}

void Vis::initAxisVertices() {
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

void Vis::initOscVertices() {
	for (int i = 0; i < VIS_OSC_COUNT; ++i) {
		oscDataX[i+1] = lerpClamped(i, 0, VIS_OSC_COUNT, -500.0f, 500.0f);
		oscDataY[i+1] = 0.0f;
	}
	oscDataX[0] = oscDataX[1];
	oscDataY[0] = oscDataY[1];
	oscDataX[VIS_OSC_COUNT+1] = oscDataX[VIS_OSC_COUNT];
	oscDataY[VIS_OSC_COUNT+1] = oscDataY[VIS_OSC_COUNT];

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

void Vis::initGridVertices() {
	for (int i = 0; i < VIS_GRID_COUNT; ++i) {
		int x, z;
		hilbert(VIS_GRID_COUNT, i, &x, &z);
		gridDataXZ[i][0] = lerpClamped(x, 0, VIS_GRID_SIZE, -250.0f, 250.0f);
		gridDataXZ[i][1] = lerpClamped(z, 0, VIS_GRID_SIZE, -250.0f, 250.0f);
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

void Vis::initScreenQuadVertices() {
	float screenQuadVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	glBindVertexArray(screenQuadArray);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

double getBandDb(double *freqData, size_t freqDataSize, double freqLow, double freqHigh) {
	double hzPerBin = ((double)OUT_AUDIO_SAMPLE_RATE / 2.0) / freqDataSize;

	double realIndexLow = freqLow / hzPerBin;
	int indexLow = (int)realIndexLow;
	double tLow = realIndexLow - indexLow;
	
	double realIndexHigh = freqHigh / hzPerBin;
	int indexHigh = (int)realIndexHigh;
	double tHigh = realIndexHigh - indexHigh;

	double power = 0;
	for (int i = indexLow; i <= indexHigh; ++i) {
		power += freqData[i];
		if (i == indexLow) {
			power -= freqData[i] * tLow;
		}
		if (i == indexHigh) {
			power -= freqData[i] * (1.0 - tHigh);
		}
	}
	return 20.0 * log10(power);
}

void Vis::populateOscData(double *timeData, size_t timeDataSize) {
	int downsample = timeDataSize / VIS_OSC_COUNT;
	for (int i = 0; i < VIS_OSC_COUNT; ++i) {
		oscDataY[i+1] = (float)timeData[i * downsample] * 100.0f;
	}
	oscDataY[0] = oscDataY[1];
	oscDataY[VIS_OSC_COUNT+1] = oscDataY[VIS_OSC_COUNT];

	glBindBuffer(GL_ARRAY_BUFFER, oscBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(oscDataX), sizeof(oscDataY), oscDataY);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Vis::populateGridData(double *freqData, size_t freqDataSize) {	
	double minPitch = log2(VIS_MIN_FREQ);
	double maxPitch = log2(VIS_MAX_FREQ);
	double bandSizePitch = (maxPitch - minPitch) / VIS_GRID_COUNT;

	for (int i = 0; i < VIS_GRID_COUNT; ++i) {
		double bandLowPitch = minPitch + i * bandSizePitch;
		double bandHighPitch = minPitch + (i+1) * bandSizePitch;

		double bandLowFreq = pow(2, bandLowPitch);
		double bandHighFreq = pow(2, bandHighPitch);

		double db = getBandDb(freqData, freqDataSize, bandLowFreq, bandHighFreq);
		gridDataYScale[i][0] = lerpClamped((float)db, -100.0f, -40.0f, 0.0f, 200.0f);
		gridDataYScale[i][1] = lerpClamped((float)db, -100.0f, -20.0f, 1.0f, 5.0f);
	}

	glBindBuffer(GL_ARRAY_BUFFER, gridBuffer);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(gridDataXZ), sizeof(gridDataYScale), gridDataYScale);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Vis::drawAxes() {
	axisShader->use();
	axisShader->setMat4("projection", projectionMatrix);
	axisShader->setMat4("view", viewMatrix);
	axisShader->setVec3("normal", 0.0f, 0.0f, -1.0f);
	axisShader->setFloat("lineWidth", 4.0f);
	glBindVertexArray(axisArray);
	glDrawArrays(GL_LINES, 0, 12);
	glBindVertexArray(0);
}

void Vis::drawOsc() {
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 300.0f));
	lineShader->use();
	lineShader->setMat4("projection", projectionMatrix);
	lineShader->setMat4("view", viewMatrix);
	lineShader->setMat4("model", modelMatrix);
	lineShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	lineShader->setFloat("lineWidth", 2.0f);
	glBindVertexArray(oscArray);
	glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, VIS_OSC_COUNT + 2);
	glBindVertexArray(0);
}

void Vis::drawGrid(float time) {
	glEnable(GL_PROGRAM_POINT_SIZE);
	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::rotate(modelMatrix, 0.1f * time, glm::vec3(0.0f, 1.0f, 0.0f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -50.0f, 0.0f));
	gridShader->use();
	gridShader->setMat4("projection", projectionMatrix);
	gridShader->setMat4("view", viewMatrix);
	gridShader->setMat4("model", modelMatrix);
	gridShader->setVec2("resolution", (float)GEN_VIDEO_WIDTH, (float)GEN_VIDEO_HEIGHT);
	gridShader->setVec3("color", 1.0f, 1.0f, 1.0f);
	glBindVertexArray(gridArray);
	glDrawArrays(GL_POINTS, 0, VIS_GRID_COUNT);
	glBindVertexArray(0);
	glDisable(GL_PROGRAM_POINT_SIZE);
}

void Vis::drawScreenQuad(unsigned int texture) {
	glBindVertexArray(screenQuadArray);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void Vis::swapFramebuffers() {
	framebufferFront.swap(framebufferBack);
}

void Vis::drawTiltShiftPass() {
	framebufferBack->activate();
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	tiltShiftShader->use();
	tiltShiftShader->setFloat("focusPosition", 0.35f);
	tiltShiftShader->setFloat("amount", 0.002f);
	tiltShiftShader->setFloat("brightness", 0.8f);
	drawScreenQuad(framebufferFront->texture);
	swapFramebuffers();
}

void Vis::drawBloomPass() {
	bloomPass->brightFramebuffer->activate();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	bloomPass->highPassShader->use();
	bloomPass->highPassShader->setVec4("defaultColor", 0.0f, 0.0f, 0.0f, 0.0f);
	bloomPass->highPassShader->setFloat("luminosityThreshold", 0.85f);
	bloomPass->highPassShader->setFloat("smoothWidth", 0.01f);
	drawScreenQuad(framebufferFront->texture);

	unsigned int inputTexture = bloomPass->brightFramebuffer->texture;
	for (int i = 0; i < 5; ++i) {
		bloomPass->hFramebuffers[i]->activate();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bloomPass->blurShaders[i]->use();
		bloomPass->blurShaders[i]->setVec2("texSize",
			(float)bloomPass->hFramebuffers[i]->width,
			(float)bloomPass->hFramebuffers[i]->height);
		bloomPass->blurShaders[i]->setVec2("direction", 1.0f, 0.0f);
		drawScreenQuad(inputTexture);

		bloomPass->vFramebuffers[i]->activate();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		bloomPass->blurShaders[i]->setVec2("direction", 0.0f, 1.0f);
		drawScreenQuad(bloomPass->hFramebuffers[i]->texture);

		inputTexture = bloomPass->vFramebuffers[i]->texture;
	}

	bloomPass->hFramebuffers[0]->activate();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	bloomPass->compositeShader->use();
	bloomPass->compositeShader->setInt("blurTexture0", 0);
	bloomPass->compositeShader->setInt("blurTexture1", 1);
	bloomPass->compositeShader->setInt("blurTexture2", 2);
	bloomPass->compositeShader->setInt("blurTexture3", 3);
	bloomPass->compositeShader->setInt("blurTexture4", 4);
	bloomPass->compositeShader->setFloat("strength", 1.5f);
	bloomPass->compositeShader->setFloat("radius", 0.4f);
	float factors[5] = { 1.0f, 0.8f, 0.6f, 0.4f, 0.2f };
	bloomPass->compositeShader->setFloatArray("factors", 5, factors);
	glm::vec3 tintColors[5] = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f)
	};
	bloomPass->compositeShader->setVec3Array("tintColors", 5, tintColors);
	glBindVertexArray(screenQuadArray);
	for (int i = 0; i < 5; ++i) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, bloomPass->vFramebuffers[i]->texture);
	}
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(0);

	framebufferFront->activate();
	copyShader->use();
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	drawScreenQuad(bloomPass->hFramebuffers[0]->texture);
	glDisable(GL_BLEND);
}

void Vis::drawFilmPass(float time, double *freqData, size_t freqDataSize) {
	float dbLow = getBandDb(freqData, freqDataSize, 20, 40);
	float dbHigh = getBandDb(freqData, freqDataSize, 2000, 16000);

	float seed = time - (long)time;
	float noiseLevel = lerpClamped(dbHigh, -100, -30, 0, 0.75);
	float scanlineLevel = lerpClamped(dbLow, -60, -30, 0, 0.25);

	framebufferBack->activate();
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	filmShader->use();
	filmShader->setFloat("seed", seed);
	filmShader->setFloat("noiseLevel", noiseLevel);
	filmShader->setFloat("scanlineLevel", scanlineLevel);
	filmShader->setFloat("scanlineCount", 648.0);
	drawScreenQuad(framebufferFront->texture);
	swapFramebuffers();
}

void Vis::drawScreenPass() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window->framebufferWidth, window->framebufferHeight);
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	copyShader->use();
	drawScreenQuad(framebufferFront->texture);
	glfwSwapBuffers(window->window);
}

void Vis::drawFrame(float time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize) {	
	populateOscData(timeData, timeDataSize);
	populateGridData(freqData, freqDataSize);

	framebufferFront->activate();
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// drawAxes();
	drawOsc();
	drawGrid(time);

	glDisable(GL_DEPTH_TEST);

	drawTiltShiftPass();
	drawBloomPass();
	drawFilmPass(time, freqData, freqDataSize);
	drawScreenPass();

	glfwPollEvents();
}

void Vis::drawEndFrame() {
	framebufferFront->activate();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	drawScreenQuad(endFrame);	

	drawScreenPass();
}

void Vis::readPixels(uint8_t *pixels) {
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferFront->id);
	glReadPixels(0, 0, GEN_VIDEO_WIDTH, GEN_VIDEO_HEIGHT, GL_BGRA, GL_UNSIGNED_BYTE, pixels);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
