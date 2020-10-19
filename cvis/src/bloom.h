#pragma once
#include <map>
#include <memory>
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
		highPassShader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomLuminosityHighPassShader.frag");
		blurShaders[0] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "3"}});
		blurShaders[1] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "5"}});
		blurShaders[2] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "7"}});
		blurShaders[3] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "9"}});
		blurShaders[4] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "11"}});
		compositeShader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomCompositeShader.frag");
		
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
