#version 450 

layout (location = 0) in vec3 inWorldPos;
layout (location = 1) in vec3 inNormal;

#define MAX_LIGHTS_COUNT 20

layout (binding = 0, std140, UPDATE_FREQ_PER_FRAME) uniform ViewUbo {
	mat4 mViewMat;
    mat4 mProjMat;
	vec3 mCamPos;
	vec3 mLightPositions[MAX_LIGHTS_COUNT];
	vec3 mLightColors[MAX_LIGHTS_COUNT];
	int mLightsCount;
} view;

layout (binding = 1, std140, UPDATE_FREQ_PER_DRAW) uniform InstanceUBO {
    mat4 mModelMat;
    vec3 mAlbedo;
    float mMetallic;
    float mRoughness;
    float mAO;
} instance;

layout (location = 0) out vec4 outFragColor;

const float pi = 3.14159265359;

vec3 fresnel_schlick(float cos_theta, vec3 f0)
{
	return f0 + (1.0 - f0) * pow(1.0 - cos_theta, 5.0);
}

float distribution_ggx(vec3 n, vec3 h, float roughness)
{
	float a = roughness * roughness;
	float a_2 = a * a;
	float n_dot_h = max(dot(n, h), 0);
	float n_dot_h_2 = n_dot_h * n_dot_h;

	float num = a_2;
	float denom = (n_dot_h_2 * (a_2 - 1.0) + 1.0);
	denom = pi * denom * denom;

	return num / denom;
}

float geometry_schlick_ggx(float n_dot_v, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float num = n_dot_v;
	float denom = n_dot_v * (1.0 - k) + k;

	return num / denom;
}

float geometry_smith(vec3 n, vec3 v, vec3 l, float roughness)
{
	float n_dot_v = max(dot(n, v), 0);
	float n_dot_l = max(dot(n, l), 0);
	float ggx2 = geometry_schlick_ggx(n_dot_v, roughness);
	float ggx1 = geometry_schlick_ggx(n_dot_l, roughness);

	return ggx1 * ggx2;
}

void main()
{
	vec3 n = normalize(inNormal);
	vec3 v = normalize(view.mCamPos - inWorldPos);

	vec3 lo = vec3(0);
	for (int i = 0; i < view.mLightsCount; i++)
	{
		vec3 l = normalize(view.mLightPositions[i] - inWorldPos);
		vec3 h = normalize(v + l);

		float distance = length(view.mLightPositions[i] - inWorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = view.mLightColors[i] * attenuation;

		vec3 f0 = vec3(0.04);
		f0 = mix(f0, instance.mAlbedo, instance.mMetallic);
		vec3 f = fresnel_schlick(max(dot(h, v), 0), f0);

		float ndf = distribution_ggx(n, h, instance.mRoughness);
		float g = geometry_smith(n, v, l, instance.mRoughness);

		vec3 numerator = ndf * g * f;
		float denominator = 4.0 * max(dot(n, v), 0) * max(dot(n, l), 0);
		vec3 specular = numerator / max(denominator, 0.001);

		vec3 ks = f;
		vec3 kd = vec3(1) - ks;
		kd *= (1.0 - instance.mMetallic);

		float n_dot_l = max(dot(n, l), 0);
		lo += (kd * instance.mAlbedo / pi + specular) * radiance * n_dot_l;
	}

	vec3 ambient = vec3(0.03) * instance.mAlbedo * instance.mAO;
	vec3 color = ambient + lo;

	color = color / (color + vec3(1));
	color = pow(color, vec3(1.0 / 2.2));

	outFragColor = vec4(color, 1.0);

	//out_frag_color = vec4((in_normal + vec3(1)) * 0.5, 1);
}

#if 0
void main()
{
	outFragColor = vec4((inNormal.xyz + vec3(1)) * 0.5, 1);
}
#endif
