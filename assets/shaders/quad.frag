in vec2 uv;

out vec4 fragColor;

uniform vec4 textColor;
uniform sampler2D sampler;

void main()
{
	fragColor = texture2D(sampler, uv) * textColor;
}
