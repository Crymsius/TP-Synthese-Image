
//! \file tp4_app.cpp TP4

#include "vec.h"
#include "mat.h"

#include "mesh.h"
#include "wavefront.h"
#include "texture.h"

#include "program.h"
#include "uniforms.h"

#include "orbiter.h"
#include "app.h"        // classe Application a deriver

#include "tutos/mesh_data.h"
#include "tutos/mesh_buffer.h"
#include "tutos/material_data.h"


class TP4_App : public App
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP4_App( ) : App(1024, 640) {}
    
    int init( )
    {
        m_framebuffer_width= 1024;
        m_framebuffer_height= 640;
        
        float data[] = {
            -1.f, 1.f, 0.f, 1.f,
            -1.f, -1.f, 0.f, 0.f,
            1.f, -1.f, 1.f, 0.f,
            -1.f, 1.f, 0.f, 1.f,
            1.f, -1.f, 1.f, 0.f,
            1.f, 1.f, 1.f, 1.f
        };
        // cree les buffers et le vao
        glGenVertexArrays(1, &m_screen_vao);
        glBindVertexArray(m_screen_vao);
        
        // buffer : positions + texcoords + normals
        glGenBuffers(1, &m_screen_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_screen_buffer);
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, /* stride */ 4*sizeof(float), (const GLvoid *) 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, /* stride */ 4*sizeof(float), reinterpret_cast<void *>(2*sizeof(float)));
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &m_screen_buffer);
        
        
        // etape 1 : creer une texture couleur...
        glGenTextures(1, &m_color_buffer);
        glBindTexture(GL_TEXTURE_2D, m_color_buffer);
        
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_RGBA, m_framebuffer_width, m_framebuffer_height, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // ... et tous ses mipmaps
        glGenerateMipmap(GL_TEXTURE_2D);
        
        // et son sampler
        glGenSamplers(1, &color_sampler);
        
        glSamplerParameteri(color_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glSamplerParameteri(color_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glSamplerParameteri(color_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
        // etape 1 : creer aussi une texture depth, sinon pas de zbuffer...
        glGenTextures(1, &m_depth_buffer);
        glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
        
        glTexImage2D(GL_TEXTURE_2D, 0,
            GL_DEPTH_COMPONENT, m_framebuffer_width, m_framebuffer_height, 0,
            GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0); //on n'utilise que le niveau 0
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //on ne fait pas l'interpolation (car pas linéaire!)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        
        // etape 2 : creer et configurer un framebuffer object
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_COLOR_ATTACHMENT0, /* texture */ m_color_buffer, /* mipmap level */ 0);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER,  /* attachment */ GL_DEPTH_ATTACHMENT, /* texture */ m_depth_buffer, /* mipmap level */ 0);
        
        // le fragment shader ne declare qu'une seule sortie, indice 0
        GLenum glBuffers[]= { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, glBuffers);
        
        // nettoyage
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        
        // charge un mesh
        MeshData m_data = read_mesh_data("data/sponza/sponza.obj");
//        MeshData m_data = read_mesh_data("data/bigguy.obj");
        
        if (m_data.normals.size() == 0) // calculer les normales, si necessaire
            normals(m_data);
        
        mesh = buffers(m_data);
        Point pmin, pmax;
        bounds(m_data, pmin, pmax);
        m_camera.lookat(pmin, pmax);
        m_framebuffer_camera.lookat(pmin, pmax);

        read_textures(mesh.materials);
        
        // cree les buffers et le vao
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        // buffer : positions + texcoords + normals
        glGenBuffers(1, &m_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_buffer);
        
        size_t size= mesh.vertex_buffer_size() + mesh.texcoord_buffer_size() + mesh.normal_buffer_size();
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
        
        // transfere les positions des sommets
        size_t offset= 0;
        size= mesh.vertex_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.vertex_buffer());
        // configure l'attribut 0, vec3 position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
        glEnableVertexAttribArray(0);

        // transfere les texcoords des sommets
        offset= offset + size;
        size= mesh.texcoord_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.texcoord_buffer());
        // configure l'attribut 1, vec2 texcoord
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
        glEnableVertexAttribArray(1);

        // transfere les normales des sommets
        offset= offset + size;
        size= mesh.normal_buffer_size();
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, mesh.normal_buffer());
        // configure l'attribut 2, vec3 normal
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, /* stride */ 0, (const GLvoid *) offset);
        glEnableVertexAttribArray(2);
        
        // index buffer
        glGenBuffers(1, &index_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.index_buffer_size(), mesh.index_buffer(), GL_STATIC_DRAW);
     
        //
//        mesh.release();
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // shaders
        m_texture_program= read_program("shader/tp4_vert_frag_app.glsl");
        m_depth_program= read_program("shader/tp4_vert_frag_app_depth.glsl");
        m_color_program= read_program("shader/tp4_vert_frag_app_color.glsl");
        m_reflection_program= read_program("shader/tp4_reflect.glsl");
        program_print_errors(m_texture_program);
        program_print_errors(m_depth_program);
        program_print_errors(m_color_program);
        
        // etat openGL par defaut
        glClearColor(1.f, 1.f, 1.f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepthf(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        glDeleteTextures(1, &m_color_buffer);
        glDeleteTextures(1, &m_depth_buffer);
        glDeleteFramebuffers(1, &m_framebuffer);
        glDeleteSamplers(1, &color_sampler);
        
        release_program(m_texture_program);
        glDeleteTextures(1, &m_color_texture);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_buffer);
        return 0;
    }
    
