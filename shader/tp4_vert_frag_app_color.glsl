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
uniform sampler2D color_texture;
in vec2 texCoord;
out vec4 fragment_color;

void main( )
{
    vec3 col = texture(color_texture, texCoord).rgb;

    fragment_color = vec4(col, 1.0);
}

#endif