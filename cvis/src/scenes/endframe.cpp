#include "endframe.h"

EndFrameScene::EndFrameScene(std::shared_ptr<GlContext> glContext, const char *frameFilename)
: glContext(glContext), frameTexture(frameFilename) {}

void EndFrameScene::draw() {
	glContext->framebufferFront->activate();
	glDisable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glContext->drawFullScreen(frameTexture);
}
