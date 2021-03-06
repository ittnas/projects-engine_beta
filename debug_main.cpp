#define NO_SDL_GLEXT
#include <SDL/SDL.h>
#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <glm/glm.hpp>

#include <iostream>

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include "model_buffer.hpp"
#include "projection_buffer.hpp"
#include "debug_object.hpp"
#include "tetrahedron.hpp"
#include "viewer.hpp"
#include "key_table.hpp"
//#include "key_action.hpp"
#include "parametric_functions.hpp"
#include "parametric_plot.hpp"
#include "firework_shape.hpp"
#include "texture1D.hpp"
#include "movable.hpp"
#include "world.hpp"

#define SCREEN_W 1920
#define SCREEN_H 1080
#define BITS_PER_PIXEL 32

const GLfloat PI2 = 6.2831853;
const GLfloat PI =  3.1415925;

KeyTable * kt;
SDL_Event event;
const GLdouble aspect_ratio = SCREEN_W / SCREEN_H;
//const GLdouble fps = 60;
//const GLuint frame_length = 1000/60 // in milliseconds

void key_event() {
  
  // Poll whether a key has been pressed.
    GLubyte action = 0;
    if(SDL_PollEvent(&event) && (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)) {
      GLubyte action = 0;
      if(event.type == SDL_KEYDOWN) {
	action = 1;
      } else {
	action = 0;
      }
      kt->set_status(event.key.keysym.sym,action);
      /*
      switch(event.key.keysym.sym) {
	
      case SDLK_LEFT:
	key_table[0] = action;
	//camera_orient[3] += rotation;
	break;
      case SDLK_RIGHT:
	key_table[1] = action;
	//camera_orient[3] -= rotation;
	break;
      case SDLK_UP:
	key_table[2] = action;
	
      //camera_orient[4] += rotation;
	break;
      case SDLK_DOWN:
	key_table[3] = action;
	//camera_orient[4] -= rotation;
	break;
      case SDLK_w:
	key_table[4] = action;
	//move(1);
	break;
      
      case SDLK_s:
	key_table[5] = action;
	//      move(-1);
	break;
      case SDLK_a:
	key_table[6] = action;
	//      camera_orient[5] -= rotation;
	break;
      case SDLK_d:
      key_table[7] = action;
      //      camera_orient[5] += rotation;
      break;
      case SDLK_ESCAPE:
	SDL_Quit();
	return;
       
      }
      */
      
    }
}

void apply_blur(GLfloat q) {
  static GLint i = 0;
  if(i == 0) {
    glAccum(GL_LOAD,q);
  } else {
    glAccum(GL_MULT,q);
    glAccum(GL_ACCUM, 1-q);
    
    glAccum(GL_RETURN,1.0); 
  }
  i = 1;
}

void init_GL() {

  //First init sdl
  
  GLint init_success = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
  assert(~init_success); //0 for success, -1 for failure
  
  
  SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,   16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,  16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);
  
  
  SDL_Surface * screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H,BITS_PER_PIXEL, SDL_OPENGL); //SDL_Surface creates the opengl context, and therefore needs to be called before glewInit();
  
  // And then glew
  
  GLenum err = glewInit();
  
  if( GLEW_OK != err )
    {
      fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(err) );
      assert(0);
    }
    
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

  glClearAccum(0.0, 0.0, 0.0, 0.0);
  glClear(GL_ACCUM_BUFFER_BIT);
  SDL_ShowCursor(0);
  
}

int main() {
  init_GL();
  kt = new KeyTable();
  Object * object = new Object();
  Object * object2 = new Object();

  Camera * fl = new Camera();
  World * world2 = new World();
  //Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  //Camera * mv = new FlyingCamera();
  //Movable * movable = new Movable();
  //ParametricPlot * sphere = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0,PI2),glm::vec2(0,PI),3,3,GL_TRUE,GL_TRUE);

  //movable->set_shape(sphere);

  glClearColor(0.0,0.0,0.0,0.0);
 
  SDL_Event event;
  KeyTable * key_table = new KeyTable();
  GLuint tick = -1;
  GLuint previous_tick = -1;
  GLdouble fps = 0;
  while(true) {
    tick = SDL_GetTicks();
    if(previous_tick != -1 && tick != previous_tick) {
      fps = 1000/(tick - previous_tick);
      //std::cout << "fps: " << fps << std::endl;
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glAccum(GL_RETURN,0.96f);
    //glClear(GL_ACCUM_BUFFER_BIT);

    //World::action_on_worlds(tick);
    //glViewport(0,0,SCREEN_W/2,SCREEN_H);
    glViewport(0,0,SCREEN_W,SCREEN_H); // TODO! Should this really be here in the loop?
    //mv->draw_view(0);
    //text_object->render_text(std::string("jee"),100,100);
    //apply_blur(0.9);
    //glViewport(SCREEN_W/2,0,SCREEN_W/2,SCREEN_H);
    //mv2->draw_view(0);
        
    
    glAccum(GL_MULT,0.97);
    glAccum(GL_ACCUM,0.03);
    glAccum(GL_RETURN,1.0);
    
    SDL_GL_SwapBuffers();
    //glAccum(GL_ACCUM,0.99f);
    //SDL_Delay(10);
    key_event();
    if(key_table->get_status(SDLK_ESCAPE)) {
      SDL_Quit();
      return 0;
    }
    previous_tick = tick;
  }
    return 0;
}
