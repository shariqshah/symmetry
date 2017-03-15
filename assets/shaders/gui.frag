//include version.glsl

in vec2 uv;
in vec4 color;

out vec4 frag_color;

uniform sampler2D sampler;

void main()
{
	frag_color = color * texture(sampler, uv);
	//frag_color = color;
}
