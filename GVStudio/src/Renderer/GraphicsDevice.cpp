#include "Renderer/GraphicsDevice.h"
#include "3rdParty/glad/glad.h"

namespace GV
{
    void GraphicsDevice::Init()
    {
        glEnable(GL_DEPTH_TEST);
    }

    void GraphicsDevice::BeginFrame(uint32_t fbo, int width, int height)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, width, height);

        glClearColor(0.08f, 0.08f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void GraphicsDevice::EndFrame()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GraphicsDevice::BindShader(uint32_t shader)
    {
        glUseProgram(shader);
    }

    void GraphicsDevice::BindTexture(uint32_t tex)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tex);
    }

    void GraphicsDevice::SetViewport(int width, int height)
    {
        glViewport(0, 0, width, height);
    }

    void GraphicsDevice::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}