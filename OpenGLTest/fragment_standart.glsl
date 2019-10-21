#version 330 core
layout(location = 0) out vec3 color;

in vec3 fragmentColor;
in vec2 uv;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 eyeDirection_cameraspace;
in vec3 lightDirection_tangentspace;
in vec3 eyeDirection_tangentspace;
in mat3 tbn;
in vec4 shadowCoord;

uniform sampler2D textureSampler;
uniform sampler2D normalSampler;
uniform sampler2D specularSampler;
uniform sampler2D shadowSampler;
uniform vec3 lightColor;
uniform float lightPower;
uniform float time;

void main(){
	vec3 n = normalize(texture(normalSampler, uv).rgb*2.0 - 1.0);
	vec3 l = normalize(lightDirection_tangentspace);
	vec3 e = normalize(eyeDirection_tangentspace);
	vec3 r = reflect(-l, n);

	float cosAlpha = clamp(dot(e, r), 0, 1);
	float cosTheta = clamp(dot(n, l), 0, 1);

	float bias = 0.005*tan(acos(cosTheta)); // cosTheta is dot( n,l ), clamped between 0 and 1
	bias = clamp(bias, 0,0.01);
	float visibility = 1.0;
	if (texture(shadowSampler, shadowCoord.xy).z  <  shadowCoord.z){
		visibility = 0.0;
	}

	vec3 materialColor = texture(textureSampler, uv).rgb;
	vec3 specularColor = texture(specularSampler, uv).rgb;
	color = vec3(0.025, 0.025, 0.025) * materialColor + // ambient
			materialColor * lightColor * lightPower * visibility * cosTheta + // diffuse
			specularColor * lightColor * lightPower * visibility * pow(cosAlpha, 5); // specular
}