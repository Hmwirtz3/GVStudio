#pragma once

#include <cstdint>

namespace GV
{
    class GraphicsDevice
    {
    public:
        void Init();

        void BeginFrame(uint32_t fbo, int width, int height);
        void EndFrame();

        void BindShader(uint32_t shader);

        void BindTexture(uint32_t tex);

        void SetViewport(int width, int height);
        void Clear();

    };
}