
struct Light
{
	vec4  color;
	vec3  direction;
	vec3  position;
	float intensity;
	float outerAngle;
	float innerAngle;
	float falloff;
	int   radius;
	int   type;
	int   castShadow;
	int   pcfEnabled;
	float depthBias;
};

struct Material
{
	float specular;
	float diffuse;
	float specularStrength;
};

#define EPSILON 0.00001

const int MAX_LIGHTS = 32;
const int LT_SPOT    = 0;
const int LT_DIR     = 1;
const int LT_POINT   = 2;

uniform Material material;
uniform int numLights;
uniform Light lightList[MAX_LIGHTS];
uniform Light light;
uniform sampler2DShadow shadowMap0;
uniform sampler2DShadow shadowMap1;
uniform sampler2DShadow shadowMap2;
uniform sampler2DShadow shadowMap3;
uniform vec2            mapSize;
//uniform int             selectedShadowMap;

float calcShadowFactor(vec3 projCoords)
{
	float bias = 0.5;
	vec2 uvCoords;
	uvCoords.x = (projCoords.x * bias) + bias;
	uvCoords.y = (projCoords.y * bias) + bias;
	float z    = (projCoords.z * bias) + bias;
	float visibility = 1.0;
	
	//if uv outside shadowmap range then point out of shadow
	if(uvCoords.x > 1.0 || uvCoords.x < 0.0 || uvCoords.y > 1.0 || uvCoords.y < 0.0)
	{
		return 1.0;
	}
	else
	{
		float dist = distance(eyePos, vertex);
		int selectedShadowMap = 0;
		if(dist <= 30)
			selectedShadowMap = 0;
		else if(dist > 30 && dist <= 80)
			selectedShadowMap = 1;
		else if(dist > 80 && dist <= 120)
			selectedShadowMap = 2;
		else
			selectedShadowMap = 3;
		
		if(light.pcfEnabled == 0)
		{
			float depth = 0;
			if(light.type == LT_DIR)
			{
				switch(selectedShadowMap)
				{
				case 0:	depth = texture(shadowMap0, vec3(uvCoords, z + EPSILON)); break;
				case 1:	depth = texture(shadowMap1, vec3(uvCoords, z + EPSILON)); break;
				case 2:	depth = texture(shadowMap2, vec3(uvCoords, z + EPSILON)); break;
				case 3:	depth = texture(shadowMap3, vec3(uvCoords, z + EPSILON)); break;
				}
			}
			else
			{
				depth = texture(shadowMap0, vec3(uvCoords, z + EPSILON));
			}
			if((depth + light.depthBias) < z)
				visibility = 0.5;
			else
				visibility = 1.0;
		}
		else
		{
			float xOffset = 1.0/mapSize.x;
			float yOffset = 1.0/mapSize.y;
			float Factor = 0.0;

			for (int y = -1 ; y <= 1 ; y++)
			{
				for (int x = -1 ; x <= 1 ; x++)
				{
					vec2 Offsets = vec2(x * xOffset, y * yOffset);
					vec3 UVC = vec3(uvCoords + Offsets, z + EPSILON);
					if(light.type == LT_DIR)
					{
						switch(selectedShadowMap)
						{
						case 0: Factor += texture(shadowMap0, UVC); break;
						case 1: Factor += texture(shadowMap1, UVC); break;
						case 2: Factor += texture(shadowMap2, UVC); break;
						case 3: Factor += texture(shadowMap3, UVC); break;
						}
					}
					else
					{
						Factor += texture(shadowMap0, UVC);
					}
					
				}
			}

			visibility = (0.5 + (Factor / 18.0));
		}
	}
	return visibility;
}

vec4 calcDirLight(Light dirLight)
{
	vec4  diffuse  = vec4(0.0);
	vec4  specular = vec4(0.0);
	vec3  normalizedNormal = normalize(normal);
	float cosAngIncidence  = dot(normalizedNormal, -dirLight.direction);
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	float shadowFactor = 1.0;
	
	if(cosAngIncidence > 0)
	{
		diffuse = dirLight.color * material.diffuse * cosAngIncidence;

		vec3  vertexToEye    = normalize(eyePos - vertex);
		vec3  lightReflect   = normalize(reflect(dirLight.direction, normalizedNormal));
		float specularFactor = max(0.0, dot(vertexToEye, lightReflect));
		specularFactor = pow(specularFactor, material.specularStrength);
		specular = dirLight.color * material.specular * specularFactor;
		if(light.castShadow == 1)
		{
			shadowFactor = calcShadowFactor(vertLightSpace.xyz);
		}
	}
	// return dirLight.intensity * shadowFactor * (diffuse + specular);
	return (dirLight.intensity * (diffuse + specular)) * shadowFactor;
}

vec4 calcPointLight(Light pointLight)
{
	vec4  diffuse  = vec4(0.0);
	vec4  specular = vec4(0.0);
	vec3  lightDirection = vertex - pointLight.position;
	float distance = abs(length(lightDirection));

	if(distance <= pointLight.radius)
	{
		lightDirection = normalize(lightDirection);
		vec3 normalizedNormal = normalize(normal);
		float cosAngIncidence = dot(normalizedNormal, -lightDirection);
		cosAngIncidence = clamp(cosAngIncidence, 0, 1);

		if(cosAngIncidence > 0)
		{
			diffuse = pointLight.color * material.diffuse * cosAngIncidence;
			vec3 vertexToEye = normalize(eyePos - vertex);
			vec3 lightReflect = normalize(reflect(lightDirection, normalizedNormal));
			float specularFactor = max(0.0, dot(vertexToEye, lightReflect));
			specularFactor = pow(specularFactor, material.specularStrength);
			specular = pointLight.color * material.specular * specularFactor;
		}
		float attenuation = pow(max(0.0, (1.0 - (distance / pointLight.radius))), pointLight.falloff + 1.0f);
		return (((diffuse + specular) * attenuation) * pointLight.intensity);
	}
	else
	{
		return vec4(0.0);
	}
}

vec4 calcSpotLight(Light spotLight)
{
	vec4 color = vec4(0.0);
	vec3 lightToSurface = vertex - spotLight.position;
	float angle = dot(spotLight.direction, normalize(lightToSurface));
	if(acos(angle) < spotLight.outerAngle)
	{
		color = calcPointLight(spotLight);
		color *= smoothstep(cos(spotLight.outerAngle), cos(spotLight.innerAngle), angle);
		if(light.castShadow != 0)
		{
			float shadowFactor = calcShadowFactor(vertLightSpace.xyz / vertLightSpace.w);
			color *= shadowFactor;
		}
	}
	return color;// * shadowFactor;
}

vec4 doForwardLightLoop()
{
	vec4 totalLightColor = vec4(0.0);
	for(int i = 0; i < numLights; i++)
	{
		switch(lightList[i].type)
		{
		case LT_DIR:   totalLightColor += calcDirLight(lightList[i]);   break;
		case LT_SPOT:  totalLightColor += calcSpotLight(lightList[i]);  break;
		case LT_POINT: totalLightColor += calcPointLight(lightList[i]); break;
		}
	}
	return totalLightColor;
}

vec4 calculateLight()
{
	vec4 totalLightColor = vec4(0.0);
	switch(light.type)
	{
	case LT_DIR:   totalLightColor = calcDirLight(light);   break;
	case LT_SPOT:  totalLightColor = calcSpotLight(light);  break;
	case LT_POINT: totalLightColor = calcPointLight(light); break;
	}
	return totalLightColor;
}
