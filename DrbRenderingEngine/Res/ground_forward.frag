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
	Light light[2];
} U_Lights;

layout(binding=5)uniform sampler2D U_Texture0;
layout(location=0)out vec4 OutColor0;

const mat4 offset = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0);

float GetShadow(vec4 shadow_coord, vec2 offset)
{
	float ret = 1.0;
	if(shadow_coord.z > -1.0 && shadow_coord.z < 1.0)
	{
		float depth = texture(U_Texture0, shadow_coord.xy + offset).r;
		if(shadow_coord.w > 0.0 && depth < shadow_coord.z - 0.000005)
		{
			ret = 0.1;
		}
	}
	return ret;
}

float CalculateShadow()
{
	vec4 lightSpacePos = offset * U_Lights.light[0].projection * U_Lights.light[0].view * vec4(V_WorldPos, 1.0);
	vec4 perspective_devide_coord = lightSpacePos / lightSpacePos.w;
	ivec2 texsize = textureSize(U_Texture0, 0);
	float dx = 1.0 / texsize.x;
	float dy = 1.0 / texsize.y;

	float shadow = 0.0;
	int count = 0;
	for(int y = -1; y < 1; ++y)
	{
		for(int x = -1; x < 1; ++x)
		{
			shadow += GetShadow(perspective_devide_coord, vec2(dx * x, dy * y));
			++count;
		}
	}
	shadow /= count;
	return shadow;
}

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
	OutColor0=V_Color * vec4(diffuse_color * vec3(CalculateShadow()), 1.0);
}