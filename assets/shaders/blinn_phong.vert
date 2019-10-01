//include commonVert.glsl

void main()
{
	gl_Position = transformPosition(vPosition);
	setOutputs();
}
