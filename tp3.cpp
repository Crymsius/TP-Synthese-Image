
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
#include "tutos/mesh_data.h"
#include "tutos/mesh_buffer.h"

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
GLuint index_buffer;


MeshBuffer mesh;

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
    //vertex shader
    std::string vertex_source= read("/Users/crymsius/gkit2light/src/shader/tp3GL_vertex.glsl");
    vertex_shader= glCreateShader(GL_VERTEX_SHADER);
    const char *vertex_strings[]= { vertex_source.c_str() };
    glShaderSource(vertex_shader, 1, vertex_strings, NULL);
    glCompileShader(vertex_shader);
    
    //fragment shader
    std::string fragment_source= read("/Users/crymsius/gkit2light/src/shader/tp3GL_fragment.glsl");
    fragment_shader= glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragment_strings[]= { fragment_source.c_str() };
    glShaderSource(fragment_shader, 1, fragment_strings, NULL);
    glCompileShader(fragment_shader);

    program= glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
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
//    Mesh mesh= read_mesh(smart_path("data/bigguy.obj"));
    MeshData data = read_mesh_data("data/sponzasmall/sponza.obj");
//    Mesh mesh = read_mesh(smart_path("data/sponzasmall/sponza.obj"));
    if(data.normals.size() == 0) // calculer les normales, si necessaire
        normals(data);
    
//    vertex_count = mesh.vertex_count();
    mesh = buffers(data);
    vertex_count = mesh.positions.size();
    index_count = mesh.indices.size();
    
    Point pmin, pmax;
//    mesh.bounds(pmin, pmax);
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
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
    glEnableVertexAttribArray(1);
    
    // index buffer
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer_size(), mesh.index_buffer(), GL_STATIC_DRAW);
    
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);


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
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);
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

    Transform mvpMatrix = Perspective(45, 16.f/9.f, 0.1f, 100) * camera.view() * model;
    program_uniform(program, "model", model);
    program_uniform(program, "mvpMatrix", mvpMatrix);
    program_uniform(program, "cameraPos", camera.position());
    
