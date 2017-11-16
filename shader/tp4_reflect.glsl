//! \file tp4_reflect.glsl
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

uniform sampler2D depth_texture;
uniform vec4 diffuse_color;
uniform sampler2D diffuse_texture;
uniform float ns;
uniform vec4 specular_color;
uniform mat4 view;
uniform mat4 projection;

in vec3 vertex_position;
in vec2 vertex_texcoord;
in vec3 vertex_normal;
in vec2 texCoord;

out vec4 fragment_color;

float near = 0.1;
float far = 100.0;

void main( )
{
    // float z = texture(depth_texture, texCoord).r;

    // float depth = z
    // fragment_color = vec4(vec3(depth), 1.0);
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

    /*REFLECTION*/
    vec3 m = reflect(-view_dir, nn); // glsl convention : camera->point
    vec4 droite = projection * vec4(m,1);
    vec3 droite_norm = droite/droite.w;

    /*
    vec3 a= vec3( ... );
    vec3 b= vec3( ... );
    vec3 d= { ... };
    int n= { ... };

    vev4 color= vec4( ... );
    for(int i= 0; i < n; i++)
    {
        vec3 p= a + i * d;              // p(i)
        vec3 pm= a + (i + 0.5) * d;     // p(i+.5), point intermédiaire / midpoint
        pixel(floor(p.x), floor(p.y))= color;
        pixel(floor(pm.x), floor(pm.y))= color;
    }*/

    

    vec3 color = diffuse + spec_color;

    fragment_color = vec4(color, 1.);
}

#endif