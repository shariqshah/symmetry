//include fog.glsl commonFrag.glsl common.glsl version.glsl

uniform sampler2D sampler;

void main()
{
	vec4 pixelColor = diffuseColor * texture(sampler, uv);
	fragColor       = applyFog(pixelColor);
}
