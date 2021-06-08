#version 420
layout(location=0)in vec4 V_Color;
layout(push_constant) uniform AliceBuiltinConstants
{
	vec4 Params[8];
}U_Constants;
layout(binding=4)uniform sampler2D U_Texture0;
layout(location=0)out vec4 OutColor0;
void main(){
	OutColor0=V_Color;
}