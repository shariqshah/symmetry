//include fog.glsl common.glsl commonFrag.glsl version.glsl

uniform sampler2D diffuse_texture;

out vec4 frag_color;

void main()
{
	frag_color = apply_fog(diffuse_color * texture(diffuse_texture, uv));
}
