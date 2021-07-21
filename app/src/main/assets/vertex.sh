#version 300 es
uniform mat4 uMVPMatrix;
in vec3 aPosition;
in vec2 aTexCoor;
out vec2 vTextureCoord;
void main()
{
   gl_Position = vec4(aPosition,1);
   vTextureCoord = aTexCoor;
}