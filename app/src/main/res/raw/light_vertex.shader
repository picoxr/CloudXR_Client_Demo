#version 300 es

uniform mat4 u_MVP;
uniform mat4 u_MVMatrix;

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;

out vec4 v_Color;
out vec3 v_Grid;

void main() {
   v_Color = a_Color;
   gl_Position = u_MVP * a_Position;
}