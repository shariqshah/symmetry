//include version.glsl

uniform vec3 wireframe_color;

out vec4 frag_color;

void main()
{
	frag_color = vec4(wireframe_color, 1.0);
}
