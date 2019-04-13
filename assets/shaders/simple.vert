#version 130

in vec3 vPosition;
in vec2 vUV;

out vec2 uv;

void main()
{
	uv = vUV;
	gl_Position = vec4(vPosition, 1.0);
}
