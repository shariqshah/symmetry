//include fog.glsl phongCommon.glsl common.glsl commonFrag.glsl 

void main()
{
	vec4 totalLightColor = calculateLight();
	fragColor            = applyFog(diffuseColor * (totalLightColor + ambientLight));
}
