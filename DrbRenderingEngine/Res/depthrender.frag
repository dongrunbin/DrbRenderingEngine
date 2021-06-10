#version 420

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
	OutColor0 = vec4(1.0);
}