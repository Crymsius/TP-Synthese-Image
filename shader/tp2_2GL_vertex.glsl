
//! \file tuto1GL_vertex.glsl

#version 330

/*  vec3 et vec4 sont des types de base, ils sont equivalents a :
    
    struct vec3 { float x, y, z; };
    struct vec4 { float x, y, z, w; };
 */
layout (location = 0) in vec3 position;
uniform mat4 mvpMatrix;

void main( )
{
    // intialiser les coordonnees des 3 sommets
//    vec3 positions[10]= vec3[10]( vec3(-0.5, -0.5, -1), vec3(0.5, -0.5, -1), vec3(0, 0.5, -1) );
    
    // recuperer le sommet a traiter
    
    vec4 q = mvpMatrix * vec4(position,1);
    // renvoyer le resultat du vertex shader, positon du sommet dans le repere projectif
    gl_Position = q;
}
