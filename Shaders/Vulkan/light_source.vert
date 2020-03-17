#version 460 
layout (location = 0) in vec3 inPos;

layout (binding = 0, std140, UPDATE_FREQ_PER_FRAME) uniform ViewUbo {
	mat4 mViewProjMat;
} view;

layout (binding = 1, std140, UPDATE_FREQ_PER_DRAW) uniform InstanceUbo {
    mat4 mModelMat;
    vec3 mColor;
} instance;

layout (location = 0) out vec3 outColor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    outColor = instance.mColor;
	gl_Position = view.mViewProjMat * instance.mModelMat * vec4(inPos.xyz, 1);
}
