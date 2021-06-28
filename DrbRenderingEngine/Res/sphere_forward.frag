#version 420
layout(location=0)in vec3 V_WorldPos;
layout(location=1)in vec3 V_Normal;

layout(binding=2)uniform AliceBuiltinFragmentVectors
{
	vec4 CameraPos;
}U_DefaultFragmentVectors;

struct Light
{
	vec4 pos;
	vec4 color;
	mat4 projection;
	mat4 view;
};

layout(binding=4)uniform Lights
{
	Light light[2];
} U_Lights;

layout(binding=5)uniform samplerCube U_Texture0;
layout(location=0)out vec4 OutColor0;

const mat4 offset = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0);

vec4 CalculateLight(int lightIndex, vec4 worldPos, vec4 normal)
{
	vec4 lightSpacePos = offset * U_Lights.light[lightIndex].projection * U_Lights.light[lightIndex].view * worldPos;
	vec3 n = normal.xyz;
	vec3 light_pos = U_Lights.light[lightIndex].pos.xyz;
	vec3 l = light_pos - worldPos.xyz;
	float distance_from_light = length(l);
	float attenuation = 1.0 / (1.0 + distance_from_light * 0.5);
	l = normalize(l);
	float diffuse_intensity = max(0.0,dot(l,n));
	vec3 diffuse_color = U_Lights.light[lightIndex].color.rgb * attenuation * diffuse_intensity * 10;
	vec4 outColor = vec4(diffuse_color, 1.0);
	return outColor;
}

void main()
{
	vec3 n = normalize(V_Normal);
	
	OutColor0 = CalculateLight(0, vec4(V_WorldPos, 1.0), vec4(n, 1.0));
	OutColor0 += CalculateLight(1, vec4(V_WorldPos, 1.0), vec4(n, 1.0));

	vec3 eye = normalize(V_WorldPos - U_DefaultFragmentVectors.CameraPos.xyz);
	vec3 r = reflect(eye, n);
	OutColor0=vec4(texture(U_Texture0, r).rgb * OutColor0.rgb * 10.0, 1.0);
}