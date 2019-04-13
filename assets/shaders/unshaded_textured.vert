//include commonVert.glsl version.glsl

void main()
{
	gl_Position = transformPosition(vPosition);
	setOutputs();
}
