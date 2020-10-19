#pragma once
#include "deps/gl.h"
#include "glcontext.h"
#include "gl/shader.h"

struct FilmPass {
	std::shared_ptr<GlContext> glContext;
	std::unique_ptr<gl::Shader> shader;

	FilmPass(std::shared_ptr<GlContext> glContext)
	: glContext(glContext) {
		shader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/filmShader.frag");
	}

	void draw(float time, float noiseLevel, float scanlineLevel, float scanlineCount) {
		glContext->framebufferBack->activate();
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader->use();
		shader->setFloat("seed", time - (long)time);
		shader->setFloat("noiseLevel", noiseLevel);
		shader->setFloat("scanlineLevel", scanlineLevel);
		shader->setFloat("scanlineCount", scanlineCount);
		glContext->drawFullScreen(glContext->framebufferFront->texture);
		glContext->swapFramebuffers();
	}
};
