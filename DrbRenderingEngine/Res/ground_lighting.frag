#version 420
layout(location=0)in vec4 V_Color;
layout(location=1)in vec3 V_Normal;
layout(location=2)in vec3 V_WorldPos;

struct Light
{
	vec4 pos;
	vec4 color;
	mat4 projection;
	mat4 view;
};

layout(binding=4)uniform Lights
{
	Light light[1];
} U_Lights;

layout(location=0)out vec4 OutColor0;


void main()
{
	vec3 n=normalize(V_Normal);
	vec3 light_pos=U_Lights.light[0].pos.xyz;
	vec3 l=light_pos-V_WorldPos;
	float distance_from_light=length(l);
	float attenuation=1.0/(1.0+distance_from_light*0.5);
	l=normalize(l);
	float diffuse_intensity=max(0.0,dot(l,n));
	vec3 diffuse_color=U_Lights.light[0].color.rgb*attenuation*diffuse_intensity * 10.0;
	OutColor0=V_Color * vec4(diffuse_color, 1.0);
}