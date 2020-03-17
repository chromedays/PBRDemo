#include "render.h"
#include <iterator>

void Render::Init()
{
    RendererDesc rendererDesc = {};
    rendererDesc.mApi = RENDERER_API_VULKAN;
    initRenderer(pApp->GetName(), &rendererDesc, &pRenderer);

    QueueDesc queueDesc = {};
    queueDesc.mType = QUEUE_TYPE_GRAPHICS;
    addQueue(pRenderer, &queueDesc, &pGraphicsQueue);

    CmdPoolDesc cmdPoolDesc = {};
    cmdPoolDesc.pQueue = pGraphicsQueue;
    addCmdPool(pRenderer, &cmdPoolDesc, &pCmdPool);

    CmdDesc cmdDesc = {};
    cmdDesc.pPool = pCmdPool;
    addCmd_n(pRenderer, &cmdDesc, gSwapChainImageCount, &ppCmds);

    for (uint32_t i = 0; i < gSwapChainImageCount; i++)
    {
        addFence(pRenderer, &pRenderCompleteFences[i]);
        addSemaphore(pRenderer, &pRenderCompleteSemaphores[i]);
    }

    addSemaphore(pRenderer, &pImageAcquiredSemaphore);

    initResourceLoaderInterface(pRenderer);
    PathHandle programDirectory = fsCopyProgramDirectoryPath();
    if (!fsPlatformUsesBundledResources())
    {
        PathHandle resourceDirRoot = fsAppendPathComponent(programDirectory, "../../");
        fsSetResourceDirectoryRootPath(resourceDirRoot);
    }

    ShaderLoadDesc shaderDesc = {};
    shaderDesc.mStages[0].pFileName = "test.vert";
    shaderDesc.mStages[0].mRoot = RD_SHADER_SOURCES;
    shaderDesc.mStages[1].pFileName = "test.frag";
    shaderDesc.mStages[1].mRoot = RD_SHADER_SOURCES;
    shaderDesc.mTarget = rendererDesc.mShaderTarget;

    addShader(pRenderer, &shaderDesc, &pChessPieceShader);
    // addSampler()

    float *pCubePoints;
    int cubePointsCount;
    generateCuboidPoints(&pCubePoints, &cubePointsCount);
    BufferLoadDesc cubeVbLoad = {};
    cubeVbLoad.ppBuffer = &pCubeVb;
    cubeVbLoad.pData = pCubePoints;
    cubeVbLoad.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    cubeVbLoad.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    cubeVbLoad.mDesc.mSize = cubePointsCount * sizeof(float);
    addResource(&cubeVbLoad, NULL, LOAD_PRIORITY_NORMAL);

    BufferLoadDesc viewUbLoad = {};
    viewUbLoad.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    viewUbLoad.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    viewUbLoad.mDesc.mSize = sizeof(ViewUniformData);
    for (uint32_t i = 0; i < std::size(pViewUb); i++)
    {
        viewUbLoad.ppBuffer = &pViewUb[i];
        addResource(&viewUbLoad, NULL, LOAD_PRIORITY_NORMAL);
    }

    BufferLoadDesc instanceUbLoad = {};
    instanceUbLoad.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    instanceUbLoad.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    instanceUbLoad.mDesc.mSize = sizeof(InstanceUniformData);
    for (size_t i = 0; i < std::size(pInstanceUb); i++)
    {
        instanceUbLoad.ppBuffer = &pInstanceUb[i];
        addResource(&instanceUbLoad, NULL, LOAD_PRIORITY_NORMAL);
    }
    // viewUbLoad.ppBuffer = &pViewUb;

    waitForAllResourceLoads();

    conf_free(pCubePoints);

    RootSignatureDesc rootSignatureDesc = {};
    rootSignatureDesc.ppShaders = &pChessPieceShader;
    rootSignatureDesc.mShaderCount = 1;
    // rootSignatureDesc.mMaxBindlessTextures;
    // rootSignatureDesc.ppStaticSamplerNames;
    // rootSignatureDesc.ppStaticSamplers;
    // rootSignatureDesc.mStaticSamplerCount;
    rootSignatureDesc.mFlags;
    addRootSignature(pRenderer, &rootSignatureDesc, &pRootSignature);

    DescriptorSetDesc descriptorSetDesc = {};
    descriptorSetDesc.pRootSignature = pRootSignature;
    descriptorSetDesc.mUpdateFrequency = DESCRIPTOR_UPDATE_FREQ_PER_FRAME;
    descriptorSetDesc.mMaxSets = gSwapChainImageCount;
    addDescriptorSet(pRenderer, &descriptorSetDesc, &pDescriptorSet);

    for (uint32_t i = 0; i < gSwapChainImageCount; i++)
    {
        DescriptorData descriptorUpdate[2] = {};
        descriptorUpdate[0].pName = "view";
        descriptorUpdate[0].ppBuffers = &pViewUb[i];
        descriptorUpdate[1].pName = "instance";
        descriptorUpdate[1].ppBuffers = &pInstanceUb[i];
        updateDescriptorSet(pRenderer, i, pDescriptorSet, (uint32_t)std::size(descriptorUpdate), descriptorUpdate);
    }
}

