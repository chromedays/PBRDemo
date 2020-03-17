#include <Common_3/OS/Interfaces/ICameraController.h>
#include <Common_3/OS/Interfaces/IInput.h>

#include <iostream>

#include "render.h"

class App : public IApp
{
  public:
    Render mRender;

    ICameraController *pCamera;

    eastl::vector<vec3> mLightPositions;
    eastl::vector<vec3> mLightColors;

    bool Init() override
    {
        mLightPositions = {{-1, -1, -1}};
        mLightColors = {{0.5f, 0.5f, 1.f}};

        mRender.Init(this);

        pCamera = createFpsCameraController({0, 0, 0}, {0, 0, 1});
        pCamera->setMotionParameters({30.0f, 60.0f, 20.0f});

        initInputSystem(pWindow);
        InputActionDesc inputAction = {};
        inputAction.pUserData = this;

        inputAction.mBinding = InputBindings::BUTTON_ANY;
        inputAction.pFunction = [](InputActionContext *ctx) {
            setEnableCaptureInput(ctx->mPhase != INPUT_ACTION_PHASE_CANCELED);
            return true;
        };
        addInputAction(&inputAction);

        inputAction.mBinding = InputBindings::FLOAT_RIGHTSTICK;
        inputAction.pFunction = [](InputActionContext *ctx) {
            if (*ctx->pCaptured)
            {
                ICameraController *pCamera = ((App *)ctx->pUserData)->pCamera;
                pCamera->onRotate(ctx->mFloat2 * 0.2f);
            }
            return true;
        };
        addInputAction(&inputAction);

        inputAction.mBinding = InputBindings::FLOAT_LEFTSTICK;
        inputAction.pFunction = [](InputActionContext *ctx) {
            if (*ctx->pCaptured)
            {
                ICameraController *pCamera = ((App *)ctx->pUserData)->pCamera;
                pCamera->onMove(ctx->mFloat2);
            }
            return true;
        };
        addInputAction(&inputAction);

        mLightPositions = {
            {-2, -2, -2}, {-2, -2, 2}, {2, -2, -2}, {2, -2, 2}, {-2, 2, -2}, {-2, 2, 2}, {2, 2, -2}, {2, 2, 2},
        };

        mLightColors = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0},
                        {1, 0, 1}, {0, 1, 1}, {1, 1, 1}, {0.5f, 0.5f, 0.5f}};

        return true;
    }

    void Exit() override
    {
        mLightColors.set_capacity(0);
        mLightPositions.set_capacity(0);

        exitInputSystem();

        destroyCameraController(pCamera);

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
        updateInputSystem(mSettings.mWidth, mSettings.mHeight);
        // pCamera->onRotate({30 * deltaTime, 0.f});
        pCamera->update(deltaTime);
    }

    void Draw() override
    {
        mRender.Draw(pCamera->getViewMatrix(), pCamera->getViewPosition(), mLightPositions, mLightColors);
    }

    const char *GetName() override
    {
        return "ChessGame";
    }
};

DEFINE_APPLICATION_MAIN(App)
