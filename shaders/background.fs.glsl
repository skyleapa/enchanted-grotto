#version 330

uniform sampler2D screen_texture;
uniform sampler2D background_texture;
uniform float darken_screen_factor;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main()
{
	color = texture(background_texture, texcoord);
}