in vec2 uv;

out vec4 frag_color;

uniform sampler2D albedo_map;
//uniform sampler2D light_map;

void main()
{
	vec4 albedo_color = texture(albedo_map, uv);
	//frag_color = albedo_color * texture(light_map, uv);
	//frag_color += (albedo_color * vec4(0.2, 0.2, 0.2, 1.0));
	frag_color = albedo_color;
}
