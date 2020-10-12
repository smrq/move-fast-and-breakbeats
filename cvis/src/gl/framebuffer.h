#pragma once
#include "deps/gl.h"

namespace gl {

struct Framebuffer {
	int width;
	int height;
	bool depth;
	bool alpha;
	unsigned int id;
	unsigned int texture;
	unsigned int renderbuffer;

	Framebuffer(int width, int height, bool depth, bool alpha)
	: width(width), height(height), depth(depth), alpha(alpha) {
		glGenFramebuffers(1, &id);
		glBindFramebuffer(GL_FRAMEBUFFER, id);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

		if (depth) {
			glGenRenderbuffers(1, &renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
		}

		int result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (result != GL_FRAMEBUFFER_COMPLETE) { fprintf(stderr, "Framebuffer is not complete: %d\n", result); exit(1); }

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void activate() {
		glBindFramebuffer(GL_FRAMEBUFFER, id);
		glViewport(0, 0, width, height);
	}

	~Framebuffer() {
		glDeleteRenderbuffers(1, &renderbuffer);
		glDeleteTextures(1, &texture);
		glDeleteFramebuffers(1, &id);
	}
};

}
