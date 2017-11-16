
//! \file tuto1GL_vertex.glsl

#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 anormal;

uniform mat4 mvpMatrix;
uniform mat4 model;
uniform vec3 cameraPos;
uniform vec4 mesh_color;

out vec3 normal;
out vec3 lightPos;
out vec3 FragPos;
out vec3 CameraPosition;
out vec4 Mesh_color;

void main( )
{
    gl_Position = mvpMatrix * vec4(position,1);
    normal = anormal;
    
    FragPos = vec3(model * vec4(position, 1.0));
//    lightPos = vec3(0.f,0.f,10.f);//cameraPos;
    lightPos = cameraPos;
    CameraPosition = cameraPos;
    Mesh_color = mesh_color;
}
