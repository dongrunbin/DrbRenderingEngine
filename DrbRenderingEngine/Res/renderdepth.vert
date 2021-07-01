#version 420
layout(location=0)in vec4 position;
layout(location=1)in vec4 texcoord;
layout(location=2)in vec4 normal;

layout(location=0) out vec4 V_TexCoord;

void main()
{
	V_TexCoord = texcoord;
	gl_Position=position;
}