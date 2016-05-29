//include common.glsl commonFrag.glsl version.glsl

uniform sampler2D diffuse_texture;

out vec4 gbuffer[4];

void main()
{
	// gl_FragData[0] = diffuse_color * texture(diffuse_texture, uv);
	// gl_FragData[1] = vec4(vertex.xyz, 1.0);
	// gl_FragData[2] = vec4(normal.xyz, 1.0);
	// gl_FragData[3] = vec4(uv, 0.0, 1.0);

	gbuffer[0] = diffuse_color * texture(diffuse_texture, uv);
	gbuffer[1] = vec4(vertex.xyz, 1.0);
	gbuffer[2] = vec4(normal.xyz, 1.0);
	gbuffer[3] = vec4(uv, 0.0, 1.0);

	// gbuffer[0] = (diffuse_color * texture(diffuse_texture, uv)).xyz;
	// gbuffer[1] = vertex.xyz;
	// gbuffer[2] = normal.xyz;
	// gbuffer[3] = vec3(uv, 0.0);
}
