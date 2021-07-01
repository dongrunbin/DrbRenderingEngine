#version 420
layout(location=0)in vec4 V_Texcoord;
layout(location=1)in vec4 V_Normal;
layout(location=2)in vec4 V_WorldPos;

layout(binding=4)uniform sampler2D Grass;
layout(binding=5)uniform sampler2D Terrain;
layout(binding=6)uniform sampler2D Stone;

layout(location=0)out vec4 OutColor0;
layout(location=1)out vec4 OutColor1;
layout(location=2)out vec4 OutColor2;

void main()
{
	vec3 n = normalize(V_Normal.xyz);
	vec4 terrainColor = texture(Terrain, V_Texcoord.zw);
	vec4 stoneColor = texture(Stone, vec2(V_Texcoord.x, 1 - V_Texcoord.y));
	OutColor0 = vec4(V_WorldPos.xyz, 1.0);
	OutColor1 = vec4(n, 0.0);
	OutColor2 = texture(Grass, vec2(V_Texcoord.x, 1 - V_Texcoord.y)) * (1.0 - terrainColor.r) + stoneColor * terrainColor.r;
}