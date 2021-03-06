
//! \file tp2.cpp application minimaliste openGL3 core

#include <fstream>
#include <sstream>
#include <string>

#include "glcore.h"
#include "uniforms.h"
#include "SDL2/SDL.h"
#include "orbiter.h"
#include "draw.h"
#include "mesh.h"
#include "wavefront.h"

/* une application opengl est composee de plusieurs composants :
    1. une fenetre pour voir ce que l'on dessine
    2. un contexte openGL pour dessiner
    3. 3 fonctions : 
        init( ) pour creer les objets que l'on veut dessiner, 
        draw( ) pour les afficher / dessiner
        quit( ) pour detruire les objets openGL crees dans init( ), a la fermeture de l'application
        
    ces 3 fonctions sont appelees dans le main.
 */


// identifiants des shaders
GLuint vertex_shader;
GLuint fragment_shader;
// identifiant du shader program
GLuint program;

// identifiant du vertex array object
GLuint vao;

//buffer
GLuint buffer;

int vertex_count;

Transform model = Transform();
Transform view = Transform();
Transform projection = Transform();
vec3 positions[10];


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
    // charger le source du vertex shader
    //PARTIE 1 :
    //std::string vertex_source= read("/Users/crymsius/gkit2light/src/shader/tp2GL_vertex.glsl");
    //PARTIE 2 :
    std::string vertex_source= read("/Users/crymsius/gkit2light/src/shader/tp2_2GL_vertex.glsl");
    // creer un objet openGL : vertex shader
    vertex_shader= glCreateShader(GL_VERTEX_SHADER);
    
    // preparer les chaines de caracteres pour compiler le shader
    const char *vertex_strings[]= { vertex_source.c_str() };
    glShaderSource(vertex_shader, 1, vertex_strings, NULL);
    // compiler les sources
    glCompileShader(vertex_shader);
    
    // pareil pour le fragment shader
    //PARTIE 1 :
    //std::string vertex_source= read("/Users/crymsius/gkit2light/src/shader/tp2GL_fragment.glsl");
    //PARTIE 2 :
    std::string fragment_source= read("/Users/crymsius/gkit2light/src/shader/tp2_2GL_fragment.glsl");
    fragment_shader= glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragment_strings[]= { fragment_source.c_str() };
    glShaderSource(fragment_shader, 1, fragment_strings, NULL);
    glCompileShader(fragment_shader);

    // creer un object openGL : shader program 
    program= glCreateProgram();
    // inclure les 2 shaders dans le program
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    // linker les shaders
    glLinkProgram(program);
    
    // verifier que tout c'est bien passe, si les shaders ne se sont pas compiles correctement, le link du program va echouer.
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) 
    {
        printf("[error] compiling shaders / linking program...\n");
        return false;
    }
    
    // ok, pas d'erreur
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
    
    // charge un objet
    Mesh mesh= read_mesh(smart_path("data/bigguy.obj"));
    vertex_count= mesh.vertex_count();
    
    Point pmin, pmax;
    mesh.bounds(pmin, pmax);
    camera.lookat(pmin, pmax);
    
//    for (int i = -5; i < 5; i++) {
//        for (int j = -5; j < 5; j++) {
//            positions[i] = vec3(i*0.3, j*0.2, -1);
//            std::printf("%f",positions[i].x);
//            std::printf("%f\n",positions[i].y);
//        }
//    }

//    positions[0]= vec3(-1, -1, -1);
//    positions[1]= vec3(-1, -0.5, -1);
//    positions[2]= vec3(-0.5, -0.5, -1);

    // init vao
    if(!init_vao())
        return false;
    
    // options globales du pipeline
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);  // dessiner dans les images associees a la fenetre
    glDrawBuffer(GL_BACK);      // dessiner dans l'image non affichee de la fenetre
    
/* cf l'option SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); dans main() qui demande la creation d'au moins 2 images, GL_FRONT et GL_BACK.
    GL_FRONT est l'image affichee par la fenetre, on peut dessiner dans GL_BACK sans perturber l'affichage. lorsque le dessin est fini, on echange les 2 images...
    
    remarque : si une seule image est associee a la fenetre, 
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0); 
        utiliser :
        glDrawBuffer(GL_FRONT); 
 */
    glBindVertexArray(vao);
    // creer, initialiser le buffer : positions + normals + texcoords du mesh
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    // taille totale du buffer
    glBufferData(GL_ARRAY_BUFFER, mesh.vertex_buffer_size(), mesh.vertex_buffer(), GL_STATIC_DRAW);
    // transfere les positions des sommets
    // et configure l'attribut 0, vec3 position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, 0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glViewport(0, 0, 1024, 640);        // definir la region (x, y, largeur, hauteur) de la fenetre dans laquelle dessiner
    
    glClearColor(0.2, 0.2, 0.2, 1);     // definir la couleur par defaut (gris)
    
    glDisable(GL_DEPTH_TEST);           // pas de test sur la profondeur
    glDisable(GL_CULL_FACE);            // pas de test sur l'orientation
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // dessiner tous les pixels du triangle
    
    return true;
}


// destruction des objets openGL
void quit( )
{
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);
}


// affichage
void draw( )
{
    // effacer la fenetre : copier la couleur par defaut dans tous les pixels de la fenetre
    //~ // cf init(): glClearColor(0.2, 0.2, 0.2, 1); 
    //~ glClear(GL_COLOR_BUFFER_BIT);
    
    float black[]= { .2f, .2f, .2f, 1.f };
    glClearBufferfv(GL_COLOR, 0, black);
    
    // configurer le pipeline, selectionner le vertex array a utiliser
    glBindVertexArray(vao);

    // configurer le pipeline, selectionner le shader program a utiliser
    glUseProgram(program);
    
    
    
    int mx, my;
    unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
    
    // deplace la camera
    if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
        // tourne autour de l'objet
        camera.rotation(mx, my);
    
    else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
        // approche / eloigne l'objet
        camera.move(my);
    
    else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
        // deplace le point de rotation
        camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());

    Transform mvpMatrix = Perspective(45, 16.f/9.f, 0.1f, 100) * camera.view() * model;
    //std::cout << Perspective(45, 1, 0.1f, 10) << std::endl;
    program_uniform(program, "mvpMatrix", mvpMatrix);
    
    // dessiner 1 triangle, soit 3 indices (gl_VertexID varie de 0 a 3)
    glDrawArrays(GL_TRIANGLES, 0, vertex_count);

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
            //else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_j)
                //program_uniform(program, "rotationMatrix", RotationZ(20.f));
            //else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_k)
                //program_uniform(program, "rotationMatrix", RotationZ(-20.f));
        }
        
        // dessiner
        draw();
        
        // presenter / montrer le resultat, echanger les images associees a la fenetre, GL_FRONT et GL_BACK, cf init()
        SDL_GL_SwapWindow(window);
    }

    // etape 5 : nettoyage
    quit();
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    return 0;
}
