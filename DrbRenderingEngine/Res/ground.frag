#version 420
layout(location=0)in vec4 V_Color;
layout(location=1)in vec3 V_Normal;
layout(location=2)in vec3 V_WorldPos;

layout(location=0)out vec4 OutColor0;
layout(location=1)out vec4 OutColor1;
layout(location=2)out vec4 OutColor2;

void main()
{
	vec3 n=normalize(V_Normal);
	OutColor0=vec4(V_WorldPos, 1.0);
	OutColor1=vec4(n, 0.0);
	OutColor2=V_Color;
}