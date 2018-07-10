#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = transpose(inverse(mat3(model))) * aNormal;

  	float tex_x = (aPos.x + time/20.0) / 8.0 + 0.5;
  	float tex_y = 0.5 - (aPos.y + time/25.0) / 5.0;
    TexCoords = vec2(tex_x, tex_y);
    //TexCoords = vec2(0.1,0.1);

    gl_Position = projection * view * model * vec4(aPos, 1.0f);
}
