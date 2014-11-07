 
#version 140

out  float water_level;
out  vec3 in_Position;
in   vec3 Vertex;
in   vec3 TexCoord;
in   float water;

uniform mat4 ModelViewProjectionMatrix;

void main(void)
{
	gl_Position = vec4((ModelViewProjectionMatrix*vec4(Vertex,1.0)).xyz,1.0);
	in_Position = TexCoord;
	water_level = water;
}
