#version 420

layout(location=0) in vec4 V_TexCoord;

layout(push_constant) uniform AliceBuiltinConstants
{
	vec4 Params[8];
}U_Constants;

layout(binding=4)uniform sampler2D U_Texture0;
layout(location=0)out vec4 OutColor0;
void main()
{
	float depthValue = texture(U_Texture0, vec2(V_TexCoord.x, 1.0 - V_TexCoord.y)).r;
	OutColor0 = vec4(vec3(pow(depthValue, 32.0)),1.0);
}