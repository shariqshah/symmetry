//include version.glsl

in vec2 uv;

out vec4 frag_color;

struct Light
{
	vec3  position;
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

uniform sampler2D albedo_map;
uniform sampler2D position_map;
uniform sampler2D normal_map;
uniform sampler2D depth_map;

uniform Light light;
uniform vec3 camera_pos;
uniform mat4 mvp;
uniform mat4 inv_proj_mat;
uniform vec2 screen_size;
uniform vec2 planes;


vec4 calc_point_light(Light point_light,
					  vec3 vertex,
					  vec3 normal,
					  vec4 albedo_color,
					  float mat_specular_strength,
					  float mat_specular)
{
	vec4  diffuse  = vec4(0.0);
	vec4  specular = vec4(0.0);
	vec3  light_direction = vertex - point_light.position;
	light_direction = normalize(light_direction);
	float distance = abs(length(light_direction));
	vec3 normalized_normal = normalize(normal);
	float cos_ang_incidence = dot(normalized_normal, -light_direction);
	cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);

	if(cos_ang_incidence > 0)
	{
		diffuse = point_light.color * albedo_color * cos_ang_incidence;
		vec3 vertex_to_eye = normalize(camera_pos - vertex);
		vec3 light_reflect = normalize(reflect(light_direction, normalized_normal));
		float specular_factor = max(0.0, dot(vertex_to_eye, light_reflect));
		specular_factor = pow(specular_factor, mat_specular_strength);
		specular = point_light.color * mat_specular * specular_factor;
	}
	float attenuation = pow(max(0.0, (1.0 - (distance / point_light.radius))), point_light.falloff + 1.0f);
	return (((diffuse + specular) * attenuation) * point_light.intensity);
	
	// if(distance <= point_light.radius)
	// {
	// 	light_direction = normalize(light_direction);
	// 	vec3 normalized_normal = normalize(normal);
	// 	float cos_ang_incidence = dot(normalized_normal, -light_direction);
	// 	cos_ang_incidence = clamp(cos_ang_incidence, 0, 1);

	// 	if(cos_ang_incidence > 0)
	// 	{
	// 		diffuse = point_light.color * albedo_color * cos_ang_incidence;
	// 		vec3 vertex_to_eye = normalize(camera_pos - vertex);
	// 		vec3 light_reflect = normalize(reflect(light_direction, normalized_normal));
	// 		float specular_factor = max(0.0, dot(vertex_to_eye, light_reflect));
	// 		specular_factor = pow(specular_factor, mat_specular_strength);
	// 		specular = point_light.color * mat_specular * specular_factor;
	// 	}
	// 	float attenuation = pow(max(0.0, (1.0 - (distance / point_light.radius))), point_light.falloff + 1.0f);
	// 	return (((diffuse + specular) * attenuation) * point_light.intensity);
	// }
	// else
	// {
	// 	return vec4(0.0);
	// }
}

float lz(float depth)
{
	return planes.x / (planes.y - depth * (planes.y - planes.x)) * planes.y;
}

void main()
{
	//vec2 uv_coords = gl_FragCoord.xy / screen_size;
	vec2 uv_coords = uv;
	vec4 albedo_color = texture(albedo_map, uv_coords);
	vec3 position = texture(position_map, uv_coords).rgb;
	vec3 normal = texture(normal_map, uv_coords).rgb;
	normal = normalize(normal);

	// vec3 position = vec3(0);
	// position = vec4(inv_proj_mat * vec4(position, 1.0)).xyz;
	// float depth = texture(depth_map, uv_coords.st).x;
	// position.x = ((gl_FragCoord.x / screen_size.x) - 0.5) * 2.0;
	// position.y = ((-gl_FragCoord.y/screen_size.y)+0.5) * 2.0 / (screen_size.x/screen_size.y);
	// position.z = lz(depth);
	// position.x *= position.z;
	// position.y *= -position.z;

	//normal = vec4(inv_proj_mat * vec4(normal, 1.0)).xyz;
	
	frag_color = calc_point_light(light, position, normal, albedo_color, 20, 1.f);
	//frag_color = vec4(albedo_color, 1.0);
}
