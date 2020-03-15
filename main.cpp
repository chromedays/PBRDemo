#include <Common_3/OS/Interfaces/IApp.h>
#include <Common_3/Renderer/IRenderer.h>
#include <Common_3/Renderer/IResourceLoader.h>

#include <iostream>

struct UniformBlock
{
    mat4 mMVP;
};

constexpr uint32_t gImageCount = 3;

class Game : public IApp
{
  public:
    Renderer *pRenderer;
    Queue *pGraphicsQueue;
    CmdPool *pCmdPool;
    Cmd **ppCmds;

    Fence *pRenderCompleteFences[gImageCount];
    Semaphore *pRenderCompleteSemaphores[gImageCount];

    Semaphore *pImageAcquiredSemaphore;

    Shader *pUnlitShader;

    RootSignature *pRootSignature;

    DescriptorSet *pDescriptorSetUniforms;

    // RasterizerState* pRasterizerState;
    // RasterizerState* pWireframeRasterizerState;

    // DepthState* pDepthState;

    Buffer *pTriangleVertexBuffer;

    Buffer *pMVPUniformBuffer[gImageCount];

    SwapChain *pSwapChain;

    RenderTarget *pDepthBuffer;

    Pipeline *pPipeline;
    Pipeline *pWireframePipeline;

    uint32_t gFrameIndex;

    bool Init() override
    {
        // FILE PATHS
        PathHandle programDirectory = fsCopyProgramDirectoryPath();
        if (!fsPlatformUsesBundledResources())
        {
            PathHandle resourceDirRoot = fsAppendPathComponent(programDirectory, "../../../src/01_Transformations");
            fsSetResourceDirectoryRootPath(resourceDirRoot);
        }
        RendererDesc settings = {};
        initRenderer(GetName(), &settings, &pRenderer);
        if (!pRenderer)
            return false;

        QueueDesc queueDesc = {};
        queueDesc.mType = QUEUE_TYPE_GRAPHICS;
        queueDesc.mFlag = QUEUE_FLAG_INIT_MICROPROFILE;
        addQueue(pRenderer, &queueDesc, &pGraphicsQueue);
        CmdPoolDesc cmdPoolDesc = {};
        cmdPoolDesc.pQueue = pGraphicsQueue;
        addCmdPool(pRenderer, &cmdPoolDesc, &pCmdPool);
        CmdDesc cmdDesc = {};
        cmdDesc.pPool = pCmdPool;
        addCmd_n(pRenderer, &cmdDesc, gImageCount, &ppCmds);

        for (uint32_t i = 0; i < gImageCount; ++i)
        {
            addFence(pRenderer, &pRenderCompleteFences[i]);
            addSemaphore(pRenderer, &pRenderCompleteSemaphores[i]);
        }
        addSemaphore(pRenderer, &pImageAcquiredSemaphore);

        initResourceLoaderInterface(pRenderer);

#if 0
		ShaderLoadDesc unlitShaderLoadDesc = {};
		unlitShaderLoadDesc.mStages[0].pFileName = "unlit.vert";
		unlitShaderLoadDesc.mStages[0].pMacros = nullptr;
		unlitShaderLoadDesc.mStages[0].mMacroCount = 0;
		unlitShaderLoadDesc.mStages[0].mRoot = RD_SHADER_SOURCES;
		unlitShaderLoadDesc.mStages[1].pFileName = "unlit.frag";
		unlitShaderLoadDesc.mStages[1].pMacros = nullptr;
		unlitShaderLoadDesc.mStages[1].mMacroCount = 0;
		unlitShaderLoadDesc.mStages[1].mRoot = RD_SHADER_SOURCES;
		addShader(pRenderer, &unlitShaderLoadDesc, &pUnlitShader);

		Shader* pShaders[] = { pUnlitShader };
		RootSignatureDesc rootDesc = {};
		rootDesc.mShaderCount = static_cast<uint32_t>(eastl::size(pShaders));
		rootDesc.ppShaders = pShaders;
		addRootSignature(pRenderer, &rootDesc, &pRootSignature);

		DescriptorSetDesc descriptorSetDesc = {};
		descriptorSetDesc.pRootSignature = pRootSignature;
		descriptorSetDesc.mUpdateFrequency = DESCRIPTOR_UPDATE_FREQ_PER_FRAME;
		descriptorSetDesc.mMaxSets = gImageCount;
		addDescriptorSet(pRenderer, &descriptorSetDesc, &pDescriptorSetUniforms);
#endif

        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_BACK;
        rasterizerStateDesc.mFrontFace = FRONT_FACE_CCW;
        // addRasterizerState(pRenderer, &rasterizerStateDesc, &pRasterizerState);

        rasterizerStateDesc.mCullMode = CULL_MODE_NONE;
        rasterizerStateDesc.mFillMode = FILL_MODE_WIREFRAME;
        // addRasterizerState(pRenderer, &rasterizerStateDesc,
        // &pWireframeRasterizerState);

        DepthStateDesc depthStateDesc = {};
        depthStateDesc.mDepthTest = true;
        depthStateDesc.mDepthWrite = true;
        depthStateDesc.mDepthFunc = CMP_LEQUAL;
        // addDepthState(pRenderer, &depthStateDesc, &pDepthState);

        float trianglePoints[] = {
            -0.5f, -0.5f, 0.0f, 0.5f,  -0.5f, 0.0f, 0.5f,  0.5f,  0.0f,
            0.5f,  0.5f,  0.0f, -0.5f, 0.5f,  0.0f, -0.5f, -0.5f, 0.0f,
        };

        BufferLoadDesc triangleVbDesc = {};
        triangleVbDesc.ppBuffer = &pTriangleVertexBuffer;
        triangleVbDesc.pData = trianglePoints;
        triangleVbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
        triangleVbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
        triangleVbDesc.mDesc.mSize = sizeof(trianglePoints);
        // triangleVbDesc.mDesc.mVertexStride = sizeof(float) * 3;
        addResource(&triangleVbDesc, NULL, LOAD_PRIORITY_NORMAL);

        BufferLoadDesc ubDesc = {};
        ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
        ubDesc.mDesc.mSize = sizeof(UniformBlock);
        ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
        for (uint32_t i = 0; i < gImageCount; ++i)
        {
            ubDesc.ppBuffer = &pMVPUniformBuffer[i];
            addResource(&ubDesc, NULL, LOAD_PRIORITY_NORMAL);
        }
        waitForAllResourceLoads();

#if 0
		for (uint32_t i = 0; i < gImageCount; ++i)
		{
			DescriptorData param = {};
			param.pName = "uniformBlock";
			param.ppBuffers = &pMVPUniformBuffer[i];
			updateDescriptorSet(pRenderer, i, pDescriptorSetUniforms, 1, &param);
		}
#endif

        return true;
    }

