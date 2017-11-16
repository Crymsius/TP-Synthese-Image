//! \file tuto4_fragment_depth.glsl
#version 330
#ifdef VERTEX_SHADER
layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 texCoords;

out vec2 texCoord;

void main(){
    texCoord = texCoords;
    gl_Position = vec4(pos, 0, 1);
}
#endif

#ifdef FRAGMENT_SHADER
uniform sampler2D depth_texture;
in vec2 texCoord;
out vec4 fragment_color;

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main( )
{
    float z = texture(depth_texture, texCoord).r;
    // applique une correction gamma au resultat
    // z = pow(z, 256);
    // fragment_color = vec4(z, z, z, 1);

    float depth = LinearizeDepth(z) / far; // divide by far for demonstration
    fragment_color = vec4(vec3(depth), 1.0);
}

#endif