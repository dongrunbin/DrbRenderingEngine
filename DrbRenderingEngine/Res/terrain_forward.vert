#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;

layout(location=0)out vec4 V_Texcoord;
layout(location=1)out vec4 V_Normal;
layout(location=2)out vec4 V_WorldPos;

layout(binding=1)uniform AliceBuiltinVertexMatrix
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 IT_Model;
}U_DefaultVertexMatrices;

void main()
{
	V_Normal=U_DefaultVertexMatrices.IT_Model*normal;
	V_WorldPos=U_DefaultVertexMatrices.Model*position;
	V_Texcoord=texcoord;
	gl_Position=U_DefaultVertexMatrices.Projection*U_DefaultVertexMatrices.View*U_DefaultVertexMatrices.Model*position;
}