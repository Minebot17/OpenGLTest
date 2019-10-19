#version 330 core
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTangent;
layout(location = 4) in vec3 vertexBitangent;

uniform mat4 mvp;
uniform mat4 model;
uniform mat4 view;
uniform mat3 mv3x3;
uniform vec3 lightPosition_worldspace;

out vec3 fragmentColor;
out vec2 uv;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace;
out vec3 eyeDirection_cameraspace;
out vec3 lightDirection_tangentspace;
out vec3 eyeDirection_tangentspace;
out mat3 tbn;

void main(){
	uv = vertexUV;
	gl_Position = mvp * vec4(vertexPosition_modelspace, 1);

	// Vector that goes from the vertex to the camera, in camera space.
	vec3 vertexPosition_cameraspace = (view * model * vec4(vertexPosition_modelspace,1)).xyz;
	eyeDirection_cameraspace = -vertexPosition_cameraspace;

	// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
	vec3 lightPosition_cameraspace = (view * vec4(lightPosition_worldspace,1)).xyz;
	lightDirection_cameraspace = lightPosition_cameraspace + eyeDirection_cameraspace;

	// Normal of the the vertex, in camera space
	normal_cameraspace = (view * model * vec4(vertexNormal, 0)).xyz; // Only correct if ModelMatrix does not scale the model ! Use its inverse transpose if not

	vec3 vertexNormal_cameraspace = mv3x3 * normalize(vertexNormal);
    vec3 vertexTangent_cameraspace = mv3x3 * normalize(vertexTangent);
    vec3 vertexBitangent_cameraspace = mv3x3 * normalize(vertexBitangent);

	tbn = transpose(mat3(
        vertexTangent_cameraspace,
        vertexBitangent_cameraspace,
        vertexNormal_cameraspace
    )); // You can use dot products instead of building this matrix and transposing it. See References for details.

	lightDirection_tangentspace = tbn * lightDirection_cameraspace;
    eyeDirection_tangentspace =  tbn * eyeDirection_cameraspace;
}