#version 330

uniform sampler2D screen_texture;
uniform float darken_screen_factor;

in vec2 texcoord;
out vec4 color;

void main()
{
    vec4 in_color = texture(screen_texture, texcoord);

    // M1 interpolation implementation

    // If darken factor is 0, keep it fully transparent
    if (darken_screen_factor <= 0.0) {
        color = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        // Fade effect: interpolate between original color and black
        vec4 fade_color = mix(in_color, vec4(0.0, 0.0, 0.0, 1.0), darken_screen_factor);
        fade_color.a = darken_screen_factor; // Gradually increase alpha as it fades
        color = fade_color;
    }
}