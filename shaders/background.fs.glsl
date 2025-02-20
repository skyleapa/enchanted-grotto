#version 330

uniform sampler2D background_texture;

in vec2 texcoord;

layout(location = 0) out vec4 color;

void main()
{
	color = texture(background_texture, texcoord);
}