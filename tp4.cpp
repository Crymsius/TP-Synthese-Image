//! \file tp2.cpp application minimaliste openGL3 core


#include <fstream>
#include <sstream>
#include <string>

#include "mesh.h"
#include "wavefront.h"
#include "texture.h"

#include "program.h"
#include "uniforms.h"

#include "orbiter.h"

#include "glcore.h"
#include "SDL2/SDL.h"
#include "draw.h"
#include "tutos/mesh_data.h"
#include "tutos/mesh_buffer.h"
#include "tutos/material_data.h"

/* une application opengl est composee de plusieurs composants :
    1. une fenetre pour voir ce que l'on dessine
    2. un contexte openGL pour dessiner
    3. 3 fonctions : 
        init( ) pour creer les objets que l'on veut dessiner, 
        draw( ) pour les afficher / dessiner
        quit( ) pour detruire les objets openGL crees dans init( ), a la fermeture de l'application
        
    ces 3 fonctions sont appelees dans le main.
 */


// identifiant du shader program
GLuint program;

// identifiant du vertex array object
GLuint vao;

//buffer
GLuint buffer;
GLuint index_buffer;

MeshBuffer mesh;

GLuint base_texture;
GLuint detail_texture;
GLuint sampler;

GLuint color_buffer;
GLuint depth_buffer;
GLuint framebuffer;
int framebuffer_width;
int framebuffer_height;

int vertex_count;
int index_count;

Transform model = Transform();
Transform view = Transform();
Transform projection = Transform();

Orbiter camera;

// utilitaire : charger un fichier texte et renvoyer une chaine de caracteres.
std::string read( const char *filename )
{
    std::stringbuf source;
    std::ifstream in(filename);
    // verifie que le fichier existe
    if(in.good() == false)
        printf("[error] loading program '%s'...\n", filename);
    else
        printf("loading program '%s'...\n", filename);
    
    // lire le fichier, le caractere '\0' ne peut pas se trouver dans le source de shader
    in.get(source, 0);
    // renvoyer la chaine de caracteres
    return source.str();
}


// creation des objets openGL
bool init_program( )
{
    program = read_program("shader/tp4_vert_frag.glsl" );
   
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) 
    {
        printf("[error] compiling shaders / linking program...\n");
        return false;
    }

    return true;
}

bool init_vao( )
{
    // creer un objet openGL : vertex array object
    glGenVertexArrays(1, &vao);
    
    return true;
}

bool init( )
{
    // init shader program
    if(!init_program())
        return false;

    // init vao
    if(!init_vao())
        return false;
    
    // charge un objet
    MeshData data = read_mesh_data("data/sponza/sponza.obj");
    if (data.normals.size() == 0) // calculer les normales, si necessaire
        normals(data);
    
    mesh = buffers(data);
    vertex_count = mesh.positions.size();
    index_count = mesh.indices.size();
    
    Point pmin, pmax;
    bounds(data, pmin, pmax);
    camera.lookat(pmin, pmax);
    
    
    // options globales du pipeline
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // dessiner dans les images associees a la fenetre
    glDrawBuffer(GL_BACK);      // dessiner dans l'image non affichee de la fenetre
    
    glBindVertexArray(vao);
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    size_t size = mesh.vertex_buffer_size() + mesh.normal_buffer_size() + mesh.texcoord_buffer_size();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);

    // sommets
    size_t offset= 0;
    size = mesh.vertex_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.vertex_buffer());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
    glEnableVertexAttribArray(0);
    
    // normales
    offset= offset + size;
    size= mesh.normal_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, mesh.normal_buffer_size(), mesh.normal_buffer());
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
    glEnableVertexAttribArray(1);
    
    //textures
    offset= offset + size;
    size= mesh.texcoord_buffer_size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.texcoord_buffer());
    // et configure l'attribut 1, vec2 texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
    glEnableVertexAttribArray(2);
    
    // index buffer
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer_size(), mesh.index_buffer(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // etape 3 : sampler, parametres de filtrage des textures
    glGenSamplers(1, &sampler);
    
    glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // etape 4 : creation des textures
    /* utilise les utilitaires de texture.h
     */
//    base_texture= read_texture(0, "data/sponzasmall/KAMEN.JPG");             // texture 'base'
//    detail_texture= read_texture(0, "data/sponzasmall/01_St_kp.JPG");    // texture 'detail'
    read_textures(mesh.materials);
    
    // nettoyage
//    glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
    glUseProgram(0);

    glViewport(0, 0, 1024, 640);        // definir la region (x, y, largeur, hauteur) de la fenetre dans laquelle dessiner
    glClearColor(0.2, 0.2, 0.2, 1);     // definir la couleur par defaut (gris)
    glClearDepthf(1.0f);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);           // pas de test sur la profondeur
    glDisable(GL_CULL_FACE);            // pas de test sur l'orientation
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // dessiner tous les pixels du triangle
    
    return true;
}


