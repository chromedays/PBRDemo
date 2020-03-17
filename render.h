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

struct MainInstanceUniformData
{
    mat4 mModelMat;
    vec3 mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
};

#define MAX_LIGHTS_COUNT 20

struct MainViewUniformData
{
    mat4 mViewMat;
    mat4 mProjMat;
    vec3 mCamPos;
    vec3 mLightPositions[MAX_LIGHTS_COUNT];
    vec3 mLightColors[MAX_LIGHTS_COUNT];
    int mLightsCount;
};

struct LightSourcePipeline
{
    struct ViewUniformData
    {
        mat4 mViewProjMat;
    };

    struct InstanceUniformData
    {
        mat4 mModelMat[MAX_LIGHTS_COUNT];
        vec3 mColor[MAX_LIGHTS_COUNT];
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

    uint32_t mInstancesCount;

    void Init(Renderer *pRenderer);
    void Exit();
    void Load(SwapChain *pSwapChain, RenderTarget *pDepthBuffer);
    void Unload();

    void UpdateUb(int frameIndex, const mat4 &projViewMat, int lightsCount, const eastl::vector<vec3> &lightPositions,
                  const eastl::vector<vec3> &lightColors);
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

    void Init(IApp *pApp);
    void Exit();
    void Load();
    void Unload();
    void Draw(const mat4 &viewMat, const vec3 &camPos, int lightsCount, const eastl::vector<vec3> &lightPositions,
              const eastl::vector<vec3> &lightColors);

  private:
    void AddPipeline();
};

#endif // RENDER_H
