#version 330 core
layout (location = 0) in vec3 aPos;

uniform vec2 uOffset;
uniform vec2 uScale;

void main()
{
    vec2 p = aPos.xy * uScale + uOffset;
    gl_Position = vec4(p, 0.0, 1.0);
}