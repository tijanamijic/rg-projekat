#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D texture1;

void main()
{
    vec4 result = texture(texture1, TexCoords);
    FragColor = vec4(result);
}