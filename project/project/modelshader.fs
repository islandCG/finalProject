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

// float ShadowCalculation(vec4 fragPosLightSpace)
// {
//     // 执行透视除法
//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
//     // 变换到[0,1]的范围
//     projCoords = projCoords * 0.5 + 0.5;
//     // 取得最近点的深度(使用[0,1]范围下的fragPosLight当坐标)
//     float closestDepth = texture(shadowMap, projCoords.xy).r; 
//     // 取得当前片元在光源视角下的深度
//     float currentDepth = projCoords.z;
//     // 检查当前片元是否在阴影中
//     float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

//     return shadow;
//     //return 0;
// }

float shadowCalculation(vec4 fragPosLightSpace, vec3 lightDir, vec3 normal) {
  // 透视除法 返回片元在光空间-1到1的坐标，正交投影下无意义，透视投影有效
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // 再变换到0到1.。。。
  projCoords = projCoords * 0.5 + 0.5;
  float shadow = 0.0;
  if (projCoords.z < 1.0) {
  		//最近深度
 	float closesDepth = texture(shadowMap, projCoords.xy).r;
 	 // 当前深度
  	float currentDepth = projCoords.z;

  	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

  	float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

  	for(int i = -1; i <= 1; ++i) {
  		for (int j = -1; j <= 1; ++j) {
  			float pcfDepth = texture(shadowMap, projCoords.xy + vec2(i, j) * texelSize).r;
  			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
  		}
  	}
  	shadow /= 9.0;
  	//shadow = currentDepth - bias > closesDepth ? 1.0 : 0.0;
  }
  return shadow;
}

void main()
{
    // ambient
    vec3 color = texture(material.diffuse, TexCoords).rgb;
    vec3 ambient = light.ambient * texture(material.diffuse, TexCoords).rgb;
    vec3 lightColor = vec3(1.0f);
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * texture(material.diffuse, TexCoords).rgb;  
    //vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);  
    //vec3 specular = spec * lightColor;
    
    // 计算阴影
    float shadow = shadowCalculation(FragPosLightSpace, lightDir, norm);     
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    

    // FragColor = vec4(result, 1.0);
    FragColor = vec4(lighting, 1.0);
} 