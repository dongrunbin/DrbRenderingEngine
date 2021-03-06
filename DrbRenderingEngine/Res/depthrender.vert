#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;

layout(binding=1)uniform BuiltinVertexMatrix
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 IT_Model;
}U_DefaultVertexMatrices;
void main()
{
	gl_Position = U_DefaultVertexMatrices.Projection * U_DefaultVertexMatrices.View * U_DefaultVertexMatrices.Model * position;
}