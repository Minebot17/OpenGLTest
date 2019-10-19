#version 330 core
in vec3 fragmentColor;
in vec2 uv;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
in vec3 eyeDirection_cameraspace;

out vec3 color;
uniform sampler2D textureSampler;
uniform vec3 lightColor;
uniform float lightPower;

void main(){
	vec3 n = normalize(normal_cameraspace);
	vec3 l = normalize(lightDirection_cameraspace);
	vec3 e = normalize(eyeDirection_cameraspace);
	vec3 r = reflect(-l, n);

	float cosAlpha = clamp(dot(e, r), 0, 1);
	float cosTheta = clamp(dot(n, l), 0, 1);
	float dist = clamp(1.0/(length(lightDirection_cameraspace) * length(lightDirection_cameraspace)), 0, 1);

	vec3 materialColor = texture(textureSampler, uv).rgb;
	color = vec3(0.07, 0.07, 0.07) * materialColor + // ambient
			materialColor * lightColor * lightPower * cosTheta * dist + // diffuse
			materialColor * lightColor * lightPower * pow(cosAlpha, 5) * dist; // specular
}