//    int update( const float time, const float delta )
//    {
//        m_model= RotationY(time / 20);
//        return 0;
//    }
    
    // dessiner une nouvelle image
    int render( )
    {
        
        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(-my);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());
        
        /*calcul des transformations*/
        Transform m= m_model;
        Transform v= m_camera.view();
        Transform p= m_camera.projection(window_width(), window_height(), 45);
        Transform mvp= p * v * m;
        Transform mv= v * m;

        {
            /* passe 1 : dessiner dans le framebuffer
             */

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer);
            glViewport(0, 0, m_framebuffer_width, m_framebuffer_height);
            glClearColor(1.f, 1.f, 1.f, 1.f);        // couleur par defaut de la fenetre
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // draw "classique"
            glBindVertexArray(m_vao);
            glUseProgram(m_texture_program);

            program_uniform(m_texture_program, "mvpMatrix", mvp);
            program_uniform(m_texture_program, "mvMatrix", mv);
            program_uniform(m_texture_program, "view", v);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, miplevels(m_framebuffer_width, m_framebuffer_height));
            glGenerateMipmap(GL_TEXTURE_2D);

            // go
            for(int i = 0; i < mesh.material_groups.size(); i++) {
                const MaterialData& material= mesh.materials[mesh.material_groups[i].material];
                program_uniform(m_texture_program, "diffuse_color", material.diffuse);
                program_uniform(m_texture_program, "ns", material.ns);
                program_uniform(m_texture_program, "specular_color", material.specular);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);

                GLint location = glGetUniformLocation(m_texture_program, "diffuse_texture");
                glUniform1i(location, 0);
                glBindSampler(0, color_sampler);
                
                glDrawElements(GL_TRIANGLES,
                               /* count */ mesh.material_groups[i].count,
                               /* index type */ GL_UNSIGNED_INT,
                               /* offset */ mesh.index_buffer_offset(mesh.material_groups[i].first));
            }
//            screenshot("log.png");
            glUseProgram(0);
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, window_width(), window_height());
        }
        if (key_state('c'))
        {
            /* affichage framebuffer : color
             */
            glDisable(GL_DEPTH_TEST);
            glClearColor(1, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glUseProgram(m_color_program);
            
            // utilise la texture attachee au framebuffer
            program_uniform(m_color_program, "color_texture", 0);     // utilise la texture configuree sur l'unite 0
            
            // configure l'unite 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_color_buffer);
            
            glBindVertexArray(m_screen_vao);
            
            //draw le quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else if (key_state('d'))
        {
            /* affichage framebuffer : depth
             */
            glDisable(GL_DEPTH_TEST);
            glClearColor(1, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glUseProgram(m_depth_program);
            
            // utilise la texture attachee au framebuffer
            program_uniform(m_depth_program, "depth_texture", 0);     // utilise la texture configuree sur l'unite 0
            
            // configure l'unite 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
//            glBindTexture(GL_TEXTURE_2D, m_color_buffer);
            
            glBindVertexArray(m_screen_vao);
            
            //draw le quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
            glBindTexture(GL_TEXTURE_2D, 0);
        } else
        {
            /* passe 2 : utiliser la texture du pour reflection
             */
            glClearColor(1, 1, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            glUseProgram(m_reflection_program);
            
            // utilise la texture attachee au framebuffer
            program_uniform(m_reflection_program, "depth_texture", 0);     // utilise la texture configuree sur l'unite 0
            program_uniform(m_reflection_program, "color_texture", 1);
            
            // configure l'unite 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
            glBindTexture(GL_TEXTURE_2D, m_color_buffer);
            
            glBindVertexArray(m_vao);
            
            program_uniform(m_reflection_program, "mvpMatrix", mvp);
            program_uniform(m_reflection_program, "mvMatrix", mv);
            program_uniform(m_reflection_program, "view", v);
            program_uniform(m_reflection_program, "projection", p);
            
            for(int i = 0; i < mesh.material_groups.size(); i++) {
                const MaterialData& material= mesh.materials[mesh.material_groups[i].material];
                program_uniform(m_reflection_program, "diffuse_color", material.diffuse);
                program_uniform(m_reflection_program, "ns", material.ns);
                program_uniform(m_reflection_program, "specular_color", material.specular);
                
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.diffuse_texture);
                
                GLint location = glGetUniformLocation(m_reflection_program, "diffuse_texture");
                glUniform1i(location, 0);
                glBindSampler(0, color_sampler);
                
                glDrawElements(GL_TRIANGLES,
                               /* count */ mesh.material_groups[i].count,
                               /* index type */ GL_UNSIGNED_INT,
                               /* offset */ mesh.index_buffer_offset(mesh.material_groups[i].first));
            }

            
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
            
            
        }
        
        return 1;
    }

protected:
    MeshBuffer mesh;
    Transform m_model;
    Orbiter m_camera;
    Orbiter m_framebuffer_camera;

    GLuint m_vao;
    GLuint m_screen_vao;
    GLuint m_buffer;
    GLuint m_screen_buffer;
    GLuint index_buffer;
    GLuint m_texture_program;
    GLuint m_depth_program;
    GLuint m_color_program;
    GLuint m_reflection_program;
    int m_vertex_count;
    int m_index_count;
    
    GLuint m_color_texture;
    GLuint color_sampler;

    GLuint m_color_buffer;
    GLuint m_depth_buffer;
    GLuint m_framebuffer;
    int m_framebuffer_width;
    int m_framebuffer_height;
};


int main( int argc, char **argv )
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    TP4_App tp;
    tp.run();
    
    return 0;
}
    
