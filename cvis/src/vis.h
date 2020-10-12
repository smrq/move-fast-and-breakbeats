#pragma once

#include <memory>
#include <string>
#include "deps/gl.h"
#include "gl/window.h"
#include "gl/wrappers.h"
#include "gl/shader.h"
#include "gl/framebuffer.h"
#include "bloom.h"
#include "constants.h"

class Vis {
	std::shared_ptr<gl::Window> window;

	std::unique_ptr<gl::Framebuffer> framebufferFront;
	std::unique_ptr<gl::Framebuffer> framebufferBack;

	gl::Buffer axisBuffer;
	gl::VertexArray axisArray;

	float oscDataX[VIS_OSC_COUNT+2];
	float oscDataY[VIS_OSC_COUNT+2];
	gl::Buffer oscBuffer;
	gl::VertexArray oscArray;

	float gridDataXZ[VIS_GRID_COUNT][2];
	float gridDataYScale[VIS_GRID_COUNT][2];
	gl::Buffer gridBuffer;
	gl::VertexArray gridArray;

	gl::Buffer screenQuadBuffer;
	gl::VertexArray screenQuadArray;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	std::unique_ptr<gl::Shader> axisShader;
	std::unique_ptr<gl::Shader> lineShader;
	std::unique_ptr<gl::Shader> gridShader;
	std::unique_ptr<gl::Shader> tiltShiftShader;
	std::unique_ptr<gl::Shader> filmShader;
	std::unique_ptr<gl::Shader> copyShader;

	std::unique_ptr<BloomPass> bloomPass;

	gl::Texture endFrame;

	void initAxisVertices();
	void initOscVertices();
	void initGridVertices();
	void initScreenQuadVertices();

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
	Vis(std::shared_ptr<gl::Window> window, const char *endFrameFilename);
	void drawFrame(float time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize);
	void drawEndFrame();
	void readPixels(uint8_t *pixels);
};