void Render::Exit()
{
    removeDescriptorSet(pRenderer, pDescriptorSet);
    removeRootSignature(pRenderer, pRootSignature);
    for (size_t i = 0; i < std::size(pInstanceUb); i++)
        removeResource(pInstanceUb[i]);
    for (size_t i = 0; i < std::size(pViewUb); i++)
        removeResource(pViewUb[i]);
    removeResource(pCubeVb);
    removeShader(pRenderer, pChessPieceShader);
    exitResourceLoaderInterface(pRenderer);
    removeSemaphore(pRenderer, pImageAcquiredSemaphore);
    for (size_t i = 0; i < std::size(pRenderCompleteFences); i++)
        removeFence(pRenderer, pRenderCompleteFences[i]);
    for (size_t i = 0; i < std::size(pRenderCompleteSemaphores); i++)
        removeSemaphore(pRenderer, pRenderCompleteSemaphores[i]);
    removeCmd_n(pRenderer, gSwapChainImageCount, ppCmds);
    removeCmdPool(pRenderer, pCmdPool);
    removeQueue(pRenderer, pGraphicsQueue);
    removeRenderer(pRenderer);
}

void Render::Load()
{
    SwapChainDesc swapChainDesc = {};
    swapChainDesc.mWindowHandle = pApp->pWindow->handle;
    swapChainDesc.ppPresentQueues = &pGraphicsQueue;
    swapChainDesc.mPresentQueueCount = 1;
    swapChainDesc.mImageCount = gSwapChainImageCount;
    swapChainDesc.mWidth = pApp->mSettings.mWidth;
    swapChainDesc.mHeight = pApp->mSettings.mHeight;
    swapChainDesc.mColorFormat = getRecommendedSwapchainFormat(true);
    swapChainDesc.mColorClearValue.r = 1;
    swapChainDesc.mColorClearValue.g = 1;
    swapChainDesc.mColorClearValue.b = 1;
    swapChainDesc.mColorClearValue.a = 1;
    addSwapChain(pRenderer, &swapChainDesc, &pSwapChain);

    RenderTargetDesc depthRTDesc = {};
    depthRTDesc.mFlags = TEXTURE_CREATION_FLAG_ON_TILE;
    depthRTDesc.mWidth = pApp->mSettings.mWidth;
    depthRTDesc.mHeight = pApp->mSettings.mHeight;
    depthRTDesc.mDepth = 1;
    depthRTDesc.mArraySize = 1;
    depthRTDesc.mMipLevels = 1;
    depthRTDesc.mSampleCount = SAMPLE_COUNT_1;
    depthRTDesc.mFormat = TinyImageFormat_D32_SFLOAT;
    depthRTDesc.mClearValue.depth = 0;
    depthRTDesc.mClearValue.stencil = 0;
    depthRTDesc.mSampleQuality = 0;
    addRenderTarget(pRenderer, &depthRTDesc, &pDepthBuffer);

    AddPipeline();
}

void Render::Unload()
{
    removePipeline(pRenderer, pPipeline);
    pPipeline = NULL;
    removeRenderTarget(pRenderer, pDepthBuffer);
    pDepthBuffer = NULL;
    removeSwapChain(pRenderer, pSwapChain);
    pSwapChain = NULL;
}

void Render::AddPipeline()
{
    VertexLayout vertexLayout = {};
    vertexLayout.mAttribCount = 2;
    vertexLayout.mAttribs[0].mSemantic = SEMANTIC_POSITION;
    vertexLayout.mAttribs[0].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
    vertexLayout.mAttribs[0].mBinding = 0;
    vertexLayout.mAttribs[0].mLocation = 0;
    vertexLayout.mAttribs[0].mOffset = offsetof(Vertex, mPos);
    vertexLayout.mAttribs[0].mRate = VERTEX_ATTRIB_RATE_VERTEX;
    vertexLayout.mAttribs[1].mSemantic = SEMANTIC_NORMAL;
    vertexLayout.mAttribs[1].mFormat = TinyImageFormat_R32G32B32_SFLOAT;
    vertexLayout.mAttribs[1].mBinding = 0;
    vertexLayout.mAttribs[1].mLocation = 1;
    vertexLayout.mAttribs[1].mOffset = offsetof(Vertex, mNormal);
    vertexLayout.mAttribs[1].mRate = VERTEX_ATTRIB_RATE_VERTEX;

    DepthStateDesc depthState = {};
    depthState.mDepthTest = true;
    depthState.mDepthWrite = true;
    depthState.mDepthFunc = CMP_LEQUAL;

    RasterizerStateDesc rasterizerState = {};
    rasterizerState.mCullMode = CULL_MODE_NONE;
    rasterizerState.mFillMode = FILL_MODE_SOLID;
    rasterizerState.mMultiSample = true;
    rasterizerState.mScissor = true;
    rasterizerState.mFrontFace = FRONT_FACE_CCW;

    PipelineDesc pipelineDesc = {};
    pipelineDesc.mType = PIPELINE_TYPE_GRAPHICS;
    pipelineDesc.mGraphicsDesc.pShaderProgram = pChessPieceShader;
    pipelineDesc.mGraphicsDesc.pRootSignature = pRootSignature;
    pipelineDesc.mGraphicsDesc.pVertexLayout = &vertexLayout;
    pipelineDesc.mGraphicsDesc.pDepthState = &depthState;
    pipelineDesc.mGraphicsDesc.pRasterizerState = &rasterizerState;
    pipelineDesc.mGraphicsDesc.pColorFormats = &pSwapChain->ppRenderTargets[0]->mFormat;
    pipelineDesc.mGraphicsDesc.mRenderTargetCount = 1;
    pipelineDesc.mGraphicsDesc.mSampleCount = pSwapChain->ppRenderTargets[0]->mSampleCount;
    pipelineDesc.mGraphicsDesc.mSampleQuality = pSwapChain->ppRenderTargets[0]->mSampleQuality;
    pipelineDesc.mGraphicsDesc.mDepthStencilFormat = pDepthBuffer->mFormat;
    pipelineDesc.mGraphicsDesc.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;

    addPipeline(pRenderer, &pipelineDesc, &pPipeline);
}
