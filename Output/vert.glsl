 
#version 140
 
out  vec3 in_Position;
 
void main(void)
{
	gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex; //vec4(in_Position*5, 1.0);
	in_Position = gl_Color.rgb;
}
