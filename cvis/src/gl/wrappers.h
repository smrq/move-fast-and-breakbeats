#pragma once
#include "deps/gl.h"

namespace gl {

class VertexArray {
	unsigned int id;
public:
	VertexArray() { glGenVertexArrays(1, &id); }
	~VertexArray() { glDeleteVertexArrays(1, &id); }
	operator unsigned int() { return id; }
};

class Buffer {
	unsigned int id;
public:
	Buffer() { glGenBuffers(1, &id); }
	~Buffer() { glDeleteBuffers(1, &id); }
	operator unsigned int() { return id; }
};

class Texture {
	unsigned int id;

public:
	Texture(
		const char *filename,
		GLenum wrapS = GL_REPEAT,
		GLenum wrapT = GL_REPEAT,
		GLenum minFilter = GL_LINEAR,
		GLenum magFilter = GL_LINEAR,
		bool alpha = false
	) {
		int width, height, channels;
		stbi_set_flip_vertically_on_load(true);
		uint8_t *data = stbi_load(filename, &width, &height, &channels, 0);
		if (data == NULL) {
			fprintf(stderr, "Could not load image file %s\n", filename);
			exit(1);
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilter);
		glTexImage2D(GL_TEXTURE_2D, 0, alpha ? GL_RGBA : GL_RGB, width, height, 0, alpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		stbi_image_free(data);
	}

	~Texture() { glDeleteTextures(1, &id); }
	operator unsigned int() { return id; }
};

}
