#pragma once

#include <string>
#include "deps/gl.h"
#include "shader.h"
#include "framebuffer.h"
#include "bloom.h"
#include "constants.h"

class Vis {
	GLFWwindow *window;
	int windowFramebufferWidth;
	int windowFramebufferHeight;

	Framebuffer *framebufferFront;
	Framebuffer *framebufferBack;

	unsigned int axisBuffer;
	unsigned int axisArray;

	float oscDataX[VIS_OSC_COUNT+2];
	float oscDataY[VIS_OSC_COUNT+2];
	unsigned int oscBuffer;
	unsigned int oscArray;

	float gridDataXZ[VIS_GRID_COUNT][2];
	float gridDataYScale[VIS_GRID_COUNT][2];
	unsigned int gridBuffer;
	unsigned int gridArray;

	unsigned int screenQuadBuffer;
	unsigned int screenQuadArray;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	Shader *axisShader;
	Shader *lineShader;
	Shader *gridShader;
	Shader *tiltShiftShader;
	Shader *filmShader;
	Shader *copyShader;

	BloomPass *bloomPass;

	void initWindow();
	void initFramebuffers();
	void initAxisVertices();
	void initOscVertices();
	void initGridVertices();
	void initScreenQuadVertices();
	void initMatrices();
	void initShaders();

	void populateOscData(double *timeData, size_t timeDataSize);
	void populateGridData(double *freqData, size_t freqDataSize);

	void drawAxes();
	void drawOsc();
	void drawGrid(float time);
	void drawScreenQuad(unsigned int texture);
	void swapFramebuffers();

	void drawTiltShiftPass();
	void drawBloomPass();
	void drawFilmPass(float time, double *freqData, size_t freqDataSize);
	void drawScreenPass();

public:
	Vis();
	void drawFrame(float time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize);
	void readPixels(uint8_t *pixels);
	~Vis();
};