// destruction des objets openGL
void quit( )
{
    release_textures(mesh.materials);
    glDeleteBuffers(1, &buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteSamplers(1, &sampler);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(program);
//    glDeleteTextures(1, &base_texture);
//    glDeleteTextures(1, &detail_texture);
}


// affichage
void draw( )
{
    float black[]= { .2f, .2f, .2f, 1.f };
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearBufferfv(GL_COLOR, 0, black);
    
    glBindVertexArray(vao);
    glUseProgram(program);
    
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
    
    // deplace la camera
    if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
        // tourne autour de l'objet
        camera.rotation(mx, my);
    
    else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
        // approche / eloigne l'objet
        camera.move(-my);
    
    else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
        // deplace le point de rotation
        camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());

    Transform mvpMatrix = Perspective(45, 16.f/9.f, 0.1f, 10000) * camera.view() * model;
    program_uniform(program, "model", model);
    program_uniform(program, "mvpMatrix", mvpMatrix);
    program_uniform(program, "cameraPos", camera.position());
    
    // texture et parametres de filtrage de la texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, base_texture);
    glBindSampler(0, sampler);
    
    glActiveTexture(GL_TEXTURE0 +1);
    glBindTexture(GL_TEXTURE_2D, detail_texture);
    glBindSampler(1, sampler);
    
    // uniform sampler2D declares par le fragment shader
    GLint location;
    location= glGetUniformLocation(program, "base_texture");
    glUniform1i(location, 0);
    
    location= glGetUniformLocation(program, "detail_texture");
    glUniform1i(location, 1);
    
    for(int i= 0; i < mesh.material_groups.size(); i++){
        
        const MaterialData& material= mesh.materials[mesh.material_groups[i].material];
        program_uniform(program, "diffuse_color", material.diffuse);
        
        // utilise une texture
        // . selectionne l'unite de texture 0
        glActiveTexture(GL_TEXTURE0);
        // . selectionne la texture
        glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
        // . parametre le shader avec le numero de l'unite sur laquelle est selectionee la texture
        GLint location= glGetUniformLocation(program, "diffuse_texture");
        glUniform1i(location, 0);
        
        // . parametres de filtrage
        glBindSampler(0, sampler);
        
//        program_uniform(program, "mesh_color", Color((i % 100) / 99.f, 1 - (i % 10) / 9.f, (i % 4) / 3.f));
        // afficher chaque x de triangles
        glDrawElements(GL_TRIANGLES, /* count */ mesh.material_groups[i].count, /* index type */ GL_UNSIGNED_INT, /* offset */ mesh.index_buffer_offset(mesh.material_groups[i].first));
    }
}


int main( int argc, char **argv )
{
    // init sdl
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("[error] SDL_Init() failed:\n%s\n", SDL_GetError());
        return 1;       // erreur lors de l'init de sdl2
    }

    // enregistre le destructeur de sdl
    atexit(SDL_Quit);

    // etape 1 : creer la fenetre, utilise sdl
    SDL_Window *window= SDL_CreateWindow("gKit", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 640, SDL_WINDOW_OPENGL);
    if(window == NULL)
    {
        printf("[error] SDL_CreateWindow() failed.\n");
        return 1;       // erreur lors de la creation de la fenetre ou de l'init de sdl2
    }

    // etape 2 : creer un contexte opengl core profile pour dessiner, uitlise sdl
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GLContext context= SDL_GL_CreateContext(window);
    if(context == NULL)
    {
        printf("[error] creating openGL context.\n");
        return 1;
    }

    SDL_GL_SetSwapInterval(1);  // attendre l'ecran

#ifndef NO_GLEW
    // initialise les extensions opengl
    glewExperimental= 1;
    GLenum err= glewInit();
    if(err != GLEW_OK)
    {
        printf("[error] loading extensions\n%s\n", glewGetErrorString(err));
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);        
        return 1;       // erreur lors de l'init de glew / import des fonctions opengl
    }

    // purge les erreurs opengl generees par glew !
    while(glGetError() != GL_NO_ERROR) {;}
#endif
    
    // etape 3 : creation des objets 
    if(!init())
    {
        printf("[error] init failed.\n");
        return 1;
    }
    
    // etape 4 : affichage de l'application, tant que la fenetre n'est pas fermee. 
    bool done= false;
    while(!done)
    {
        // gestion des evenements
        SDL_Event event;
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                done= true;  // sortir si click sur le bouton de la fenetre
            else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                done= true;  // sortir si la touche esc / echapp est enfoncee
        }
        
        // dessiner
        draw();
        
        // presenter / montrer le resultat, echanger les images associees a la fenetre, GL_FRONT et GL_BACK, cf init()
        glFinish(); // execute 1 fois puis bascule
        SDL_GL_SwapWindow(window);
    }

    // etape 5 : nettoyage
    quit();
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    return 0;
}
