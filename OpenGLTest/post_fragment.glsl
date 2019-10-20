#version 330 core

in vec2 uv;

out vec3 color;

uniform sampler2D rendered_texture;
uniform float time;

void main(){
	color = texture(rendered_texture, uv).rgb;
}