    void Exit() override
    {
        waitQueueIdle(pGraphicsQueue);

        for (uint32_t i = 0; i < gImageCount; ++i)
        {
            removeResource(pMVPUniformBuffer[i]);
        }

        removeResource(pTriangleVertexBuffer);
        // removeDepthState(pDepthState);
        // removeRasterizerState(pWireframeRasterizerState);
        // removeRasterizerState(pRasterizerState);
        removeDescriptorSet(pRenderer, pDescriptorSetUniforms);
        removeRootSignature(pRenderer, pRootSignature);
        removeShader(pRenderer, pUnlitShader);
        exitResourceLoaderInterface(pRenderer);
        removeSemaphore(pRenderer, pImageAcquiredSemaphore);
        for (uint32_t i = 0; i < gImageCount; ++i)
        {
            removeSemaphore(pRenderer, pRenderCompleteSemaphores[i]);
            removeFence(pRenderer, pRenderCompleteFences[i]);
        }

        removeCmd_n(pRenderer, gImageCount, ppCmds);
        removeCmdPool(pRenderer, pCmdPool);
        removeQueue(pRenderer, pGraphicsQueue);
        removeRenderer(pRenderer);
    }

    bool Load() override
    {
        SwapChainDesc swapChainDesc = {};
        swapChainDesc.mWindowHandle = pWindow->handle;
        swapChainDesc.mPresentQueueCount = 1;
        swapChainDesc.ppPresentQueues = &pGraphicsQueue;
        swapChainDesc.mWidth = mSettings.mWidth;
        swapChainDesc.mHeight = mSettings.mHeight;
        swapChainDesc.mImageCount = gImageCount;
        swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(true);
        swapChainDesc.mEnableVsync = false;
        addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);
        if (!pSwapChain)
            return false;

