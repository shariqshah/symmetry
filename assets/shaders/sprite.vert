uniform mat4 mvp;

in vec2 vPosition;
in vec2 vUV;
in vec4 vColor;

out vec2 uv;
out vec4 color;

void main()
{
	gl_Position = mvp * vec4(vPosition, 0.0, 1.0);
	uv = vUV;
	color = vColor;
}
