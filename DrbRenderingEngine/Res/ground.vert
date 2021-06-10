#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;

layout(location=0)out vec4 V_Color;
layout(location=1)out vec3 V_Normal;
layout(location=2)out vec3 V_WorldPos;
layout(location=3)out vec4 V_LightSpacePos;

layout(binding=0)uniform AliceBuiltinVertexVectors
{
	vec4 Value[8];
}U_DefaultVertexVectors;

layout(binding=1)uniform AliceBuiltinVertexMatrix
{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 IT_Model;
	mat4 LightProjection;
	mat4 LightView;
}U_DefaultVertexMatrices;

const mat4 offset = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0);

void main()
{
	V_Color=texcoord;
	V_Normal = normalize(vec3(U_DefaultVertexMatrices.IT_Model * normal));
	V_WorldPos = vec3(U_DefaultVertexMatrices.Model * position);
	V_LightSpacePos = offset * U_DefaultVertexMatrices.LightProjection*U_DefaultVertexMatrices.LightView*U_DefaultVertexMatrices.Model*position;
	gl_Position=U_DefaultVertexMatrices.Projection*U_DefaultVertexMatrices.View*U_DefaultVertexMatrices.Model*position;
}