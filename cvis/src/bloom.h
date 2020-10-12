#pragma once
#include "deps/gl.h"
#include "gl/framebuffer.h"
#include "gl/shader.h"
#include "constants.h"

struct BloomPass {
	std::unique_ptr<gl::Shader> highPassShader;
	std::unique_ptr<gl::Shader> blurShaders[5];
	std::unique_ptr<gl::Shader> compositeShader;
	std::unique_ptr<gl::Framebuffer> brightFramebuffer;
	std::unique_ptr<gl::Framebuffer> hFramebuffers[5];
	std::unique_ptr<gl::Framebuffer> vFramebuffers[5];

	BloomPass(int width, int height) {
		highPassShader = std::make_unique<gl::Shader>(
#include "shaders/bloomLuminosityHighPassShader.glsl"
		);

		blurShaders[0] = std::make_unique<gl::Shader>(
#define BLOOM_KERNEL_SIZE 3
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[1] = std::make_unique<gl::Shader>(
#define BLOOM_KERNEL_SIZE 5
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[2] = std::make_unique<gl::Shader>(
#define BLOOM_KERNEL_SIZE 7
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[3] = std::make_unique<gl::Shader>(
#define BLOOM_KERNEL_SIZE 9
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);
		blurShaders[4] = std::make_unique<gl::Shader>(
#define BLOOM_KERNEL_SIZE 11
#include "shaders/bloomBlurShader.glsl"
#undef BLOOM_KERNEL_SIZE
		);

		compositeShader = std::make_unique<gl::Shader>(
#include "shaders/bloomCompositeShader.glsl"
		);

		brightFramebuffer = std::make_unique<gl::Framebuffer>(width, height, false, true);

		int w = width / 2;
		int h = height / 2;
		for (int i = 0; i < 5; ++i) {
			hFramebuffers[i] = std::make_unique<gl::Framebuffer>(w, h, false, true);
			vFramebuffers[i] = std::make_unique<gl::Framebuffer>(w, h, false, true);
			w /= 2;
			h /= 2;
		}
	}
};
