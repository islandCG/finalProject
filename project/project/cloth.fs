#version 430 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

uniform float ambientStrength;
uniform float diffStrength;
uniform float specularStrength;
uniform int shiny;

uniform sampler2D ourTexture;

void main() {
	//环境光
	//float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;
	//漫反射
	vec3 norm = normalize(Normal);
	vec3 lightDirection = normalize(lightPos - FragPos);
	float diff = diffStrength * max(dot(norm, lightDirection), 0.0);
	vec3 diffuse = diff * lightColor;
	//镜面反射
	//float specularStrength = 0.5;
	vec3 viewDirection = normalize(viewPos - FragPos);
	vec3 reflectDirection = reflect(-lightDirection, norm);
	float spec = pow(max(dot(viewDirection, reflectDirection), 0.0), shiny);
	vec3 specular = specularStrength * spec * lightColor;

	vec3 result = (ambient+diffuse+specular) * objectColor;
	FragColor = vec4(result, 1.0f) * texture(ourTexture, TexCoord);
}