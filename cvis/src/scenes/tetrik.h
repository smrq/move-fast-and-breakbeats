#pragma once
#include "deps/gl.h"
#include "glcontext.h"
#include "gl/framebuffer.h"
#include "gl/shader.h"
#include "gl/wrappers.h"
#include "postprocessing/bloom.h"
#include "postprocessing/film.h"
#include "postprocessing/tiltshift.h"
#include "util.h"

#define TETRIK_OSC_COUNT  1024
#define TETRIK_GRID_SIZE  64
#define TETRIK_GRID_COUNT (TETRIK_GRID_SIZE*TETRIK_GRID_SIZE)
#define TETRIK_MIN_FREQ   40
#define TETRIK_MAX_FREQ   16000

class TetrikScene {
	std::shared_ptr<GlContext> glContext;

	gl::Buffer axisBuffer;
	gl::VertexArray axisArray;

	float oscDataX[TETRIK_OSC_COUNT+2];
	float oscDataY[TETRIK_OSC_COUNT+2];
	gl::Buffer oscBuffer;
	gl::VertexArray oscArray;

	float gridDataXZ[TETRIK_GRID_COUNT][2];
	float gridDataYScale[TETRIK_GRID_COUNT][2];
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

	void initAxisVertices();
	void initOscVertices();
	void initGridVertices();

	void populateOscData(double *timeData, size_t timeDataSize);
	void populateGridData(double *freqData, size_t freqDataSize, int sampleRate);

	void drawAxes();
	void drawOsc();
	void drawGrid(float time);

public:
	TetrikScene(std::shared_ptr<GlContext> glContext);
	void draw(float time, double *timeData, size_t timeDataSize, double *freqData, size_t freqDataSize, int sampleRate);
};
