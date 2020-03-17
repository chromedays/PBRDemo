#include <Common_3/OS/Interfaces/ICameraController.h>
#include <Common_3/OS/Interfaces/IInput.h>

#include <iostream>

#include "render.h"

class App : public IApp
{
  public:
    Render mRender;

    ICameraController *pCamera;

    App() : mRender(this)
    {
    }

    bool Init() override
    {
        mRender.Init();

        pCamera = createFpsCameraController({0, 0, 0}, {0, 0, 1});
        pCamera->setMotionParameters({10.0f, 60.0f, 20.0f});

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

        return true;
    }

    void Exit() override
    {
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
        mRender.Draw(pCamera->getViewMatrix());
    }

    const char *GetName() override
    {
        return "ChessGame";
    }
};

DEFINE_APPLICATION_MAIN(App)
