//include common.glsl commonFrag.glsl version.glsl

uniform sampler2D diffuse_texture;

void main()
{
	frag_color = diffuse_color * texture(diffuse_texture, uv);
	//frag_color = vec4(1, 0, 0, 1);
}
