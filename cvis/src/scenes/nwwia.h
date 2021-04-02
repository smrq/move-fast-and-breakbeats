#pragma once
#include <map>
#include <string>
#include <vector>
#include "deps/gl.h"
#include "glcontext.h"
#include "gl/framebuffer.h"
#include "gl/shader.h"
#include "gl/wrappers.h"
#include "postprocessing/bloom.h"
#include "postprocessing/film.h"
#include "postprocessing/tiltshift.h"
#include "util.h"

#define NWWIA_OSC_COUNT  1024
#define NWWIA_GRID_SIZE  64
#define NWWIA_GRID_COUNT (NWWIA_GRID_SIZE*NWWIA_GRID_SIZE)
#define NWWIA_MIN_FREQ   40
#define NWWIA_MAX_FREQ   16000

class NwwiaScene {
	std::shared_ptr<GlContext> glContext;

	gl::Buffer axisBuffer;
	gl::VertexArray axisArray;

	std::vector<std::multimap<double, double>> automation;

	float oscDataX[NWWIA_OSC_COUNT+2];
	float oscDataY[NWWIA_OSC_COUNT+2];
	gl::Buffer oscBuffer;
	gl::VertexArray oscArray;

	float gridDataXZ[NWWIA_GRID_COUNT][2];
	float gridDataYScale[NWWIA_GRID_COUNT][2];
	gl::Buffer gridBuffer;
	gl::VertexArray gridArray;

	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;

	std::unique_ptr<gl::Shader> axisShader;
	std::unique_ptr<gl::Shader> lineShader;
	std::unique_ptr<gl::Shader> gridShader;

	std::unique_ptr<TiltShiftPass> tiltShiftPass;
	std::unique_ptr<BloomPass> bloomPass;
	std::unique_ptr<FilmPass> filmPass;

	void readAutomationFiles(std::vector<std::string> &automationFilenames);
	double getAutomationLevel(int trackIndex, double time);

	void initAxisVertices();
	void initOscVertices();
	void initGridVertices();

	void populateOscData(double *timeData, size_t timeDataSize);
	void populateGridData(double *freqData, size_t freqDataSize, int sampleRate);

	void drawAxes();
	void drawOsc();
	void drawGrid(double time);

public:
	NwwiaScene(std::shared_ptr<GlContext> glContext, std::vector<std::string> &automationFilenames);
	void draw(double time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize, int sampleRate);
};
