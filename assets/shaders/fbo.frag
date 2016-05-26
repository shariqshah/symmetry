//include version.glsl

in vec2 uv;

out vec4 frag_color;

uniform sampler2D sampler;

void main()
{
	frag_color = texture2D(sampler, uv);
	//frag_color = vec4(1, 0, 0, 1);
}
