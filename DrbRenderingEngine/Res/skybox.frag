#version 420
layout(location=1)in vec4 V_Texcoord;

layout(binding=4)uniform samplerCube U_Texture0;
layout(location=0)out vec4 OutColor0;

void main()
{
	OutColor0 = texture(U_Texture0, V_Texcoord.xyz);
}