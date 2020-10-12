#pragma once
#include "deps/gl.h"

namespace gl {

struct Window {
	GLFWwindow *window;
	int framebufferWidth;
	int framebufferHeight;

	Window(unsigned int width, unsigned int height, const char *title) {
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	#endif

		window = glfwCreateWindow(width, height, title, NULL, NULL);
		if (window == NULL) {
			fprintf(stderr, "Failed to create GLFW window\n");
			glfwTerminate();
			exit(1);
		}

		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			fprintf(stderr, "Failed to initialize GLAD\n");
			glfwTerminate();
			exit(1);
		}
	}

	~Window() {
		glfwTerminate();
	}
};

}
