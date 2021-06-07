#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;
layout(location=0)out vec4 V_Color;
layout(location=1)out vec4 V_Texcoord;
layout(binding=0)uniform AliceBuiltinVertexVectors{
	vec4 Value[8];
}U_DefaultVertexVectors;
layout(binding=1)uniform AliceBuiltinVertexMatrix{
	mat4 Model;
	mat4 View;
	mat4 Projection;
	mat4 IT_Model;
}U_DefaultVertexMatrices;
void main(){
	V_Color=normal;
	V_Texcoord=texcoord;
	gl_Position=position;
}