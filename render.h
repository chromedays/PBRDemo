#ifndef RENDER_H
#define RENDER_H

#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>

const uint32_t gSwapChainImageCount = 3;

struct Vertex
{
    vec3 mPos;
    vec3 mNormal;
};

struct InstanceUniformData
{
    mat4 mModelMat;
    float mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
};

struct ViewUniformData
{
    mat4 mProjectViewMat;
};

struct Render
{
    IApp *pApp;

    Renderer *pRenderer;
    Queue *pGraphicsQueue;
    CmdPool *pCmdPool;
    Cmd **ppCmds;

    Fence *pRenderCompleteFences[gSwapChainImageCount];
    Semaphore *pRenderCompleteSemaphores[gSwapChainImageCount];

    Semaphore *pImageAcquiredSemaphore;

    Shader *pChessPieceShader;

    RootSignature *pRootSignature;
    DescriptorSet *pDescriptorSet;

    Buffer *pCubeVb;

    Buffer *pInstanceUb[gSwapChainImageCount];
    Buffer *pViewUb[gSwapChainImageCount];

    SwapChain *pSwapChain;
    RenderTarget *pDepthBuffer;
    Pipeline *pPipeline;

    uint32_t mFrameIndex;

    explicit Render(IApp *pApp) : pApp(pApp)
    {
    }

    void Init();

    void Exit();

    void Load();

    void Unload();

    void AddPipeline();
};

#endif // RENDER_H
