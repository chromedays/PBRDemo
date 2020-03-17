#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>

#include <iostream>

#include "render.h"

struct UniformBlock
{
    mat4 mMVP;
};

constexpr uint32_t gImageCount = 3;

class Game : public IApp
{
  public:
    Render mRender;

    Game() : mRender(this)
    {
    }

    bool Init() override
    {
        mRender.Init();
        return true;
    }

    void Exit() override
    {
        mRender.Exit();
    }

    bool Load() override
    {
        mRender.Load();
        return true;
    }

    void Unload() override
    {
        mRender.Unload();
    }

    void Update(float deltaTime) override
    {
    }

    void Draw() override
    {
    }

    const char *GetName() override
    {
        return "ChessGame";
    }
};

DEFINE_APPLICATION_MAIN(Game)