        RenderTargetDesc depthRenderTargetDesc = {};
        depthRenderTargetDesc.mArraySize = 1;
        depthRenderTargetDesc.mClearValue.depth = 1.0f;
        depthRenderTargetDesc.mClearValue.stencil = 0;
        depthRenderTargetDesc.mDepth = 1;
        depthRenderTargetDesc.mFormat = TinyImageFormat_D32_SFLOAT;
        depthRenderTargetDesc.mWidth = mSettings.mWidth;
        depthRenderTargetDesc.mHeight = mSettings.mHeight;
        depthRenderTargetDesc.mSampleCount = SAMPLE_COUNT_1;
        depthRenderTargetDesc.mSampleQuality = 0;
        // depthRenderTargetDesc.mFlags = TEXTURE_CREATION_FLAG_ON_TILE
        addRenderTarget(pRenderer, &depthRenderTargetDesc, &pDepthBuffer);
        if (!pDepthBuffer)
            return false;

        VertexLayout vertexLayout = {};
        vertexLayout.mAttribCount = 1;
        vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
        vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
        vertexLayout.mAttribs[0].mBinding = 0;
        vertexLayout.mAttribs[0].mLocation = 0;
        vertexLayout.mAttribs[0].mOffset = 0;
        vertexLayout.mAttribs[1].mSemantic = SEMANTIC_NORMAL;
        vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
        vertexLayout.mAttribs[1].mBinding = 0;
        vertexLayout.mAttribs[1].mLocation = 1;
        vertexLayout.mAttribs[1].mOffset = sizeof(float) * 3;

#if 0
		PipelineDesc pipelineDesc = {};
		pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
		pipelineDesc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
		pipelineDesc.mGraphicsDesc.mRenderTargetCount = 1;
		pipelineDesc.mGraphicsDesc.pDepthState = pDepthState;
		pipelineDesc.mGraphicsDesc.pColorFormats = &pSwapChain->ppRenderTargets[0]->mDesc.mFormat;
		pipelineDesc.mGraphicsDesc.mSampleCount = pSwapChain->ppRenderTargets[0]->mDesc.mSampleCount;
		pipelineDesc.mGraphicsDesc.mSampleQuality = pSwapChain->ppRenderTargets[0]->mDesc.mSampleQuality;
		pipelineDesc.mGraphicsDesc.mDepthStencilFormat = pDepthBuffer->mDesc.mFormat;
		pipelineDesc.mGraphicsDesc.pRootSignature = pRootSignature;
		pipelineDesc.mGraphicsDesc.pShaderProgram = pUnlitShader;
		pipelineDesc.mGraphicsDesc.pVertexLayout = &vertexLayout;
		pipelineDesc.mGraphicsDesc.pRasterizerState = pRasterizerState;
		addPipeline(pRenderer, &pipelineDesc, &pPipeline);

		pipelineDesc.mGraphicsDesc.pRasterizerState = pWireframeRasterizerState;
		addPipeline(pRenderer, &pipelineDesc, &pWireframePipeline);
#endif