//      glDrawArrays(GL_TRIANGLES, 0, vertex_count);
    for(int i= 0; i < mesh.material_groups.size(); i++){
        program_uniform(program, "mesh_color", Color((i % 100) / 99.f, 1 - (i % 10) / 9.f, (i % 4) / 3.f));
        // afficher chaque x de triangles
        glDrawElements(GL_TRIANGLES, /* count */ mesh.material_groups[i].count, /* index type */ GL_UNSIGNED_INT, /* offset */ mesh.index_buffer_offset(mesh.material_groups[i].first));
    }
    glDrawElements(GL_TRIANGLES, /* count */ mesh.indices.size(), /* index type */ GL_UNSIGNED_INT, /* offset */ 0);
    
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
//
///* affichage d'un mesh par matiere
// */
//std::vector<Group> groups;
//
//Mesh data= read_mesh( ... );
//Mesh mesh(GL_TRIANGLES);
//
//// construction des groupes de triangles
//sort_materials(data, mesh, groups);
//
//// go !
//for(int i= 0; i < groups.size(); i++)
//{
//    // recupere la couleur du groupe
//    program_uniform(m_program, "color", mesh.mesh_material(m_groups[i].material).diffuse);
//    // program_uniform(m_program, "color", Color((i % 100) / 99.f, (i % 10) / 9.f, (i % 4) / 3.f));
//    
//    // affiche les triangles associes a la matiere
//    glDrawArrays(GL_TRIANGLES, m_groups[i].first, m_groups[i].count);
//}
//
//
//// representation d'un groupe de triangles associe a une matiere
//struct Group
//{
//    int material;     //!< indice de la matiere
//    int first;        //!< indice de depart / indice du sommet du premier triangle du groupe
//    int count;        //!< nombre d'indices
//    
//    Point pmin;       //!< englobant du groupe de triangles
//    Point pmax;
//    
//    Group( const int _id= -1, const int _first= 0 ) : material(_id), first(_first), count(0) {}
//};
//
//
//// predicat pour std::sort
//struct compareMaterial
//{
//    const std::vector<unsigned int>& materials;
//    
//    compareMaterial( const std::vector<unsigned int>& _materials ) : materials(_materials) {}
//    
//    bool operator() ( const int &a, const int& b ) const
//    {
//        // compare l'indice des matieres des triangles a et b
//        return materials[a] < materials[b];
//    }
//};
//
//
///*  tri par matiere :
// data : mesh a trier
// mesh : resultat trie
// groups : sequences de triangles utilisant la meme matiere
// */
//void sort_materials( const Mesh& data, Mesh& mesh, std::vector<Group>& groups )
//{
//    std::vector<int> remap(data.triangle_count());
//    for(int i= 0; i < data.triangle_count(); i++)
//        remap[i]= i;
//    
//    // tri les triangles par matiere
//    std::stable_sort(remap.begin(), remap.end(), compareMaterial(data.materials()));
//    
//    // copie les matieres
//    mesh.mesh_materials(data.mesh_materials());
//    
//    // separe les groupes de triangles
//    // . matiere du triangle 0
//    groups.push_back( Group(data.materials().at(remap[0]), 0) );
//    
//    // . copie les sommets des triangles dans l'ordre et construit les groupes
//    // . construit aussi la boite englobante de chaque groupe
//    Point pmin= Point(data.positions().at(3*remap[0]));
//    Point pmax= pmin;
//    
//    for(int i= 0; i < data.triangle_count(); i++)
//    {
//        // indice du triangle
//        int triangle_id= remap[i];
//        
//        // sommets du triangle
//        vec3 a= data.positions().at(3*triangle_id);
//        vec3 b= data.positions().at(3*triangle_id +1);
//        vec3 c= data.positions().at(3*triangle_id +2);
//        
//        // matiere
//        int material= data.materials().at(triangle_id);
//        if(material != groups.back().material)
//        {
//            mesh.material(material);
//            
//            // termine le groupe precedent
//            groups.back().count= 3*i - groups.back().first;
//            groups.back().pmin= pmin;
//            groups.back().pmax= pmax;
//            
//            // demarre un nouveau groupe
//            groups.push_back( Group(material, 3*i) );
//            pmin= Point(a);
//            pmax= pmin;
//        }
//        
//        // attributs
//        if(data.texcoords().size())
//        {
//            mesh.texcoord(data.texcoords().at(3*triangle_id));
//            mesh.texcoord(data.texcoords().at(3*triangle_id +1));
//            mesh.texcoord(data.texcoords().at(3*triangle_id +2));
//        }
//        if(data.normals().size())
//        {
//            mesh.normal(data.normals().at(3*triangle_id));
//            mesh.normal(data.normals().at(3*triangle_id +1));
//            mesh.normal(data.normals().at(3*triangle_id +2));
//        }
//        
//        // englobant
//        pmin= Point( std::min(pmin.x, a.x), std::min(pmin.y, a.y), std::min(pmin.z, a.z) );
//        pmax= Point( std::max(pmax.x, a.x), std::max(pmax.y, a.y), std::max(pmax.z, a.z) );
//        
//        pmin= Point( std::min(pmin.x, b.x), std::min(pmin.y, b.y), std::min(pmin.z, b.z) );
//        pmax= Point( std::max(pmax.x, b.x), std::max(pmax.y, b.y), std::max(pmax.z, b.z) );
//        
//        pmin= Point( std::min(pmin.x, c.x), std::min(pmin.y, c.y), std::min(pmin.z, c.z) );
//        pmax= Point( std::max(pmax.x, c.x), std::max(pmax.y, c.y), std::max(pmax.z, c.z) );
//        
//        // termine la description du sommet
//        mesh.vertex(a);
//        mesh.vertex(b);
//        mesh.vertex(c);
//    }
//    
//    // termine le dernier groupe
//    groups.back().count= 3*mesh.triangle_count() - groups.back().first;
//    groups.back().pmin= pmin;
//    groups.back().pmax= pmax;
//}

