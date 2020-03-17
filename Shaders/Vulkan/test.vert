#version 450 

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;

#define MAX_LIGHTS_COUNT 20

layout (binding = 0, std140, UPDATE_FREQ_PER_FRAME) uniform ViewUbo {
	mat4 mViewMat;
    mat4 mProjMat;
	vec3 mCamPos;
	vec3 mLightPositions[MAX_LIGHTS_COUNT];
	vec3 mLightColors[MAX_LIGHTS_COUNT];
} view;

layout (binding = 1, std140, UPDATE_FREQ_PER_DRAW) uniform InstanceUBO {
    mat4 mModelMat;
    vec3 mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
} instance;

layout (location = 0) out vec3 outWorldPos;
layout (location = 1) out vec3 outNormal;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
	mat4 viewModelMat = view.mViewMat * instance.mModelMat;
    mat3 normalMat = mat3(transpose(inverse(instance.mModelMat)));
    outWorldPos = (instance.mModelMat * vec4(inPos.xyz, 1)).xyz;
    outNormal = normalMat * inNormal;
    //outColor = ((inNormal) + vec3(1)) * 0.5;
	gl_Position = view.mProjMat * viewModelMat * vec4(inPos.xyz, 1);
}