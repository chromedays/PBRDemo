#version 450 

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

layout (binding = 0, std140, UPDATE_FREQ_PER_FRAME) uniform ViewUbo {
	mat4 mViewMat;
    mat4 mProjMat;
} view;

layout (binding = 1, std140, UPDATE_FREQ_PER_DRAW) uniform InstanceUbo {
    mat4 mModelMat;
    float mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
} instance;

layout (location = 0) out vec3 outColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
	mat4 viewModelMat = view.mViewMat * instance.mModelMat;
    mat3 normalMat = mat3(transpose(inverse(viewModelMat)));
    outColor = ((inNormal) + vec3(1)) * 0.5;
	gl_Position = view.mProjMat * viewModelMat * vec4(inPos.xyz, 1);
}