
//! \file tuto4_fragment.glsl

#version 330

#ifdef VERTEX_SHADER

layout(location= 0) in vec3 position;
layout(location= 1) in vec2 texcoord;
layout(location= 2) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;

out vec3 vertex_position;
out vec2 vertex_texcoord;
out vec3 vertex_normal;

void main()
{
    gl_Position= mvpMatrix * vec4(position, 1);
    
    vertex_position= vec3(mvMatrix * vec4(position, 1));
    vertex_texcoord= texcoord;
    vertex_normal= mat3(mvMatrix) * normal;
}

#endif

#ifdef FRAGMENT_SHADER

#define M_PI 3.14159265358979323846

uniform vec4 diffuse_color;
uniform sampler2D diffuse_texture;
uniform float ns;
uniform vec4 specular_color;
uniform mat4 view;

in vec3 vertex_position;
in vec2 vertex_texcoord;
in vec3 vertex_normal;

out vec4 fragment_color;

void main( )
{
    vec3 light_pos = vec3(view * vec4(200.f, 400.f, 0.f, 1.));

    vec3 nn = normalize(vertex_normal);

    vec3 light_dir = normalize(light_pos - vertex_position);
    vec3 view_dir = normalize(- vertex_position);
    vec3 h_dir = normalize(light_dir + view_dir);

    float cos_theta = max(0., dot(light_dir, nn));
    float cos_theta_h = max(0., dot(h_dir, nn));

    vec4 color_texture = texture(diffuse_texture, vertex_texcoord);
    vec3 color_totale = diffuse_color.rgb * color_texture.rgb;

    vec3 spec_color = (ns + 1.)/(2. * M_PI) * pow(cos_theta_h, ns) * cos_theta * specular_color.rgb;

    vec3 diffuse = color_totale * cos_theta;

    vec3 color = diffuse + spec_color;

    fragment_color = vec4(color, 1.);
}


#endif
