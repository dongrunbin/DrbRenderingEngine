#version 420
#extension GL_EXT_nonuniform_qualifier : enable
layout(location=0)in vec4 V_Color;
layout(location=1)in vec4 V_Texcoord;

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

const mat4 offset = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0);

layout(binding=5)uniform sampler2D U_Texture0; // WorldPos
layout(binding=6)uniform sampler2D U_Texture1; // Normal
layout(binding=7)uniform sampler2D U_Texture2; // Color / Texture
layout(binding=8)uniform sampler2D U_Texture3; // Depth buffer
layout(binding=9)uniform sampler2D U_Texture4; // Depth buffer
layout(binding=10)uniform sampler2D U_Texture5; // Depth buffer

layout(location=0)out vec4 OutColor0;


float GetShadow(vec4 shadow_coord, vec2 offset, int lightIndex)
{
	float ret = 1.0;
	if(shadow_coord.z > -1.0 && shadow_coord.z < 1.0)
	{
		if(lightIndex == 0)
		{
			float depth = texture(U_Texture3, shadow_coord.xy + offset).r;
			if(shadow_coord.w > 0.0 && depth < shadow_coord.z - 0.0005)
			{
				ret = 0.1;
			}
		}
		else
		{
			float depth = texture(U_Texture4, shadow_coord.xy + offset).r;
			if(shadow_coord.w > 0.0 && depth < shadow_coord.z - 0.0005)
			{
				ret = 0.1;
			}
		}

	}
	return ret;
}

float CalculateShadow(vec4 lightSpacePos, int lightIndex)
{
	vec4 perspective_devide_coord = lightSpacePos / lightSpacePos.w;

	ivec2 texsize;
	if(lightIndex == 0)
	{
		texsize = textureSize(U_Texture3, 0);
	}
	else
	{
		texsize = textureSize(U_Texture4, 0);
	}
	float dx = 1.0 / texsize.x;
	float dy = 1.0 / texsize.y;

	float shadow = 0.0;
	int count = 0;
	for(int y = -3; y <= 3; ++y)
	{
		for(int x = -3; x <= 3; ++x)
		{
			shadow += GetShadow(perspective_devide_coord, vec2(dx * x, dy * y), lightIndex);
			++count;
		}
	}
	shadow /= count;
	return shadow;
}

vec4 CalculateLight(int lightIndex, vec4 worldPos, vec4 normal, vec4 color)
{
	vec4 lightSpacePos = offset * U_Lights.light[lightIndex].projection * U_Lights.light[lightIndex].view * worldPos;
	vec3 n = normal.xyz;
	vec3 light_pos = U_Lights.light[lightIndex].pos.xyz;
	vec3 l = light_pos - worldPos.xyz;
	float distance_from_light = length(l);
	float attenuation = 1.0 / (1.0 + distance_from_light * 0.5);
	l = normalize(l);
	float diffuse_intensity = max(0.0,dot(l,n));
	vec3 diffuse_color = color.rgb * U_Lights.light[lightIndex].color.rgb * attenuation * diffuse_intensity * 10;
	vec4 outColor = vec4(diffuse_color * vec3(CalculateShadow(lightSpacePos, lightIndex)), 1.0);
	return outColor;
}

void main()
{
	vec4 worldPos = texture(U_Texture0,vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	vec4 normal = texture(U_Texture1,vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	vec4 color = texture(U_Texture2,vec2(V_Texcoord.x, 1 - V_Texcoord.y));


	OutColor0 = CalculateLight(0, worldPos, normal, color);
	OutColor0 += CalculateLight(1, worldPos, normal, color);
}