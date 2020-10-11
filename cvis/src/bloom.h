#pragma once
#include "deps/gl.h"
#include "constants.h"
#include "shader.h"
#include "framebuffer.h"

struct BloomPass {
	Shader *highPassShader;
	Shader *blurShaders[5];
	Shader *compositeShader;
	Framebuffer *brightFramebuffer;
	Framebuffer *hFramebuffers[5];
	Framebuffer *vFramebuffers[5];

	BloomPass(int width, int height) {
		highPassShader = new Shader(
#include "shaders/bloomLuminosityHighPassShader.glsl"
		);

		blurShaders[0] = new Shader(
#define BLOOM_KERNEL_SIZE 3
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[1] = new Shader(
#define BLOOM_KERNEL_SIZE 5
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[2] = new Shader(
#define BLOOM_KERNEL_SIZE 7
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[3] = new Shader(
#define BLOOM_KERNEL_SIZE 9
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[4] = new Shader(
#define BLOOM_KERNEL_SIZE 11
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);

		compositeShader = new Shader(
#include "shaders/bloomCompositeShader.glsl"
		);

		brightFramebuffer = new Framebuffer(width, height, false, true);

		int w = width / 2;
		int h = height / 2;
		for (int i = 0; i < 5; ++i) {
			hFramebuffers[i] = new Framebuffer(w, h, false, true);
			vFramebuffers[i] = new Framebuffer(w, h, false, true);
			w /= 2;
			h /= 2;
		}
	}

	~BloomPass() {
		for (int i = 0; i < 5; ++i) {
			delete hFramebuffers[i];
			delete vFramebuffers[i];
			delete blurShaders[i];
		}
	}
};
