in vec2 uv;
in vec4 color;

uniform sampler2D sampler;

out vec4 frag_color;

void main()
{
	frag_color = color * texture(sampler, uv);
	//frag_color = vec4(0, 1, 0, 1);
}
