#version 130

in vec2 vPosition;
in vec2 vUV;

out vec2 uv;

uniform mat4 mvp;

void main()
{
	uv = vUV;
	gl_Position = mvp * vec4(vPosition, 0, 1.0);
}