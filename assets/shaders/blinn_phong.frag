//include fog.glsl common.glsl commonFrag.glsl 

struct Light
{
	vec3  position;
	vec3  direction;
	float outer_angle;
	float inner_angle;
	float falloff;
	float intensity;
	vec3  color;
	uint  pcf_enabled;
	int   type;
	int   radius; 
	float depth_bias;
};

const int LT_SPOT    = 0;
const int LT_DIR     = 1;
const int LT_POINT   = 2;

uniform sampler2D diffuse_texture;
uniform Light lights[MAX_LIGHTS];
uniform int total_active_lights;

uniform float specular;
uniform float diffuse;
uniform float specular_strength;
out vec4 frag_color;

vec3 calc_point_light(in Light light)
{
	vec3  diffuse_comp  = vec3(0.0);
	vec3  specular_comp = vec3(0.0);
	vec3  light_direction = vertex - light.position;
	float dist = abs(length(light_direction));

	if(dist <= light.radius)
	{
		light_direction = normalize(light_direction);
		vec3 normalized_normal = normalize(normal);
		float cos_ang_incidence = dot(normalized_normal, -light_direction);
		cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);

		if(cos_ang_incidence > 0)
		{
			diffuse_comp = light.color * diffuse * cos_ang_incidence;
			vec3 vertex_to_eye = normalize(camera_pos - vertex);
			vec3 halfway = normalize(light.direction + vertex_to_eye);
			float specular_factor = max(0.0, dot(normalized_normal, halfway));
			specular_factor = pow(specular_factor, specular_strength);
			specular_comp = light.color * specular * specular_factor;
		}
		float attenuation = pow(max(0.0, (1.0 - (dist / light.radius))), light.falloff + 1.0f);
		return (((diffuse_comp + specular_comp) * attenuation) * light.intensity);
	}
	else
	{
		return vec3(0.0);
	}
}

vec3 calc_dir_light(in Light light)
{
	vec3  diffuse_comp  = vec3(0.0);
	vec3  specular_comp = vec3(0.0);
	vec3  normalized_normal = normalize(normal);
	float cos_ang_incidence  = dot(normalized_normal, -light.direction);
	cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);
	float shadow_factor = 1.0;
	
	if(cos_ang_incidence > 0)
	{
		diffuse_comp = light.color * diffuse * cos_ang_incidence;

		vec3  vertex_to_eye    = normalize(camera_pos - vertex);
		vec3  light_reflect   = normalize(reflect(light.direction, normalized_normal));
		vec3 halfway = normalize(light.direction + vertex_to_eye);
		float specular_factor = max(0.0, dot(normalized_normal, halfway));
		specular_factor = pow(specular_factor, specular_strength);
		specular_comp = light.color * specular * specular_factor;
		// if(light.castShadow == 1)
		// {
		// 	shadow_factor = calcShadowFactor(vertLightSpace.xyz);
		// }
	}
	//return (light.intensity * (diffuse_comp + specular_comp)) * shadow_factor;
	return (light.intensity * (diffuse_comp + specular_comp));
}

vec3 calc_spot_light(in Light light)
{
	vec3 color = vec3(0.0);
	vec3 light_to_surface = vertex - light.position;
	float angle = dot(light.direction, normalize(light_to_surface));
	if(acos(angle) < light.outer_angle)
	{
		color = calc_point_light(light);
		color *= smoothstep(cos(light.outer_angle), cos(light.inner_angle), angle);
		// if(light.cast_shadow != 0)
		// {
		// 	float shadow_factor = calc_shadow_factor(vert_light_space.xyz / vert_light_space.w);
		// 	color *= shadow_factor;
		// }
	}
	return color;// * shadowFactor;
}


void main()
{
	vec4 albedo_color = diffuse_color * texture(diffuse_texture, vec2(uv.x * uv_scale.x, uv.y * uv_scale.y));
	vec3 light_contribution = vec3(0.0, 0.0, 0.0);
	
	for(int i = 0; i < total_active_lights; i++)
	{
		if(lights[i].type == LT_POINT)
			light_contribution += calc_point_light(lights[i]);
		else if(lights[i].type == LT_DIR)
			light_contribution += calc_dir_light(lights[i]);
		else
			light_contribution += calc_spot_light(lights[i]);
	}
	
	frag_color = apply_fog((albedo_color * vec4(light_contribution + ambient_light, 1.0)));
}
