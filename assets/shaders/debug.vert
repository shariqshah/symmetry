//include version.glsl

uniform mat4 mvp;

in vec3 vPosition;

void main()
{
	gl_Position = mvp * vec4(vPosition, 1.0);
}
