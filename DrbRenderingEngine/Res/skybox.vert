#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;
layout(location=1)out vec4 V_Texcoord;

layout(binding=1)uniform BuiltinVertexMatrix
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 IT_Model;
}U_DefaultVertexMatrices;

layout(binding=0)uniform BuiltinVertexVectors
{
	vec4 CameraPos;
}U_DefaultVertexVectors;

void main()
{
	V_Texcoord=position;
	vec4 pos = position + U_DefaultVertexVectors.CameraPos;
	pos.w = 1.0;
	gl_Position=(U_DefaultVertexMatrices.Projection * U_DefaultVertexMatrices.View * pos).xyww;
}