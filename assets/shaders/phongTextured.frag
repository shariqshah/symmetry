//include fog.glsl phongCommon.glsl common.glsl commonFrag.glsl version.glsl 

uniform sampler2D sampler;

void main()
{
	vec4 pixelColor      = diffuseColor * texture(sampler, uv);
	vec4 totalLightColor = calculateLight();
	fragColor            = applyFog(pixelColor * (totalLightColor + ambientLight));
}
