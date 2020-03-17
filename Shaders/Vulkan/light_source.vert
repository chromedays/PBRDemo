#version 450 
layout (location = 0) in vec3 inPos;

layout (binding = 0, std140, UPDATE_FREQ_PER_FRAME) uniform ViewUbo {
	mat4 mViewProjMat;
} view;

#define MAX_LIGHTS_COUNT 20

layout (binding = 1, std140, UPDATE_FREQ_PER_DRAW) uniform InstanceUbo {
    mat4 mModelMat[MAX_LIGHTS_COUNT];
    vec3 mColor[MAX_LIGHTS_COUNT];
} instance;

layout (location = 0) out vec3 outColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    outColor = instance.mColor[gl_InstanceIndex];
	gl_Position = view.mViewProjMat * instance.mModelMat[gl_InstanceIndex] * vec4(inPos.xyz, 1);
}
