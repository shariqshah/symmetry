struct Fog
{
	int mode;
	float density;
	float start_dist;
	float max_dist;
	vec3 color;
};

uniform Fog fog;

const int FM_NONE             = 0;
const int FM_LINEAR           = 1;
const int FM_EXPONENTIAL      = 2;
const int FM_EXPONENTIAL_SQRD = 3;

vec4 apply_fog(vec4 color)
{
	vec4 final_color = color;
	if(fog.mode != FM_NONE)
	{
		float fog_factor;
		float dist_from_eye = abs(length(vertex - camera_pos));
		if(fog.mode == FM_LINEAR)
		{
			fog_factor = (fog.max_dist - dist_from_eye) / (fog.max_dist - fog.start_dist);
		}
		else if(fog.mode == FM_EXPONENTIAL)
		{
			fog_factor = exp(fog.density * -dist_from_eye);
		}
		else if(fog.mode == FM_EXPONENTIAL_SQRD)
		{
			fog_factor = exp(-pow(fog.density * dist_from_eye, 2));
		}
		fog_factor = clamp(fog_factor, 0.0, 1.0);
		final_color = mix(vec4(fog.color, 1.0), color, fog_factor);
	}
	return final_color;
}

