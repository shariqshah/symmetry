struct Fog
{
	int fogMode;
	float density;
	float start;
	float max;
	vec4 color;
};

uniform Fog fog;

const int FOG_NONE             = 0;
const int FOG_LINEAR           = 1;
const int FOG_EXPONENTIAL      = 2;
const int FOG_EXPONENTIAL_SQRD = 3;

vec4 applyFog(vec4 color)
{
	vec4 finalColor = color;
	if(fog.fogMode != FOG_NONE)
	{
		float fogFactor;
		float distFromEye = abs(length(vertex - eyePos));
		if(fog.fogMode == FOG_LINEAR)
		{
			fogFactor = (fog.max - distFromEye) / (fog.max - fog.start);
		}
		else if(fog.fogMode == FOG_EXPONENTIAL)
		{
			fogFactor = exp(fog.density * -distFromEye);
		}
		else if(fog.fogMode == FOG_EXPONENTIAL_SQRD)
		{
			fogFactor = exp(-pow(fog.density * distFromEye, 2));
		}
		fogFactor = clamp(fogFactor, 0.0, 1.0);
		finalColor = mix(fog.color, color, fogFactor);
	}
	return finalColor;
}

