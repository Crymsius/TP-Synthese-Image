
//! \file tuto1GL_fragment.glsl

#version 330

in vec3 normal;
in vec3 lightPos;
in vec3 FragPos;
in vec3 CameraPosition;
in vec4 Mesh_color;

out vec4 fragment_color;

void main( )
{
    vec3 ambiantColor = vec3(1.0f, 1.0f, 1.0f);
    vec3 lightColor = vec3(0.f, 0.0f, 1.0f);
    vec3 lightColor2 = vec3(1.0f, 0.0f, 0.0f);
    vec3 specularColor = vec3(1.f, 1.f, 1.0f);
//    vec3 objectColor = vec3(0.6f, 0.6f, 0.6f);
    
    vec3 objectColor = Mesh_color.rgb;

    vec3 lightPos = vec3(15.f, 15.f, 0.f);
    vec3 lightPos2 = vec3(-10.f, 0.f, 0.f);
    
    float ambientStrength = 0.9f;
    float specularStrength = 0.4f;

    vec3 ambient = ambientStrength * ambiantColor;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    
    float diff = max(dot(norm, lightDir),0);
    float diff2 = max(dot(norm, lightDir2),0);
    
//    float diff = pow(((dot(norm, lightDir)+1.f)/2.f),2.f);

    //cellshading :
    vec3 camDir = normalize(CameraPosition - FragPos);
    float scalarCam = max(dot(norm,camDir),0.f);
    if (scalarCam < 0.2f) {
        fragment_color = vec4(0.f,0.f,0.f,0.f);
        return;
    }
//    if (diff < 0.5f)
//        diff = 0.3f;
//    else if (diff < 0.8f)
//        diff = 0.5f;
//    else if (diff < 1.f)
//        diff = 0.8f;
//    else
//        diff = 1.f;
    
    vec3 diffuse = (diff * lightColor) + (diff2 * lightColor2);
    
    vec3 viewDir = normalize(CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * specularColor;
    
    
//    vec3 result = (ambient + diffuse) * objectColor;
//    vec3 result = (ambient + specular) * objectColor;
    vec3 result = (ambient + diffuse + specular) * objectColor;
    fragment_color = vec4(result, 1.0);
}
