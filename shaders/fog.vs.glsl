#version 330 core
layout (location = 0) in vec3 aPos;

// Simple vertex shader for fog
void main()
{
	gl_Position = vec4(aPos, 1.f);
}