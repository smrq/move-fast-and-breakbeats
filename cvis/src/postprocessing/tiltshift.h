#pragma once
#include "deps/gl.h"
#include "glcontext.h"
#include "gl/shader.h"

struct TiltShiftPass {
	std::shared_ptr<GlContext> glContext;
	std::unique_ptr<gl::Shader> shader;

	TiltShiftPass(std::shared_ptr<GlContext> glContext)
	: glContext(glContext) {
		shader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/tiltShiftShader.frag");
	}

	void draw(double focusPosition, double amount, double brightness) {
		glContext->framebufferBack->activate();
		glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		shader->use();
		shader->setFloat("focusPosition", (float)focusPosition);
		shader->setFloat("amount", (float)amount);
		shader->setFloat("brightness", (float)brightness);
		glContext->drawFullScreen(glContext->framebufferFront->texture);
		glContext->swapFramebuffers();
	}
};
