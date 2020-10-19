#include "glcontext.h"

GlContext::GlContext(int windowWidth, int windowHeight, int internalWidth, int internalHeight)
: window(windowWidth, windowHeight, "cvis")
, copyShader("shaders/uv2d.vert", "shaders/copyShader.frag")
, pixels(4 * internalWidth * internalHeight)
, width(internalWidth)
, height(internalHeight) {
	framebufferFront = std::make_unique<gl::Framebuffer>(internalWidth, internalHeight, true, true);
	framebufferBack = std::make_unique<gl::Framebuffer>(internalWidth, internalHeight, true, true);

	float screenVertices[] = {
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	glBindVertexArray(screenArray);
	glBindBuffer(GL_ARRAY_BUFFER, screenBuffer);

	glBufferData(GL_ARRAY_BUFFER, sizeof(screenVertices), screenVertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void GlContext::swapFramebuffers() {
	framebufferFront.swap(framebufferBack);
}

void GlContext::drawFullScreen(unsigned int texture) {
	glBindVertexArray(screenArray);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

void GlContext::updateWindow() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window.framebufferWidth, window.framebufferHeight);
	glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	copyShader.use();
	drawFullScreen(framebufferFront->texture);
	glfwSwapBuffers(window.window);
	glfwPollEvents();
}

void GlContext::readPixels() {
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferFront->id);
	glReadPixels(0, 0, width, height, GL_BGRA, GL_UNSIGNED_BYTE, pixels.data());
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
