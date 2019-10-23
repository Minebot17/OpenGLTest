#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D rendered_texture;
uniform sampler2D shadow_texture;
uniform float time;
uniform float gamma;

void main(){
	if (gl_FragCoord.x < 200 && gl_FragCoord.y < 200){
		float a = texture(shadow_texture, vec2(gl_FragCoord.x, gl_FragCoord.y)/200.0).r;
		color = vec3(a, a, a);
		return;
	}

	vec3 texture_color = pow(texture(rendered_texture, uv).rgb, vec3(gamma));
	color = texture_color;
}