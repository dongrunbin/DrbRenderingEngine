#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;
layout(location=0)out vec4 V_Color;
layout(location=1)out vec4 V_Texcoord;

const mat4 offset = mat4(
		0.5, 0.0, 0.0, 0.0,
		0.0, 0.5, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.5, 0.5, 0.0, 1.0);
void main(){
	V_Color=normal;
	V_Texcoord=texcoord;
	gl_Position=position;
}