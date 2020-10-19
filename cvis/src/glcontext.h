#pragma once
#include <vector>
#include "gl/framebuffer.h"
#include "gl/shader.h"
#include "gl/window.h"
#include "gl/wrappers.h"

struct GlContext {
	gl::Window window;

	std::unique_ptr<gl::Framebuffer> framebufferFront;
	std::unique_ptr<gl::Framebuffer> framebufferBack;

	gl::Buffer screenBuffer;
	gl::VertexArray screenArray;

	gl::Shader copyShader;

	std::vector<uint8_t> pixels;
	int width;
	int height;

	GlContext(int windowWidth, int windowHeight, int internalWidth, int internalHeight);
	void swapFramebuffers();
	void drawFullScreen(unsigned int texture);
	void updateWindow();
	void readPixels();
};
