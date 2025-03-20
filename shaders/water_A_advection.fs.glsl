#version 330 core
out vec4 fragColor;

uniform sampler2D iChannel0;
uniform vec2 iResolution;
uniform vec4 iMouse;
uniform float dt;
uniform vec2 cauldronCoords;
uniform float maxSqm;
uniform float crSq;
uniform float scale;

// advection & boundary & control
void main()
{
    vec2 fragCoord = gl_FragCoord.xy;
    vec2 dist = fragCoord - cauldronCoords;
    float sqm = dot(dist, dist);
    if(sqm > maxSqm) {
        fragColor = vec4(0.f);
        return;
    }

    vec4 o = texelFetch(iChannel0, ivec2(fragCoord), 0);  
    vec4 n = texelFetch(iChannel0, ivec2(fragCoord) + ivec2( 0, 1), 0);
    vec4 e = texelFetch(iChannel0, ivec2(fragCoord) + ivec2( 1, 0), 0);
    vec4 s = texelFetch(iChannel0, ivec2(fragCoord) + ivec2( 0,-1), 0);
    vec4 w = texelFetch(iChannel0, ivec2(fragCoord) + ivec2(-1, 0), 0);

    // advection
    vec4 a = texture(iChannel0, (fragCoord-o.xy*dt)/iResolution.xy);
    fragColor = a;

    // interaction
    if(sign(iMouse.z)==1.0f)
    {
        vec2 d = fragCoord-iMouse.xy;
        float r = length(d);
        vec2 d2 = iMouse.xy-abs(iMouse.zw);
        float r2 = length(d2);
        if(r2>0.0f)
        {
            float m = exp(-r*r*0.01f/scale);
            fragColor.xy = (1.0-m)*fragColor.xy + m*d2/r2;
            fragColor.w += 0.5f*m;
            fragColor.w = min(fragColor.w, .8f);
        }
    }

    // boundary condition (~158px)
    if(sqm < crSq) {
        return;
    }

    float angle = atan(dist.y, dist.x);
    fragColor.w = 0.0f;

    // Right side
    if (abs(angle) < 0.785f) {
        fragColor.xy = -w.xy;
        fragColor.z = w.z;
        return;
    }

    // Left side
    if (abs(angle) > 2.356f) {
        fragColor.xy = -e.xy;
        fragColor.z = e.z;
        return;
    }

    // Top side
    if (0.785f < angle && angle < 2.356f) {
        fragColor.xy = -s.xy;
        fragColor.z = s.z;
        return;
    }

    // Bottom side
    fragColor.xy = -n.xy;
    fragColor.z = n.z;
}