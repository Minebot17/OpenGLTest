#version 330 core
in vec3 fragmentColor;
in vec2 uv;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace;
out vec3 color;
uniform sampler2D textureSampler;
uniform vec3 lightColor;
uniform float lightPower;

void main(){
	// Normal of the computed fragment, in camera space
	vec3 n = normalize(normal_cameraspace);
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(lightDirection_cameraspace);
	float cosTheta = clamp(n.x*l.x + n.y*l.y + n.z*l.z, 0, 1);
	float dist = clamp(1.0/(length(lightDirection_cameraspace) * length(lightDirection_cameraspace)), 0, 1);

	vec3 materialColor = texture(textureSampler, uv).rgb;
	color = materialColor * lightColor * lightPower * cosTheta * dist + vec3(0.05, 0.05, 0.05)*materialColor;
}