        return true;
    }
    void Unload() override
    {
        waitQueueIdle(pGraphicsQueue);

        removePipeline(pRenderer, pWireframePipeline);
        removePipeline(pRenderer, pPipeline);
        removeRenderTarget(pRenderer, pDepthBuffer);
        removeSwapChain(pRenderer, pSwapChain);
    }

    void Update(float deltaTime) override
    {
        mCamAngleDeg += 45.0f * deltaTime;
        if (mCamAngleDeg > 360.0f)
            mCamAngleDeg -= 360.0f;
    }

    void Draw() override
    {
        acquireNextImage(pRenderer, pSwapChain, pImageAcquiredSemaphore, NULL, &gFrameIndex);

        RenderTarget *pRenderTarget = pSwapChain->ppRenderTargets[gFrameIndex];
        Semaphore *pRenderCompleteSemaphore = pRenderCompleteSemaphores[gFrameIndex];
        Fence *pRenderCompleteFence = pRenderCompleteFences[gFrameIndex];

        FenceStatus fenceStatus;
        getFenceStatus(pRenderer, pRenderCompleteFence, &fenceStatus);
        if (fenceStatus == FENCE_STATUS_INCOMPLETE)
            waitForFences(pRenderer, 1, &pRenderCompleteFence);

        UniformBlock uniformData = {};
        float camAngleRad = degToRad(mCamAngleDeg);
        float vx = cosf(camAngleRad);
        float vz = sinf(camAngleRad);
        mat4 viewMat = mat4::lookAt({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f});
        mat4 modelMat = mat4::translation({0.0f, 0.0f, 1.1f}) * mat4::rotationY(camAngleRad);
        mat4 projMat = mat4::perspective(PI / 2.f, (float)mSettings.mHeight / (float)mSettings.mWidth, 0.1f, 100.0f);
        uniformData.mMVP = projMat * viewMat * modelMat;
        BufferUpdateDesc mvpUpdateDesc = {};
        mvpUpdateDesc.pBuffer = pMVPUniformBuffer[gFrameIndex];
        mvpUpdateDesc.pMappedData = &uniformData;
        beginUpdateResource(&mvpUpdateDesc);
        endUpdateResource(&mvpUpdateDesc, NULL);

        Cmd *cmd = ppCmds[gFrameIndex];
        beginCmd(cmd);

        LoadActionsDesc loadActions = {};
        loadActions.mLoadActionsColor[0] = LOAD_ACTION_CLEAR;
        loadActions.mClearColorValues[0].r = 1.0f;
        loadActions.mClearColorValues[0].g = 1.0f;
        loadActions.mClearColorValues[0].b = 0.0f;
        loadActions.mClearColorValues[0].a = 1.0f;
        loadActions.mLoadActionDepth = LOAD_ACTION_CLEAR;
        loadActions.mClearDepth.depth = 1.0f;
        loadActions.mClearDepth.stencil = 0;

        cmdBindRenderTargets(cmd, 1, &pRenderTarget, pDepthBuffer, &loadActions, NULL, NULL, -1, -1);
        cmdSetViewport(cmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
        cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);

#if 0
		cmdBindPipeline(cmd, pPipeline);
		cmdBindDescriptorSet(cmd, gFrameIndex, pDescriptorSetUniforms);
		cmdBindVertexBuffer(cmd, 1, &pTriangleVertexBuffer, NULL);
		cmdDraw(cmd, 6, 0);

		cmdBindPipeline(cmd, pWireframePipeline);
		cmdDraw(cmd, 6, 0);
#endif

        endCmd(cmd);

        QueueSubmitDesc queueSubmitDesc = {};
        queueSubmitDesc.mCmdCount = 1;
        queueSubmitDesc.ppCmds = &cmd;
        queueSubmitDesc.pSignalFence = pRenderCompleteFence;
        queueSubmitDesc.mWaitSemaphoreCount = 1;
        queueSubmitDesc.ppWaitSemaphores = &pImageAcquiredSemaphore;
        queueSubmitDesc.mSignalSemaphoreCount = 1;
        queueSubmitDesc.ppSignalSemaphores = &pRenderCompleteSemaphore;
        queueSubmit(pGraphicsQueue, &queueSubmitDesc);

        QueuePresentDesc queuePresentDesc = {};
        queuePresentDesc.pSwapChain = pSwapChain;
        queuePresentDesc.mIndex = gFrameIndex;
        queuePresentDesc.mWaitSemaphoreCount = 1;
        queuePresentDesc.ppWaitSemaphores = &pRenderCompleteSemaphore;
        queuePresent(pGraphicsQueue, &queuePresentDesc);
    }

    const char *GetName() override
    {
        return "Phong";
    }

  private:
    float mCamAngleDeg = 0.0f;
};

DEFINE_APPLICATION_MAIN(Game)
