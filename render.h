#ifndef RENDER_H
#define RENDER_H

#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>

const uint32_t gSwapChainImageCount = 3;

struct Vertex
{
    float mPos[3];
    float mNormal[3];
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
    mat4 mViewMat;
    mat4 mProjMat;
};

struct LightSourcePipeline
{
    struct ViewUniformData
    {
        mat4 mViewProjMat;
    };

    struct InstanceUniformData
    {
        mat4 mModelMat;
        vec3 mColor;
    };

    Renderer *pRenderer;

    Shader *pShader;
    RootSignature *pRootSignature;
    DescriptorSet *pDescriptorSetView;
    DescriptorSet *pDescriptorSetInstance;
    Buffer *pLightSourceVb;
    int mLightSourceVerticesCount;
    Buffer *pViewUb[gSwapChainImageCount];
    Buffer *pInstanceUb[gSwapChainImageCount];

    Pipeline *pPipeline;

    void Init(Renderer *pRenderer);

    void Exit();
    void Load(SwapChain *pSwapChain, RenderTarget *pDepthBuffer);
    void Unload();

    void UpdateUb(const mat4 &projViewMat, int frameIndex);
    void BuildCmd(Cmd *pCmd);
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
    DescriptorSet *pDescriptorSetView;
    DescriptorSet *pDescriptorSetInstance;

    Buffer *pCubeVb;

    Buffer *pInstanceUb[gSwapChainImageCount];
    Buffer *pViewUb[gSwapChainImageCount];

    LightSourcePipeline mLightSourcePipeline;

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

    void Draw(const mat4 &viewMat);

  private:
    void AddPipeline();
};

#endif // RENDER_H
