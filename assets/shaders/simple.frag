uniform sampler2D sampler;

in  vec2 uv;

out vec4 fragColor;

void main()
{
	// fragColor = vec4(0, 1, 0, 1) * texture2D(sampler, uv.st);
	fragColor = vec4(0, 1, 0, 1);
}
