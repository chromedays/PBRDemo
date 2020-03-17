#version 450 

layout (location = 0) in vec3 inColor;

#if 0
layout (binding = 0, std140) uniform ViewUBO {
	mat4 mViewMat;
    mat4 mProjMat;
} view;

layout (binding = 1, std140) uniform InstanceUBO {
    mat4 mModelMat;
    float mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
} instance;
#endif

layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = vec4(inColor.rgb, 1);
}
