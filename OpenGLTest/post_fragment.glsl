#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D rendered_texture;
uniform sampler2D shadow_texture;
uniform float time;
uniform float gamma;

void main(){
	/*if (gl_FragCoord.x < 200 && gl_FragCoord.y < 200){
		float a = texture(shadow_texture, vec2(gl_FragCoord.x, gl_FragCoord.y)/200.0).r;
		color = vec3(a, a, a);
		return;
	}*/

	vec4 original = texture(rendered_texture, uv);
	vec3 texture_color = pow(original.rgb, vec3(gamma));
	if (gl_FragCoord.w > 0.99)
		color = original.rgb;
	else
		color = texture_color;
}