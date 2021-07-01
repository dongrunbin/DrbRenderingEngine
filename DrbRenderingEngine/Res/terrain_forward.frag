#version 420
layout(location=0)in vec4 V_Texcoord;
layout(location=1)in vec4 V_Normal;
layout(location=2)in vec4 V_WorldPos;

layout(binding=2)uniform AliceBuiltinFragmentVectors
{
	vec4 CameraPos;
}U_DefaultFragmentVectors;

layout(binding=4)uniform sampler2D Grass;
layout(binding=5)uniform sampler2D Terrain;
layout(binding=6)uniform sampler2D Stone;

layout(location=0)out vec4 OutColor0;

void main()
{
	vec4 color=vec4(0.0,0.0,0.0,0.0);
	vec4 ambientColor=vec4(0.4,0.4,0.4,1.0)*vec4(0.1,0.1,0.1,1.0);
	vec3 L=vec3(0.0,50.0,0.0)-V_WorldPos.xyz;
	float distance_from_light=length(L);
	float attenuation=1.0/(1.0+0.02*distance_from_light+0.2*0.2*distance_from_light);
	L=normalize(L);
	vec3 n=normalize(V_Normal.xyz);
	float diffuseIndensity=max(0.0,dot(L,n))*40.0*attenuation;
	vec4 diffuseColor=diffuseIndensity*vec4(1.0,1.0,1.0,1.0)*vec4(0.6,0.6,0.6,1.0);
	vec4 terrainColor=texture(Terrain,V_Texcoord.zw);
	vec4 stoneColor=texture(Stone,vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	color=texture(Grass,vec2(V_Texcoord.x, 1 - V_Texcoord.y))*(ambientColor+diffuseColor)*(1.0-terrainColor.r)+stoneColor*terrainColor.r;
	OutColor0=color;
}