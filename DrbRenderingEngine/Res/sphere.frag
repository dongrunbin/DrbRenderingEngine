#version 420
layout(location=0)in vec3 V_WorldPos;
layout(location=1)in vec3 V_Normal;

layout(binding=2)uniform AliceBuiltinFragmentVectors
{
	vec4 CameraPos;
}U_DefaultFragmentVectors;

layout(binding=4)uniform Light
{
	vec4 pos;
	vec4 color;
	mat4 projection;
	mat4 view;
} U_Lights[];

layout(binding=5)uniform samplerCube U_Texture0;
layout(location=0)out vec4 OutColor0;
layout(location=1)out vec4 OutColor1;
layout(location=2)out vec4 OutColor2;

void main()
{
	//forward render
//	vec3 n = normalize(V_Normal);
//	vec3 light_pos=U_Lights[0].pos.xyz;
//	vec3 l=light_pos-V_WorldPos;
//	float distance_from_light=length(l);
//	float attenuation=1.0/(1.0+distance_from_light*0.5);
//	l=normalize(l);
//	float diffuse_intensity=max(0.0,dot(l,n));
//	vec3 diffuse_color=U_Lights[0].color.rgb*attenuation*diffuse_intensity;
//	vec3 eye = normalize(V_WorldPos - U_DefaultFragmentVectors.CameraPos.xyz);
//	vec3 r = reflect(eye, n);
//	OutColor0=vec4(texture(U_Texture0, r).rgb * diffuse_color, 1.0);

	//deferred render
	vec3 n = normalize(V_Normal);
	vec3 eye = normalize(V_WorldPos - U_DefaultFragmentVectors.CameraPos.xyz);
	vec3 r = reflect(eye, n);
	OutColor0=vec4(V_WorldPos, 1.0);
	OutColor1=vec4(n, 0.0);
	OutColor2=texture(U_Texture0, r);
}