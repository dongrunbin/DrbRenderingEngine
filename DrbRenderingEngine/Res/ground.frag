#version 420
layout(location=0)in vec4 V_Color;
layout(location=1)in vec3 V_Normal;
layout(location=2)in vec3 V_WorldPos;

layout(binding=2)uniform AliceBuiltinFragmentVectors
{
	vec4 LightPos;
	vec4 LightColor;
	vec4 CameraPos;
}U_DefaultFragmentVectors;
layout(binding=3)uniform AliceBuiltinFragmentMatrix
{
	mat4 Value[8];
}U_DefaultFragmentMatrices;

layout(push_constant) uniform AliceBuiltinConstants
{
	vec4 Params[8];
}U_Constants;

layout(binding=4)uniform sampler2D U_Texture0;
layout(location=0)out vec4 OutColor0;
void main()
{
	vec3 n=normalize(V_Normal);
	vec3 light_pos=U_DefaultFragmentVectors.LightPos.xyz;
	vec3 l=light_pos-V_WorldPos;
	float distance_from_light=length(l);
	float attenuation=1.0/(1.0+distance_from_light*0.5);
	l=normalize(l);
	float diffuse_intensity=max(0.0,dot(l,n));
	vec3 diffuse_color=U_DefaultFragmentVectors.LightColor.rgb*attenuation*diffuse_intensity;
	OutColor0=V_Color * vec4(diffuse_color, 1.0);
//	OutColor1=vec4(n,0.0);
//	OutColor2=V_Color;
}