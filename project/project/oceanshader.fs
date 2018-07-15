#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D diffuseTexture;

uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    // settings
    float ambientStrength = 0.5;
    float diffStrength = 0.9;
    float specularStrength = 0.9;
    float shininess = 8;

    vec3 color = texture(diffuseTexture, TexCoords).rgb;

    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射分量
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diffStrength * diff * lightColor;

    // 高光
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * color;
    FragColor = vec4(result, 0.7);
}
