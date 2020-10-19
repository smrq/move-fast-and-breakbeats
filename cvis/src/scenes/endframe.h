#pragma once
#include "deps/gl.h"
#include "gl/wrappers.h"
#include "glcontext.h"

class EndFrameScene {
	std::shared_ptr<GlContext> glContext;
	gl::Texture frameTexture;

public:
	EndFrameScene(std::shared_ptr<GlContext> glContext, const char *frameFilename);
	void draw();
};
