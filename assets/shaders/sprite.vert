//include version.glsl

uniform mat4 mvp;

in vec3 vPosition;
in vec2 vUV;
in vec4 vColor;

out vec2 uv;
out vec4 color;

void main()
{
	gl_Position = mvp * vec4(vPosition, 1.0);
	uv = vUV;
	color = vColor;
}
