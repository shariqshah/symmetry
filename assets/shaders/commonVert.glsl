
// Common inputs and outputs
in vec3 vPosition;
in vec3 vNormal;
in vec2 vUV;

out vec2 uv;
out vec3 normal;
out vec3 vertex;
out vec3 vertCamSpace;
out vec4 vertLightSpace;

// Common uniforms
uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat4 mvp;
uniform mat4 lightVPMat;

vec4 transformPosition(vec3 position)
{
	return mvp * vec4(vPosition, 1.0);
}

void setOutputs()
{
	uv = vUV;
	//Normal and vertex sent to the fragment shader should be in the same space!
	normal = vec4(modelMat * vec4(vNormal, 0.0)).xyz;
	vertex = vec4(modelMat * vec4(vPosition, 1.0)).xyz;
	vertCamSpace   = vec4(viewMat * vec4(vPosition, 1.0)).xyz;
	vertLightSpace = vec4((lightVPMat * modelMat) * vec4(vPosition, 1.0));
}
