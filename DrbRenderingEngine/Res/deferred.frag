#version 420
//#extension GL_EXT_nonuniform_qualifier : enable

layout(location=0)in vec4 V_Texcoord;

//struct Light
//{
//	vec4 pos;
//	vec4 color;
//	mat4 projection;
//	mat4 view;
//};
struct Light
{
	mat4 projection;
	mat4 view;
	vec4 pos;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	vec3 direction;
	float cutoff;//degree
	float spotLightPower;
};

layout(binding=4)uniform Lights
{
	Light light;
} U_Lights;

layout(binding=11)uniform Lights1
{
	Light light;
} U_Lights1;

layout(binding=5)uniform sampler2D U_Texture0; // WorldPos
layout(binding=6)uniform sampler2D U_Texture1; // Normal
layout(binding=7)uniform sampler2D U_Texture2; // Color / Texture
layout(binding=8)uniform sampler2D U_Texture3; // Depth buffer
layout(binding=9)uniform sampler2D U_Texture4; // Depth buffer
layout(binding=10)uniform sampler2D U_Texture5; // Depth buffer of gbuffer

layout(location=0)out vec4 OutColor0;

float calculateShadow(int lightIndex, vec4 worldPos)
{
	vec4 lightSpacePos = vec4(0.0);
	if(lightIndex == 0)
	{
		lightSpacePos = U_Lights.light.projection * U_Lights.light.view * worldPos;
	}
	else
	{
		lightSpacePos = U_Lights1.light.projection * U_Lights1.light.view * worldPos;
	}
	
	vec3 fragPos = lightSpacePos.xyz / lightSpacePos.w;
	fragPos = fragPos * 0.5 + vec3(0.5);
	if(fragPos.x >= 1.0 || fragPos.y >= 1.0 || fragPos.x <= 0.0 || fragPos.y <= 0.0 || fragPos.z >= 1.0 || fragPos.z <= 0.0)
		return 0.0;
	float depthShadowMap = 0.0;
	if(lightIndex == 0)
	{
		depthShadowMap = texture(U_Texture3, fragPos.xy).r;
	}
	else
	{
		depthShadowMap = texture(U_Texture4, fragPos.xy).r;
	}
	
	float currentDepth = fragPos.z;
	vec2 texelSize = 1.0 / textureSize(U_Texture3, 0);
	float shadow = 0.0;
	int count = 0;
	for(int y = -5; y <= 5; ++y)
	{
		for(int x = -5; x <= 5; ++x)
		{
			float pcfDepth = 0.0;
			if(lightIndex == 0)
			{
				pcfDepth = texture(U_Texture3, fragPos.xy + texelSize * vec2(x, y)).r;
			}
			else
			{
				pcfDepth = texture(U_Texture4, fragPos.xy + texelSize * vec2(x, y)).r;
			}
			shadow += currentDepth - 0.02 > pcfDepth ? 1.0 : 0.0;
			++count;
		}
	}
	shadow /= count;
	return shadow;
}

//vec4 CalculateLight(int lightIndex, vec4 worldPos, vec4 normal, vec4 color)
//{
//	vec4 lightSpacePos = offset * U_Lights.light[lightIndex].projection * U_Lights.light[lightIndex].view * worldPos;
//	vec3 n = normal.xyz;
//	vec3 light_pos = U_Lights.light[lightIndex].pos.xyz;
//	vec3 l = light_pos - worldPos.xyz;
//	float distance_from_light = length(l);
//	float attenuation = 1.0 / (1.0 + distance_from_light * 0.5);
//	l = normalize(l);
//	float diffuse_intensity = max(0.0,dot(l,n));
//	vec3 diffuse_color = color.rgb * U_Lights.light[lightIndex].color.rgb * attenuation * diffuse_intensity * 10;
//	vec4 outColor = vec4(diffuse_color * vec3(1.0 - calculateShadow(lightIndex, worldPos)), 1.0);
//	return outColor;
//}

vec4 calculateLight(Light light, int index, vec4 diffuseMaterialColor, vec4 specularMaterialColor, vec4 worldPos, vec3 normal)
{
	//diffuse
	vec3 L = vec3(0.0);
	vec3 N = normalize(normal);
	float distance = 0.0;
	float attenuation = 1.0;
	float constantFactor = 1.0;
	float linearFactor = 0.01;
	float expFactor = 0.01;
	if(light.pos.w == 0.0)
	{
		//direction light
		L = light.pos.xyz;
	}
	else
	{
		//point light or spot light
		L = light.pos.xyz - worldPos.xyz;
		distance = length(L);
		attenuation = 1.0 / (constantFactor + linearFactor * distance + expFactor * distance * distance);
	}
	L = normalize(L);
	float diffuseIntensity = 0.0;
	if(light.spotLightPower != 0.0 && light.cutoff > 0.0)
	{
		//spot light
		float radianCutoff = light.cutoff * 3.1415926 / 180.0;
		float cosTheta = cos(radianCutoff);
		vec3 spotLightDir = normalize(light.direction);
		float currentCosTheta = max(0.0, dot(-L, spotLightDir));
		if(currentCosTheta > cosTheta)
		{
			if(dot(L, N) > 0.0)
			{
				diffuseIntensity = pow(currentCosTheta, light.spotLightPower) * 2.0;
			}
		}
	}
	else
	{
		//point light or direction light
		diffuseIntensity = max(0.0, dot(L, N));
	}
	vec4 diffuseColor = light.diffuseColor * diffuseMaterialColor * diffuseIntensity * attenuation;

	//specular
	float specularIntensity = 0.0;
	if(diffuseIntensity != 0.0)
	{
		//	vec3 ref = normalize(reflect(-L, N));
		//	specularIntensity = pow(max(0.0, dot(ref, viewDir)), 128.0); // phong

		vec3 viewDir = normalize(-worldPos.xyz);
		vec3 halfDir = normalize(L + viewDir);
		specularIntensity = pow(max(0.0, dot(N, halfDir)), 128.0); // blin phong
	}
	vec4 specularColor = light.specularColor * specularMaterialColor * specularIntensity * attenuation;

	vec4 color = (diffuseColor + specularColor) * vec4(vec3(1.0 - calculateShadow(index, worldPos)), 1.0);
	return color;
}

void main()
{
	vec4 worldPos = texture(U_Texture0,vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	vec4 normal = texture(U_Texture1,vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	vec4 color = texture(U_Texture2,vec2(V_Texcoord.x, 1 - V_Texcoord.y));

//	OutColor0 = CalculateLight(0, worldPos, normal, color);
//	OutColor0 += CalculateLight(1, worldPos, normal, color);


	OutColor0 = calculateLight(U_Lights.light, 0, color, color, worldPos, normal.xyz);
	OutColor0 += calculateLight(U_Lights1.light, 1, color, color, worldPos, normal.xyz);
//	OutColor0 = U_Lights1.light.pos;
	gl_FragDepth = texture(U_Texture5, vec2(V_Texcoord.x, 1 - V_Texcoord.y)).r;
}