//include version.glsl

in vec2 uv;

out vec4 frag_color;

//uniform sampler2D sampler;
uniform sampler2D albedo_map;
uniform sampler2D position_map;
uniform sampler2D normal_map;
uniform sampler2D uv_map;

void main()
{
	//frag_color = texture2D(sampler, uv);
	frag_color = texture(albedo_map, uv);
	frag_color = texture(position_map, uv);
	frag_color = texture(normal_map, uv);
	frag_color = texture(uv_map, uv);
}
