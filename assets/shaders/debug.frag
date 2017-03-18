//include version.glsl

uniform vec3 debug_color;

out vec4 frag_color;

void main()
{
	frag_color = vec4(debug_color, 1.0);
}
