#version 420
layout(location=0)in vec3 V_WorldPos;
layout(location=1)in vec3 V_Normal;

layout(binding=2)uniform AliceBuiltinFragmentVectors
{
	vec4 CameraPos;
}U_DefaultFragmentVectors;

layout(binding=5)uniform samplerCube U_Texture0;
layout(location=0)out vec4 OutColor0;
layout(location=1)out vec4 OutColor1;
layout(location=2)out vec4 OutColor2;

void main()
{
	vec3 n = normalize(V_Normal);
	vec3 eye = normalize(V_WorldPos - U_DefaultFragmentVectors.CameraPos.xyz);
	vec3 r = reflect(eye, n);
	OutColor0=vec4(V_WorldPos, 1.0);
	OutColor1=vec4(n, 0.0);
	OutColor2=texture(U_Texture0, r);
}