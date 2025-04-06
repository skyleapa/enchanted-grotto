#version 330

// SHADER FROM https://www.shadertoy.com/view/7ldGWf

out vec4 fragColor;

// Change these to uniforms to be able to adjust them
const vec3 COLOR = vec3(0.5, 0.0, 0.55);
const vec3 BG = vec3(0.0, 0.0, 0.0);
const vec2 MOTION = vec2(-0.5, 0.3);
const float ZOOM = 3.0;
const int OCTAVES = 4;

uniform vec2 iResolution;
uniform float iTime;
uniform float INTENSITY;


float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9818,79.279)))*43758.5453123);
}

vec2 random2(vec2 st) {
    st = vec2(dot(st,vec2(127.1,311.7)), dot(st,vec2(269.5,183.3)));
    return -1.0 + 2.0 * fract(sin(st) * 7.);
}

float noise(vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // smootstep
    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( dot( random2(i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ),
                     dot( random2(i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( random2(i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ),
                     dot( random2(i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}


float fractal_brownian_motion(vec2 coord) {
	float value = 0.0;
	float scale = 0.2;
	for (int i = 0; i < 4; i++) {
		value += noise(coord) * scale;
		coord *= 2.0;
		scale *= 0.5;
	}
	return value + 0.2;
}

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 st = fragCoord.xy / iResolution.xy;
	st *= iResolution.xy  / iResolution.y;    
    vec2 pos = vec2(st * ZOOM);
	vec2 motion = vec2(fractal_brownian_motion(pos + vec2(iTime * MOTION)));
	float final = fractal_brownian_motion(pos + motion) * INTENSITY;
    fragColor = vec4(mix(BG, COLOR, final), final);
}