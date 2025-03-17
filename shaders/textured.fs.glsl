#version 330

// From vertex shader
in vec2 texcoord;

// Application data
uniform sampler2D sampler0;
uniform vec3 fcolor;

// add red for enemies flashing
uniform float damage_flash = 0.f; // 0 = normal, 1 = full red flash
uniform bool is_enemy_or_player = false;

// Output color
layout(location = 0) out  vec4 color;

void main()
{
	color = vec4(fcolor, 1.0) * texture(sampler0, vec2(texcoord.x, texcoord.y));
	// only apply the red flash to enemies or players
	if (is_enemy_or_player && damage_flash > 0) {
		float alpha = color.a; // maintain the transparent bits
		color = mix(color, vec4(1.0, 0.0, 0.0, 1.0), damage_flash * 0.5); // blend weaker red tint with original colour
		color.a = alpha;
	}
}
