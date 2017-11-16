
//! \file tuto4_fragment.glsl

#version 330

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 texcoord;

uniform mat4 mvpMatrix;
uniform mat4 model;
uniform vec3 cameraPos;
uniform vec4 mesh_color;

out vec3 normal;
out vec3 lightPos;
out vec3 FragPos;
out vec3 CameraPosition;
out vec4 Mesh_color;
out vec2 vertex_texcoord;

void main( )
{
    gl_Position = mvpMatrix * vec4(position,1);
    normal = anormal;
    
    FragPos = vec3(model * vec4(position, 1.0));
    //    lightPos = vec3(0.f,0.f,10.f);//cameraPos;
    lightPos = cameraPos;
    CameraPosition = cameraPos;
    Mesh_color = mesh_color;
    vertex_texcoord = texcoord;
}

#endif

#ifdef FRAGMENT_SHADER

in vec3 FragPos;
in vec3 normal;
in vec3 lightPos;
in vec3 CameraPosition;
in vec4 Mesh_color;
in vec2 vertex_texcoord;

//uniform sampler2D base_texture;
//uniform sampler2D detail_texture;

uniform vec4 diffuse_color;
uniform sampler2D diffuse_texture;

out vec4 fragment_color;

void main( )
{
    vec3 lightPos = vec3(100.f, 5000.f, 400.f);
    
    vec3 ambiantColor = vec3(1.0f, 0.6f, 0.f);
    vec3 lightColor = vec3(1.f, 1.f, 1.f);
    vec3 specularColor = vec3(1.f, 1.f, 1.f);

    float ambientStrength = 0.2f;
    float specularStrength = 0.1f;

    vec3 ambient = ambientStrength * ambiantColor;
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - FragPos);

    float diff = max(dot(norm, lightDir),0);

    vec3 diffuse = (diff * lightColor);

    vec3 viewDir = normalize(CameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * specularColor;

    
    vec4 color_texture= texture(diffuse_texture, vertex_texcoord);
    vec3 color = diffuse * diffuse_color.rgb;
    
    vec3 result = (ambient + diffuse + specular) * color_texture.rgb;


    fragment_color = vec4(result, 1.0);
}

#endif
