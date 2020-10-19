#pragma once
#include <map>
#include "deps/gl.h"
#include "glcontext.h"
#include "gl/framebuffer.h"
#include "gl/shader.h"

struct BloomPass {
	std::shared_ptr<GlContext> glContext;

	std::unique_ptr<gl::Shader> highPassShader;
	std::unique_ptr<gl::Shader> blurShaders[5];
	std::unique_ptr<gl::Shader> compositeShader;
	std::unique_ptr<gl::Framebuffer> brightFramebuffer;
	std::unique_ptr<gl::Framebuffer> hFramebuffers[5];
	std::unique_ptr<gl::Framebuffer> vFramebuffers[5];

	BloomPass(std::shared_ptr<GlContext> glContext)
	: glContext(glContext) {
		highPassShader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomLuminosityHighPassShader.frag");
		blurShaders[0] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "3"}});
		blurShaders[1] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "5"}});
		blurShaders[2] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "7"}});
		blurShaders[3] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "9"}});
		blurShaders[4] = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomBlurShader.frag", gl::Shader::defines_t{{"BLOOM_KERNEL_SIZE", "11"}});
		compositeShader = std::make_unique<gl::Shader>("shaders/uv2d.vert", "shaders/bloomCompositeShader.frag");
		
		brightFramebuffer = std::make_unique<gl::Framebuffer>(glContext->width, glContext->height, false, true);

		int w = glContext->width / 2;
		int h = glContext->height / 2;
		for (int i = 0; i < 5; ++i) {
			hFramebuffers[i] = std::make_unique<gl::Framebuffer>(w, h, false, true);
			vFramebuffers[i] = std::make_unique<gl::Framebuffer>(w, h, false, true);
			w /= 2;
			h /= 2;
		}
	}

	void draw(float strength, float radius, float threshold) {
		brightFramebuffer->activate();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		highPassShader->use();
		highPassShader->setVec4("defaultColor", 0.0f, 0.0f, 0.0f, 0.0f);
		highPassShader->setFloat("luminosityThreshold", threshold);
		highPassShader->setFloat("smoothWidth", 0.01f);
		glContext->drawFullScreen(glContext->framebufferFront->texture);

		unsigned int inputTexture = brightFramebuffer->texture;
		for (int i = 0; i < 5; ++i) {
			hFramebuffers[i]->activate();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			blurShaders[i]->use();
			blurShaders[i]->setVec2("texSize",
				(float)hFramebuffers[i]->width,
				(float)hFramebuffers[i]->height);
			blurShaders[i]->setVec2("direction", 1.0f, 0.0f);
			glContext->drawFullScreen(inputTexture);

			vFramebuffers[i]->activate();
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			blurShaders[i]->setVec2("direction", 0.0f, 1.0f);
			glContext->drawFullScreen(hFramebuffers[i]->texture);

			inputTexture = vFramebuffers[i]->texture;
		}

		hFramebuffers[0]->activate();
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		compositeShader->use();
		compositeShader->setInt("blurTexture0", 0);
		compositeShader->setInt("blurTexture1", 1);
		compositeShader->setInt("blurTexture2", 2);
		compositeShader->setInt("blurTexture3", 3);
		compositeShader->setInt("blurTexture4", 4);
		compositeShader->setFloat("strength", strength);
		compositeShader->setFloat("radius", radius);
		float factors[5] = { 1.0f, 0.8f, 0.6f, 0.4f, 0.2f };
		compositeShader->setFloatArray("factors", 5, factors);
		glm::vec3 tintColors[5] = {
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f),
			glm::vec3(1.0f, 1.0f, 1.0f)
		};
		compositeShader->setVec3Array("tintColors", 5, tintColors);
		glBindVertexArray(glContext->screenArray);
		for (int i = 0; i < 5; ++i) {
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, vFramebuffers[i]->texture);
		}
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(0);

		glContext->framebufferFront->activate();
		glContext->copyShader.use();
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glContext->drawFullScreen(hFramebuffers[0]->texture);
		glDisable(GL_BLEND);
	}
};
