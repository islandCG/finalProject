// #version 330 core
// out vec4 FragColor;

// in vec2 TexCoords;

// //uniform Material {
// //	sampler2D texture_diffuse1;
// //} material;
// uniform sampler2D texture_diffuse1;
// //uniform sampler2D texture_normal1;

// void main()
// {    
//    FragColor = texture(texture_diffuse1, TexCoords);
//    //FragColor = mix(texture(texture_diffuse1, TexCoords), texture(texture_normal1, TexCoords), 0.5);
// }
#version 330 core
out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    vec3 specular;    
    float shininess;
}; 

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;
in vec4 FragPosLightSpace;
  
uniform vec3 viewPos;
uniform sampler2D shadowMap;
uniform Material material;
uniform Light light;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // 变换到[0,1]的范围
    projCoords = projCoords * 0.5 + 0.5;
    // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // 取得当前片元在光源视角下的深度
    float currentDepth = projCoords.z;
    // 检查当前片元是否在阴影中
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    // return shadow;
    return 0;
}

void main()
{
    // ambient
    vec3 color = texture(material.diffuse, TexCoords).rgb;
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
        
    vec3 result = ambient + diffuse + specular;
    
    // 计算阴影
    float shadow = ShadowCalculation(FragPosLightSpace);     
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));    

    // FragColor = vec4(result, 1.0);
    FragColor = vec4(lighting, 1.0);
} 