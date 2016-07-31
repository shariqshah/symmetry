//include common.glsl commonFrag.glsl version.glsl

struct Light
{
	vec3  position;
	vec3  direction;
	float outer_angle;
	float inner_angle;
	float falloff;
	float intensity;
	vec4  color;
	uint  pcf_enabled;
	int   type;
	int   radius; 
	float depth_bias;
};

const int LT_SPOT    = 0;
const int LT_DIR     = 1;
const int LT_POINT   = 2;

const int MAX_LIGHTS = 128;

uniform vec3 camera_pos;

uniform sampler2D diffuse_texture;
uniform Light lights[MAX_LIGHTS];
uniform int total_active_lights;

uniform float specular;
uniform float diffuse;
uniform float specular_strength;

out vec4 frag_color;

vec4 calc_point_light(in Light point_light)
{
	vec4  diffuse_comp  = vec4(0.0);
	vec4  specular_comp = vec4(0.0);
	vec3  light_direction = vertex - point_light.position;
	float dist = abs(length(light_direction));

	if(dist <= point_light.radius)
	{
		light_direction = normalize(light_direction);
		vec3 normalized_normal = normalize(normal);
		float cos_ang_incidence = dot(normalized_normal, -light_direction);
		cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);

		if(cos_ang_incidence > 0)
		{
			diffuse_comp = point_light.color * diffuse * cos_ang_incidence;
			vec3 vertex_to_eye = normalize(camera_pos - vertex);
			vec3 light_reflect = normalize(reflect(light_direction, normalized_normal));
			float specular_factor = max(0.0, dot(vertex_to_eye, light_reflect));
			specular_factor = pow(specular_factor, specular_strength);
			specular_comp = point_light.color * specular * specular_factor;
		}
		float attenuation = pow(max(0.0, (1.0 - (dist / point_light.radius))), point_light.falloff + 1.0f);
		return (((diffuse_comp + specular_comp) * attenuation) * point_light.intensity);
	}
	else
	{
		return vec4(0.0);
	}
}

vec4 calc_dir_light(in Light dir_light)
{
	vec4  diffuse_comp  = vec4(0.0);
	vec4  specular_comp = vec4(0.0);
	vec3  normalized_normal = normalize(normal);
	float cos_ang_incidence  = dot(normalized_normal, -dir_light.direction);
	cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);
	float shadow_factor = 1.0;
	
	if(cos_ang_incidence > 0)
	{
		diffuse_comp = dir_light.color * diffuse * cos_ang_incidence;

		vec3  vertex_to_eye    = normalize(camera_pos - vertex);
		vec3  light_reflect   = normalize(reflect(dir_light.direction, normalized_normal));
		float specular_factor = max(0.0, dot(vertex_to_eye, light_reflect));
		specular_factor = pow(specular_factor, specular_strength);
		specular_comp = dir_light.color * specular * specular_factor;
		// if(light.castShadow == 1)
		// {
		// 	shadow_factor = calcShadowFactor(vertLightSpace.xyz);
		// }
	}
	//return (dir_light.intensity * (diffuse_comp + specular_comp)) * shadow_factor;
	return (dir_light.intensity * (diffuse_comp + specular_comp));
}


void main()
{
	vec4 albedo_color = diffuse_color * texture(diffuse_texture, uv);
	vec4 light_contribution = vec4(0.0, 0.0, 0.0, 1.0);
	
	for(int i = 0; i < total_active_lights; i++)
	{
		if(i == total_active_lights) break;

		if(lights[i].type == LT_POINT)
			light_contribution += calc_point_light(lights[i]);
		else if(lights[i].type == LT_DIR)
			light_contribution += calc_dir_light(lights[i]);
	}
	
	 frag_color = (albedo_color * vec4(0.1, 0.1, 0.1, 1.0)) +
		          (albedo_color * light_contribution);
}
