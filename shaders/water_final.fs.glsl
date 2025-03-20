#version 330 core
out vec4 fragColor;

uniform vec4 color;
uniform vec2 iResolution;
uniform sampler2D iChannel0;
uniform vec2 cauldronCoords;
uniform float maxSqm;

void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 dist = fragCoord - cauldronCoords;
    float sqm = dot(dist, dist);
    if(sqm > maxSqm) {
        fragColor = vec4(0.f);
        return;
    }

    vec4 c = texture(iChannel0, gl_FragCoord.xy/iResolution.xy);
    //weird green/red effect below
    //fragColor = vec4(abs(c.xy)*4.0f, 0.0, 1.0);
    fragColor = vec4(c.w) * color;
}