#version 330 core
out vec4 fragColor;

uniform sampler2D iChannel0;
uniform vec2 cauldronCoords;
uniform float maxSqm;
uniform float dx;

// pressure poisson
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

    // divergence of the velocity
    float div = (e.x - w.x + n.y - s.y) / (2.0f * dx * dx);

    // one jacobi iteration
    float a = 1.0f / ( dx * dx);
    float p = 1.0f / ( -4.0f * a ) * ( div - a * (n.z + e.z + s.z + w.z));

    fragColor = vec4(o.xy, p, o.w);
}