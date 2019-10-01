uniform mat4 proj_mat;

in vec2 vPosition;
in vec2 vUV;
in vec4 vColor;

out vec2 uv;
out vec4 color;

void main()
{
	uv    = vUV;
	color = vColor;
	gl_Position = proj_mat * vec4(vPosition, 0, 1.0);
}
