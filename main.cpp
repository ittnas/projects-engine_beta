#define NO_SDL_GLEXT
#define GLM_FORCE_RADIANS
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 

#include <iostream>
#include <algorithm>

//#include <ft2build.h>
//#include FT_FREETYPE_H

#include <thread>
#include <mutex>
#include <condition_variable>

#include "model_buffer.hpp"
#include "projection_buffer.hpp"
#include "object.hpp"
#include "world.hpp"
#include "movable.hpp"
#include "light.hpp"
#include "moving_object_test.hpp"
#include "camera.hpp"
#include "flying_camera.hpp"
#include "tetrahedron.hpp"
#include "viewer.hpp"
#include "key_table.hpp"
#include "key_action.hpp"
#include "uniform_material.hpp"
#include "parametric_functions.hpp"
#include "parametric_plot.hpp"
#include "firework_shape.hpp"
#include "firework_shooter.hpp"
#include "text_object.hpp"
#include "fps_text_object.hpp"
#include "texture1D.hpp"
#include "texture2D.hpp"
#include "texture_buffer.hpp"
#include "texture2D_object.hpp"
#include "uniform_list.hpp"
#include "oscillator_action.hpp"
#include "rectangle.hpp"
#include "lightable_text.hpp"
#include "aabb.hpp"
#include "simple_collision_detector.hpp"
#include "cube.hpp"
#include "rigid_body.hpp"
#include "bounding_geometry.hpp"
#include "bounding_geometry_generation_algorithms.hpp"
#include "uniform_color_collision_object.hpp"
#include "parametric_surface_generator.hpp"
#include "collision_hierarchy_node.hpp"
#include "collision_detector.hpp"
#include "cuboid.hpp"
#include "collision_detection_algorithms.hpp"
#include "triangle.hpp"
#include "basic_geometry_shapes.hpp"
#include "mesh_algorithms.hpp"
#include "debug_utility_functions.hpp"
#include "fps_logger.hpp"
#include "indexed_shape.hpp"
#include "tetrahedron.hpp"

SDL_Window * window;
SDL_GLContext glcontext;

const GLuint SCREEN_W = 2560;
const GLuint SCREEN_H = 1440;
#define BITS_PER_PIXEL 32

const GLfloat PI2 = 6.2831853;
const GLfloat PI =  3.1415925;

KeyTable * kt;
SDL_Event event;
const GLfloat aspect_ratio = (GLfloat)SCREEN_W / SCREEN_H;
const GLfloat max_fps = 10000;
//const GLuint frame_length = 1000/max_fps;
const GLfloat simulation_speed = 1.0;
const GLfloat simulation_time_step = 0.0001; // Time in seconds
const GLuint simulation_rounds = 1;
//const GLuint simulation_rounds = std::max(1.0f/(max_fps*simulation_time_step/simulation_speed),1.0f);
const GLfloat frame_length = std::max(simulation_time_step*simulation_rounds/simulation_speed,static_cast<GLfloat>(1.0/max_fps));
GLfloat simulation_time = 0.0;
GLboolean exit_program = GL_FALSE;

// Synchronization
std::condition_variable simulation_ready;
std::condition_variable updating_ready;
//std::mutex lock_drawing;
//std::mutex lock_updating;
std::mutex lock_thread;
GLboolean drawing_notified = GL_FALSE;
GLboolean simulation_notified = GL_TRUE;
GLboolean simulation_performed = GL_FALSE;
GLboolean updating_performed = GL_TRUE;

FPSLogger fps_logger(500);
//const GLdouble fps = 60;
//const GLuint frame_length = 1000/60 // in milliseconds

//#define DEBUG

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
  if(init_success < 0) {
    std::cerr << "Failed to init SDL. SDL_Error: " << SDL_GetError() << std::endl;
    assert(~init_success); //0 for success, -1 for failure
  }
  
  
  SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE,   16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE,  16);
  SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 16);

  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  
  // THIS IS NOW DEPRACATED
  //SDL_Surface * screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H,BITS_PER_PIXEL, SDL_OPENGL); //SDL_Surface creates the opengl context, and therefore needs to be called before glewInit(); 
  
  window = SDL_CreateWindow("Demo",0,0,SCREEN_W,SCREEN_H,SDL_WINDOW_OPENGL| SDL_WINDOW_RESIZABLE);
  glcontext = SDL_GL_CreateContext(window);
  //SDL_GL_SetSwapInterval(1); // Sync with framerate

  // And then glew
  
  GLenum err = glewInit();
  
  if( GLEW_OK != err )
    {
      fprintf(stderr, "Error initializing GLEW: %s\n", glewGetErrorString(err) );
      assert(0);
    }
  
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  //glCullFace(GL_FRONT);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearAccum(0.0, 0.0, 0.0, 0.0);
  glClear(GL_ACCUM_BUFFER_BIT);
  SDL_ShowCursor(0);
}

void simulation_loop() {
  while(!exit_program) {
    //std::cout << "In the simulation loop." << std::endl;
    std::unique_lock<std::mutex> lock(lock_thread);
    //while(!simulation_notified) {
    while(!updating_performed) {
      updating_ready.wait(lock);
    }
      //}
    for(GLuint i = 0; i < simulation_rounds;i++) {
      GLfloat dt = simulation_time_step*simulation_speed;
      World::action_on_worlds(simulation_time*1000,dt);
      simulation_time += dt;
      //std::cout << simulation_time << std::endl;
    }
    updating_performed = GL_FALSE;
    simulation_performed =GL_TRUE;
    simulation_ready.notify_one();
  }
}

void drawing_loop(Camera * mv) {
  GLint success = SDL_GL_MakeCurrent(window,glcontext);
  if(success < 0) {
    std::cerr << "Switcing content not successful. SDL error: " << SDL_GetError() << std::endl;
  }

  SDL_Event event;
  KeyTable * key_table = new KeyTable();
  std::chrono::time_point<std::chrono::system_clock> tick,previous_tick;
  previous_tick = std::chrono::system_clock::now();
  /*
    GLuint tick = -1;
    GLuint previous_tick = -1;
  */
  GLfloat fps = 0;
  const GLuint nbr_frames_in_fps_average = 51;
  GLfloat avg_fps_buffer[nbr_frames_in_fps_average] = {0.0};
  GLuint frame_number = 0; // might overrun
  GLfloat min_fps = std::numeric_limits<GLfloat>::max();
  

  while(GL_TRUE) {
    //std::cout << "In the drawing loop." << std::endl;

    std::unique_lock<std::mutex> lock(lock_thread);
    while(!simulation_performed) {
      simulation_ready.wait(lock);
    }
    
    mv->update_world();
    simulation_performed = GL_FALSE;
    updating_performed = GL_TRUE;
    updating_ready.notify_one();

    //Timing
    //tick = SDL_GetTicks();
    tick = std::chrono::system_clock::now();
    std::chrono::duration<GLfloat> elapsed_time = tick - previous_tick;
#ifdef DEBUG
    std::cout << "Elapsed time: " << elapsed_time.count() << "  " << frame_length << "  " <<  elapsed_time.count() - frame_length <<  "   " << simulation_rounds << std::endl;
#endif
    if(elapsed_time.count() < frame_length) {
      std::this_thread::sleep_for(std::chrono::duration<GLfloat>(frame_length) - elapsed_time);
#ifdef DEBUG
      std::cout << "Frame length: " << frame_length << " Elapsed time: " << elapsed_time.count() << " Slept for: " << (std::chrono::duration<GLfloat>(frame_length) - elapsed_time).count() << std::endl;
#endif
    }
    //if(tick - previous_tick < frame_length) {
      //SDL_Delay(frame_length - tick + previous_tick);
    //}
    //tick = SDL_GetTicks();
    //std::cout << "previous tick: " << previous_tick.count() << " tick: " << tick.count() << " from main.cpp/drawing_loop" << std::endl;
    //std::cout << "Elapsed time: " << elapsed_time.count() << std::endl;
    GLuint current_frame = frame_number % nbr_frames_in_fps_average; // risk of overflow
    fps = 1.0/elapsed_time.count();
    fps_logger.add_fps(fps);
#ifdef DEBUG
    std::cout << "fps from fps_logger: " << fps_logger.get_fps() << std::endl;
#endif
    if(min_fps > fps) {
      min_fps = fps;
    }
    avg_fps_buffer[current_frame] = fps;
    if(current_frame == nbr_frames_in_fps_average - 1) {
      GLfloat avg_fps = 0.0;
      for(GLuint ii = 0; ii < nbr_frames_in_fps_average; ii++) {
	avg_fps += avg_fps_buffer[ii];
      }
      avg_fps /= nbr_frames_in_fps_average;
      std::cout << "fps: " << avg_fps << ", lowest fps: " << min_fps << std::endl;
      min_fps = std::numeric_limits<GLdouble>::max();
    }
    frame_number++;

    /*
    if(previous_tick != -1 && tick != previous_tick) {
      GLuint current_frame = frame_number % nbr_frames_in_fps_average;
      fps = 1000.0/(tick - previous_tick);
      if(min_fps > fps) {
	min_fps = fps;
      }
      //avg_fps_buffer[current_frame] = fps;
      avg_fps_buffer[current_frame] = static_cast<GLfloat>(tick-previous_tick); 
      if(current_frame == nbr_frames_in_fps_average - 1) {
	GLdouble avg_fps = 0.0;
	for(GLuint ii = 0; ii < nbr_frames_in_fps_average; ii++) {
	  avg_fps += avg_fps_buffer[ii];
	}
	avg_fps /= nbr_frames_in_fps_average;
	std::cout << "fps: " << avg_fps << ", lowest fps: " << min_fps << std::endl;
	min_fps = std::numeric_limits<GLdouble>::max();
      }
      frame_number++;
    }
    */
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mv->draw_view(frame_number,TRANSPARENT,0,ANY); // frame_number is given instead of tick. This is not the correct behaviour.

    //SDL_GL_SwapBuffers();
    SDL_GL_SwapWindow(window);
    key_event();
    if(key_table->get_status(SDLK_ESCAPE)) {
      SDL_Quit();
      //return 0;
      exit_program = GL_TRUE;
      return;
    }
    previous_tick = tick;
  }
}

Camera * populate_world_nvidia_test(World *world) {

  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  CollisionDetector * collision_detector = new CollisionDetector();
  collision_detector->set_bounding_geometry_object_program(wire_frame);
  PhysicsManager * pm = new PhysicsManager();
   world->add_simulation_manager(collision_detector);
   world->set_physcis_manager(pm);
   Force * gravity = new ConstantForce(ForceType::GRAVITY,glm::vec3(0,-1,0));
   pm->add_force(gravity);

   //ParametricSphere * co_tetr = new ParametricSphere(glm::vec3(0,0,0),1,11);
   //ParametricPlot * co_tetr = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),51,51,GL_TRUE,GL_TRUE);
   //ParametricPlot * co_tetr = new ParametricPlot(ParametricFunctions::parametric_shell,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),51,51,GL_FALSE,GL_TRUE);
   //ParametricPlot * co_tetr = new ParametricSphere(glm::vec3(0,0,0),6,51);
   Cuboid * co_tetr = new Cuboid(10,10,10);
   //Cube * co_tetr = new Cube(8);

  UniformColorCollisionObject * collision_object2 = new UniformColorCollisionObject();
   collision_object2->set_program(uniprog);
   collision_object2->set_preferred_bounding_volume(BoundingGeometryType::OBB);
   collision_object2->set_shape(co_tetr);

   collision_object2->set_ambient_color(glm::vec4(0.0,1.0,0.0,1.0));
   collision_object2->set_scale(1.0);
   collision_object2->set_moment_of_inertia_and_mass();
   world->add_child(collision_object2);

   UniformColorCollisionObject * collision_object = new UniformColorCollisionObject();
   collision_object->set_program(uniprog);
   collision_object->set_ambient_color(glm::vec4(0.0,0,1.0,1.0));
   collision_object->set_preferred_bounding_volume(BoundingGeometryType::OBB);
   collision_object->set_shape(co_tetr);
   collision_object->set_scale(1.0);

   //collision_object->set_orientation(glm::fquat(std::cos(PI/2),std::sin(PI/2),std::sin(PI/2),std::sin(PI/2)*0.1));
   collision_object->set_moment_of_inertia_and_mass();
  
   std::shared_ptr<MaterialColorCollisionCallbackFunction> callback_function(new MaterialColorCollisionCallbackFunction());
   collision_object->add_collision_callback_function_and_register(callback_function);
   world->add_child(collision_object);


   glm::vec4 position1(-15,4.7,-20,1);
   glm::vec4 position2(15,-4.7,-20,1);

   glm::vec3 impulse11(2000,0,0);
   glm::vec3 impulse11_disp(0,0,0);
   glm::vec3 impulse12(0,0,0);
   glm::vec3 impulse12_disp(0,0,0);

   glm::vec3 impulse21(-2000,0,0);
   glm::vec3 impulse21_disp(0,0,0);

   collision_object->set_position(position1);
   collision_object2->set_position(position2);
   collision_object->apply_impulse(impulse11,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse11_disp);
   collision_object->apply_impulse(impulse12,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse12_disp);
   collision_object2->apply_impulse(impulse21,collision_object2->get_center_of_mass()+glm::vec3(collision_object2->get_position()) + impulse21_disp);

   Light * light2 = new Light();
   Tetrahedron * tetr = new Tetrahedron();
   light2->set_shape(tetr);
   light2->set_position(glm::vec4(0,0,-20,1));
   light2->set_program(prog);
   light2->set_intensity(glm::vec4(1.0,1.0,1.0,1));
   world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_circumsphere(World * world) {
  glm::vec4 vert1 = glm::vec4(0,0,0,1);
  glm::vec4 vert2 = glm::vec4(5,0,0,1);
  glm::vec4 vert3 = glm::vec4(0,std::sqrt(3.0f),0,1);
  glm::vec4 vert4 = glm::vec4(1,1.0f/std::sqrt(3.0f),std::sqrt(3.0f),1);
    MeshAlgorithms::Tetrahedron * tetr = new MeshAlgorithms::Tetrahedron(vert1,vert2,vert3,vert4);
  glm::mat4 vertices = tetr->get_vertices();
  DebugUtilityFunctions::print(vertices);
  MeshAlgorithms::Sphere * circumsphere = MeshAlgorithms::create_circumsphere(*tetr);
  
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  ParametricSphere * sphere  = new ParametricSphere(circumsphere->get_center(),circumsphere->get_radius(),51);
  std::vector<glm::vec4> point_vector;
  point_vector.push_back(vert1);
  point_vector.push_back(vert2);
  point_vector.push_back(vert3);
  point_vector.push_back(vert4);
  PointCloud * tetr_shape = new PointCloud(point_vector);
  UniformMaterial * sphere_object = new UniformMaterial();
  sphere_object->set_shape(sphere);
  sphere_object->set_program(uniprog);
  sphere_object->set_ambient_color(glm::vec4(0,0,1,1));
  world->add_child(sphere_object);

  UniformMaterial * tetrahedron_object = new UniformMaterial();
  tetrahedron_object->set_shape(tetr_shape);
  tetrahedron_object->set_program(uniprog);
  tetrahedron_object->set_ambient_color(glm::vec4(1,0,0,1));
  world->add_child(tetrahedron_object);

  Light * light = new Light();
  light->set_position(glm::vec4(0,0,-5,1));
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light);

  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;

}

Camera * populate_world_distance_to_triangle(World * world) {
  glm::vec3 vert1 = glm::vec3(0,0,0);
  glm::vec3 vert2 = glm::vec3(1,0,0);
  glm::vec3 vert3 = glm::vec3(0,1,0);

  
  Triangle * triangle = new Triangle(glm::vec3(vert1),glm::vec3(vert2),glm::vec3(vert3));
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  std::vector<glm::vec4> point_vector;
  std::vector<glm::vec4> point_on_triangle_vector;
  glm::vec4 target_point = glm::vec4(4,-0.1,0,1);
  point_vector.push_back(target_point);
  glm::vec3 point_on_triangle;
  MeshAlgorithms::distance2_to_triangle(target_point,vert1,vert2,vert3,point_on_triangle);
  point_on_triangle_vector.push_back(glm::vec4(point_on_triangle,1.0));
  PointCloud * point_shape = new PointCloud(point_vector);
  PointCloud * point_on_triangle_shape = new PointCloud(point_on_triangle_vector);
  UniformMaterial * triangle_object = new UniformMaterial();
  triangle_object->set_shape(triangle);
  triangle_object->set_program(uniprog);
  triangle_object->set_ambient_color(glm::vec4(0,0,1,1));
  world->add_child(triangle_object);

  UniformMaterial * point_object = new UniformMaterial();
  point_object->set_shape(point_shape);
  point_object->set_program(uniprog);
  point_object->set_ambient_color(glm::vec4(1,0,0,1));
  world->add_child(point_object);

  UniformMaterial * point_on_triangle_object = new UniformMaterial();
  point_on_triangle_object->set_shape(point_on_triangle_shape);
  point_on_triangle_object->set_program(uniprog);
  point_on_triangle_object->set_ambient_color(glm::vec4(0,1,0,1));
  world->add_child(point_on_triangle_object);

  Light * light = new Light();
  light->set_position(glm::vec4(0,0,-5,1));
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light);

  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;

}

Camera * populate_world_convex_hull(World * world) {
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  std::vector<glm::vec4> point_vector;
  std::vector<glm::vec2> point_vector_2d;
  std::vector<glm::vec4> point_on_triangle_vector;
  std::vector<glm::vec4> point_on_triangle_vector_2d;
  std::vector<GLint> convex_hull_points;
  std::vector<GLint> convex_hull_points_2d;

  for(GLuint ii = 0; ii< 100; ii++) {
    glm::vec4 point = glm::vec4(std::sin(2*PI2*GLfloat(ii)/100.0f+0.1),std::sin(7*PI2*GLfloat(ii)/100.0f+0.25),std::sin(5*PI2*GLfloat(ii)/100.0f+0.5),1.0);
    point_vector.push_back(point);
    point_vector_2d.push_back(glm::vec2(point));
  }
  MeshAlgorithms::quickhull(point_vector,convex_hull_points);
  /*
    GLboolean lol = MeshAlgorithms::left_of_line(glm::vec2(2,0),glm::vec2(0,0),glm::vec2(0,1));
  if(lol) {
  std::cout << "Left of line: " << lol << std::endl;
  } else {
    std::cout << "Right of line: " << lol << std::endl;
  }
  */
  MeshAlgorithms::jarvis_march(point_vector_2d,convex_hull_points_2d);
  /*
  glm::vec3 v1(1,1,5);
  glm::vec3 v2;
  glm::vec3 v3;
  MeshAlgorithms::create_orthogonal_basis(v1,v2,v3);
  DebugUtilityFunctions::print(v1);
  DebugUtilityFunctions::print(v2);
  DebugUtilityFunctions::print(v3);
  */

  point_on_triangle_vector.reserve(convex_hull_points.size());
  for(GLuint ii=0;ii<convex_hull_points.size();ii++) {
    point_on_triangle_vector.push_back(point_vector[convex_hull_points[ii]]);
  }

  point_on_triangle_vector_2d.reserve(convex_hull_points_2d.size());
  for(GLint ii=0; ii<convex_hull_points_2d.size();ii++) {
    point_on_triangle_vector_2d.push_back(point_vector[convex_hull_points_2d[ii]]);
  }
  //glm::vec4 target_point = glm::vec4(4,-0.1,0,1);
  //point_vector.push_back(target_point);
  //glm::vec3 point_on_triangle;
  //MeshAlgorithms::distance2_to_triangle(target_point,vert1,vert2,vert3,point_on_triangle);
  //point_on_triangle_vector.push_back(glm::vec4(point_on_triangle,1.0));
  PointCloud * point_shape = new PointCloud(point_vector);
  PointCloud * point_on_triangle_shape = new PointCloud(point_on_triangle_vector_2d);

  UniformMaterial * point_object = new UniformMaterial();
  point_object->set_shape(point_shape);
  point_object->set_program(uniprog);
  point_object->set_ambient_color(glm::vec4(1,0,0,1));
  //world->add_child(point_object);
  
  
  UniformMaterial * point_on_triangle_object = new UniformMaterial();
  point_on_triangle_object->set_shape(point_on_triangle_shape);
  point_on_triangle_object->set_program(uniprog);
  point_on_triangle_object->set_ambient_color(glm::vec4(0,1,0,1));
  world->add_child(point_on_triangle_object);
  
  Light * light = new Light();
  light->set_position(glm::vec4(0,0,-5,1));
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light);

  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_farthest_point_to_triangle(World * world) {
  glm::vec3 vert1 = glm::vec3(-1,-1,2);
  glm::vec3 vert2 = glm::vec3(2,0,2);
  glm::vec3 vert3 = glm::vec3(0,2,2);

  
  Triangle * triangle = new Triangle(glm::vec3(vert1),glm::vec3(vert2),glm::vec3(vert3));
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  std::vector<glm::vec4> point_vector;

  for(GLuint ii = 0; ii< 100; ii++) {
    glm::vec4 point = glm::vec4(std::sin(2*PI2*GLfloat(ii)/100.0f+0.1),std::sin(7*PI2*GLfloat(ii)/100.0f+0.25),std::sin(5*PI2*GLfloat(ii)/100.0f+0.5),1.0);
      point_vector.push_back(point);
  }
  GLint farthest_point = MeshAlgorithms::farthest_point_to_triangle(point_vector,vert1,vert2,vert3);
  std::vector<glm::vec4> farthest_point_to_triangle_vector;
  farthest_point_to_triangle_vector.push_back(point_vector[farthest_point]);
  point_vector.erase(point_vector.begin() + farthest_point);
  //glm::vec4 target_point = glm::vec4(4,-0.1,0,1);
  //point_vector.push_back(target_point);
  //glm::vec3 point_on_triangle;
  //MeshAlgorithms::distance2_to_triangle(target_point,vert1,vert2,vert3,point_on_triangle);
  //point_on_triangle_vector.push_back(glm::vec4(point_on_triangle,1.0));
  PointCloud * point_shape = new PointCloud(point_vector);
  PointCloud * point_on_triangle_shape = new PointCloud(farthest_point_to_triangle_vector);
  UniformMaterial * triangle_object = new UniformMaterial();
  triangle_object->set_shape(triangle);
  triangle_object->set_program(uniprog);
  triangle_object->set_ambient_color(glm::vec4(0,0,1,1));
  world->add_child(triangle_object);

  UniformMaterial * point_object = new UniformMaterial();
  point_object->set_shape(point_shape);
  point_object->set_program(uniprog);
  point_object->set_ambient_color(glm::vec4(1,0,0,1));
  world->add_child(point_object);

  UniformMaterial * point_on_triangle_object = new UniformMaterial();
  point_on_triangle_object->set_shape(point_on_triangle_shape);
  point_on_triangle_object->set_program(uniprog);
  point_on_triangle_object->set_ambient_color(glm::vec4(0,1,0,1));
  world->add_child(point_on_triangle_object);

  Light * light = new Light();
  light->set_position(glm::vec4(0,0,-5,1));
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light);

  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;

}

Camera * populate_world_triangle_line_intersection(World * world) {
  glm::vec3 v0 = glm::vec3(1,0,0);
  glm::vec3 v1 = glm::vec3(1,1,0);
  glm::vec3 v2 = glm::vec3(0,1,0);

  glm::vec3 p = glm::vec3(1.0f,1.0f,-1.0f);
  glm::vec3 d = glm::vec3(0.0f,0.0f,1.0f);
  GLboolean intersection = CollisionDetectionAlgorithms::ray_triangle_intersection(v0,v1,v2,p,d);
  std::cout << "Intersection happened: ";
  if(intersection) {
    std::cout << "TRUE" << std::endl;
  } else {
    std::cout << "FALSE" << std::endl;
  }

  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  Triangle * triangle_shape = new Triangle(v0,v1,v2);
  RayShape * ray_shape = new RayShape(p,d);

  UniformMaterial * triangle = new UniformMaterial();
  triangle->set_shape(triangle_shape);
  triangle->set_program(uniprog);
  triangle->set_ambient_color(glm::vec4(0,0,1,1));
  //world->add_child(triangle);
  
  //  Shape * target_shape = new ParametricSphere(glm::vec3(0,0,0),5,11);
  //Shape * target_shape = new Tetrahedron(4);
  Shape * target_shape = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),11,11,GL_TRUE,GL_TRUE);

  std::vector<glm::vec4> rnd_points;
  std::vector<glm::vec4> all_points;
  std::vector<glm::vec4> target_points;
  std::vector<GLboolean> point_succesful;

  target_shape->get_mesh()->get_random_points_inside_mesh(1000,0,rnd_points,7,GL_FALSE);
  
  /*
  for(GLuint ii=0; ii<all_points.size();ii++) {
    RayShape * ray_shape_l = new RayShape(glm::vec3(all_points[ii]),glm::vec3(target_points[ii]-all_points[ii]));
    UniformMaterial * ray = new UniformMaterial();
    ray->set_shape(ray_shape_l);
    ray->set_program(uniprog);
    if(point_succesful[ii]) {
      ray->set_ambient_color(glm::vec4(0,1,0,1));
    } else {
      ray->set_ambient_color(glm::vec4(0,0,1,1));
    }
    world->add_child(ray);
  }
  */
  /*
  UniformMaterial * ray = new UniformMaterial();
  ray->set_shape(ray_shape);
  ray->set_program(uniprog);
  ray->set_ambient_color(glm::vec4(0,1,0,1));
  world->add_child(ray);
  */
  PointCloud * point_cloud_shape = new PointCloud(rnd_points);
  //PointCloud * point_cloud_shape = new PointCloud(all_points);
  UniformMaterial * point_cloud = new UniformMaterial();
  point_cloud->set_shape(point_cloud_shape);
  point_cloud->set_program(uniprog);
  point_cloud->set_ambient_color(glm::vec4(1,0,0,1));
  world->add_child(point_cloud);
  UniformMaterial * cuboid = new UniformMaterial();
  cuboid->set_shape(target_shape);
  cuboid->set_program(wire_frame);
  cuboid->set_ambient_color(glm::vec4(0.1,0,0,1));
  world->add_child(cuboid);
  Light * light = new Light();
  light->set_position(glm::vec4(0,0,-5,1));
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_junk(World *world) {
  /*GLubyte tex_data[4*4] = {1,0,0,1,
			 1,0,0,1,
			 1,0,0,1,
			 1,0,0,1};

  Texture1D * tex = new Texture1D(tex_data,GL_RGBA,GL_RGBA,GL_FLOAT,2);
  Texture2D * tex2 = new Texture2D(tex_data,GL_RGBA,GL_RGBA,GL_FLOAT,2,2);

  TextureBuffer * tex_buffer = new TextureBuffer();
  tex_buffer->add_texture(tex);
  tex_buffer->add_texture(tex2);*/

  Object * object = new Object();
  //object = new Object();
  //std::cout << object->get_parent() << std::endl;
  Object * object2 = new Object();
  //exit(0);
  object->set_render_flag(LIGHTABLE | SHADOWABLE);
  object->render(0,SHADOWABLE | LIGHTABLE,0,AND);
  object->add_child(object2);
  //  world->add_child(object);

  object->remove_child(object2);

  //world->set_projection(ProjectionBuffer::create_perspective_projection_matrix(-1/aspect_ratio,1/aspect_ratio,-1,1,1.0,50,100));
  Camera * mv2 = new FlyingCamera();
    mv2->set_position(glm::vec4(0,-9,-9,1));
    mv2->add_action(new KeyAction(-1,-1));
  //mv->render(0);
    
  //GLint max_nbr;
  //glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS,&max_nbr);
  //std::cout << max_nbr << std::endl;
  
  Tetrahedron * tetr = new Tetrahedron();
  //tetr->get_mesh()->get_center_of_mass(0);
  //tetr->get_mesh()->get_moment_of_inertia(0);
  Tetrahedron * displaced = new Tetrahedron();
  GLfloat disp[4] = {0,0,0,0};
  //displaced->get_mesh()->displace_mesh(0,4,disp);
  //displaced->get_mesh()->get_center_of_mass(0);
  Cube * cube = new Cube(2.0);
  //cube->get_mesh()->displace_mesh(0,4,disp);
  //cube->get_mesh()->get_center_of_mass(0);
  //cube->get_mesh()->get_moment_of_inertia(0,glm::vec3(10,5,2));
  /*
  RigidBody * rb = new RigidBody();
  rb->set_shape(cube);
  RigidBody * rb2 = new RigidBody();
  rb2->set_shape(cube);
  rb2->set_position(glm::vec4(-10,0,0,1));
  //rb2->set_orientation(glm::fquat(3,1,0,1));
  rb->add_child(rb2);
  rb->set_moment_of_inertia_and_mass();
  rb->apply_impulse(glm::vec3(0,250,0),glm::vec3(-10,0.0,0));
  rb->apply_impulse(glm::vec3(0,-250,0),glm::vec3(0,0.0,0));
  */
  /*
  ParametricPlot * sphere = new ParametricPlot(ParametricFunctions::parametric_sphere,glm::vec2(0,PI2),glm::vec2(0,PI),21,21,GL_TRUE,GL_TRUE);
  */
  /*
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  Program * fwprog = new Program("firework.vert","firework.frag");
  Program * texture_program = new Program("texture_material.vert","texture_material.frag");
  Program * text_program = new Program("texture_material.vert","text.frag");
  Program * lt_program = new Program("texture_material.vert","lightable_text.frag");
  UniformColorCollisionObject * co = new UniformColorCollisionObject();
  co->set_ambient_color(glm::vec4(0.0,1.0,0.0,1.0));

  ParametricPlot * co_tetr2 = new ParametricSphere(glm::vec3(0,0,0),1,51);
  co->set_shape(co_tetr);
  std::vector<glm::vec4> mesh;
  co->get_transformed_mesh_points(mesh,0);
  BoundingSphere * bs = BGAlgorithm::ritters_algorithm(mesh);
  //BoundingGeometry * bg = new BoundingSphere(co,0);
  GLfloat bs_radius = bs->get_radius();
  glm::vec4 bs_center = bs->get_center();
  Shape * bs_shape = bs->create_shape();
  Movable * co_bb = new Movable();
  co_bb->set_shape(bs_shape);
  co->add_child(co_bb);
  co_bb->set_program(wire_frame);
  co->set_program(prog);
  glm::mat4 test_transform = glm::mat4(1.0);
  test_transform[3] =glm::vec4(2.0,0.0,3.0,1.0);

  bs->transform_geometry(glm::vec4(10,0,0,1),glm::fquat(0.0,0.0,0.0,1.0),2,glm::vec3(0,0,0));
  bs_radius = bs->get_radius();
  bs_center = bs->get_center();

  */
  /*
    CollisionHierarchyNode * chnode = new CollisionHierarchyNode(collision_object);
  chnode->generate_bounding_geometry();
  chnode->create_bounding_geometry_object(wire_frame);
  chnode->update_bounding_geometry();
  collision_object->set_position(glm::vec4(10,0,0,1));
  chnode->update_bounding_geometry();
  collision_object->set_scale(3);
  chnode->update_bounding_geometry();
  */

  /*
  ParametricPlot * toroid = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0,PI2),glm::vec2(0,PI2),21,21,GL_TRUE,GL_TRUE);

  Texture2DObject * tex_obj = new Texture2DObject();
  tex_obj->set_shape(toroid);
  
  TextObject * text_object = new TextObject();
  LightableText * lightable_text = new LightableText();
  lightable_text->set_program(lt_program);
  lightable_text->set_text("Hello world!");
  lightable_text->set_position(glm::vec4(0,0,0,1));
  lightable_text->set_ambient_color(glm::vec4(0.05,0.05,0.05,1));
  lightable_text->set_diffusive_color(glm::vec4(0.1,0.0,0.5,1));
  lightable_text->set_render_flag_to_all(TRANSPARENT);
  text_object->set_program(text_program);
  text_object->set_text("Abc, kissa kavelee");
  text_object->set_position(glm::vec4(0,0,-3,1));
  text_object->set_render_flag_to_all(TRANSPARENT);
  tex_obj->set_program(texture_program);
  tex_obj->set_shininess(16);
  */
  //mv->set_shape(tetr);
  /*
    world->add_child(mv2);
    world->add_child(rb);
  */
  //world->add_child(co);
  //World * new_world = new World();
  //World::action_on_worlds();
  //mv->render(0);
  //World::action_on_worlds();
  //FireworkShape * fws = new FireworkShape(500,0.5,0.1,glm::vec3(0.5,0.1,0.0),glm::vec3(0.5,0.2,0.0));
  /*Movable * movable = new Movable();
  movable->set_shape(fws);
  movable->set_position(glm::vec4(0,0,-10,1));
  movable->set_program(fwprog);*/
  /*
  FireworkShooter * fwshooter = new FireworkShooter();
  fwshooter->set_position(glm::vec4(0,-10,0,1));
  Light * light = new Light();
  light->set_shape(cube);
  light->set_position(glm::vec4(0,-10,-2,1));
  light->set_program(wire_frame);
  light->set_intensity(glm::vec4(1.0,1.0,1.0,1.0));
  light->add_action(new OscillatorAction(0.1,10,glm::vec3(0,0,1),-1,-1));
  */

  /*
  UniformMaterial * unimat = new UniformMaterial();
  unimat->set_position(glm::vec4(-25,0,0,1));

  unimat->set_shape(sphere);
  unimat->set_program(uniprog);
  unimat->set_ambient_color(glm::vec4(0.1,0.0,0,1));
  unimat->set_diffusive_color(glm::vec4(1,1.0,1.0,1));
  */
  /*
  mv->set_program(prog);
  mv2->set_program(prog);
  //world->add_child(movable);
  //world->add_child(fwshooter);
  world->set_program(prog);
  //world->add_child(tex_obj);
  world->add_child(lightable_text);
  world->add_child(text_object);

  world->add_child(light);
  
  UniformMaterial * collision_object1 = new UniformMaterial();
  collision_object1->set_shape(tetr);
  collision_object1->add_action(new OscillatorAction(0.1,10,glm::vec3(1,0,1),-1,-1));
  collision_object1->set_program(uniprog);
  collision_object1->set_diffusive_color(glm::vec4(0.0,0.0,0.3,1.0));
  UniformMaterial * collision_object2 = new UniformMaterial();
  collision_object2->set_shape(tetr);
  collision_object2->set_program(uniprog);
  collision_object2->set_position(glm::vec4(-2,0,-2,1));
  collision_object2->set_diffusive_color(glm::vec4(0.0,0.0,0.3,1.0));

  UniformMaterial * collision_object3 = new UniformMaterial();
  collision_object3->set_shape(displaced);
  collision_object3->set_program(uniprog);
  collision_object3->set_position(glm::vec4(-2,0,-2,1));
  collision_object3->set_diffusive_color(glm::vec4(0.0,0.5,0.3,1.0));
  */
  /*
  SimpleCollisionDetector * scd = new SimpleCollisionDetector();
  world->add_child(collision_object1);
  world->add_child(collision_object2);
  world->add_child(collision_object3);
  scd->add_object(collision_object1);
  scd->add_object(collision_object2);
  scd->add_object(collision_object3);
  
  Aabb * aabb = SimpleCollisionDetector::generate_Aabb(tetr->get_mesh(),0);
  */
  /*Aabb * translated_aabb = SimpleCollisionDetector::multiply_Aabb_with_matrix(aabb, glm::translate(glm::mat4(1.0f),glm::vec3(0,0,5)));*/
  /*
  world->add_child(light2);
  world->add_child(unimat);
  */
  /*
  UniformColorCollisionObject * collision_object3 = new UniformColorCollisionObject();
  collision_object3->set_program(uniprog);
  collision_object3->set_preferred_bounding_volume(BoundingGeometryType::SPHERE);
  collision_object3->set_shape(co_tetr);
  collision_object3->set_position(glm::vec4(0,-15,-20,1));
  //collision_object2->add_action(new OscillatorAction(0.1,30,glm::vec3(0,0,1),-1,-1));
  collision_object3->set_ambient_color(glm::vec4(0.0,1.0,0.0,1.0));
  collision_object3->set_moment_of_inertia_and_mass();
  collision_object3->apply_impulse(glm::vec3(2000,0,0),collision_object3->get_center_of_mass()+glm::vec3(collision_object3->get_position()));
  world->add_child(collision_object3);
*/
  return mv2;
}

Camera * populate_obb_test(World * world) {

  /*
    std::vector<glm::vec4> points = {
    glm::vec4(-1,-1,-1,1),
    glm::vec4(1,-1,-1,1),
    glm::vec4(1,1,-1,1),
    glm::vec4(-1,1,-1,1),
    glm::vec4(-1,1,1,1)
    };
  */
  
  std::vector<glm::vec4> points = {
    glm::vec4(-5,0,0,1),
    glm::vec4(0,1,0,1),
    glm::vec4(1,2,0,1),
    glm::vec4(0,-1,0,1),
  };

  BoundingOBB * OBB = BGAlgorithm::simple_bounding_OBB(points);
  Shape * shape =  OBB->create_shape();
  //Cube * cube = new Cube(2.0);
  
  glm::vec3 q;
  CollisionDetectionAlgorithms::closest_point_on_obb_to_point(glm::vec3(-10,0,0),*OBB,q);
  std::cout << "q: " << q.x << ", " << q.y << ", " << q.z << std::endl;

  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  UniformMaterial * unimat = new UniformMaterial();
  unimat->set_position(glm::vec4(0,0,-10,1));

  unimat->set_shape(shape);
  unimat->set_program(prog);
  //unimat->set_ambient_color(glm::vec4(0.1,0.0,0,1));
  //unimat->set_diffusive_color(glm::vec4(1,1.0,1.0,1));
  world->add_child(unimat);

  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;

}

Camera * populate_world_wall_of_brigs(World * world) {
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  CollisionDetector * collision_detector = new CollisionDetector();
  collision_detector->set_bounding_geometry_object_program(wire_frame);
  PhysicsManager * pm = new PhysicsManager();
  world->add_simulation_manager(collision_detector);
  world->set_physcis_manager(pm);
  Force * gravity = new ConstantForce(ForceType::GRAVITY,glm::vec3(0,-0.1,0));
  pm->add_force(gravity);
  
  GLfloat brig_width = 1.0;
  GLfloat brig_height = 1.0;
  GLfloat brig_length = 1.0;
  Cuboid * brig = new Cuboid(brig_width,brig_height,brig_length);
  //ParametricSphere * brig = new ParametricSphere(glm::vec3(0,0,0),brig_width/1.41,21);
  ParametricSphere * ball = new ParametricSphere(glm::vec3(0,0,0),1,51);

  GLuint nbrigsy = 7;
  GLuint nbrigsx = 7;

  GLfloat marginx = 0.1;
  GLfloat marginy = 0.1;
  glm::vec3 brig_position = glm::vec3(0,0,-1);
  brig_position[0] = -(nbrigsx*brig_width/2.0f + (nbrigsx-1)*marginx/2.0f);
  for(GLuint ii=0; ii < nbrigsx; ii++) {
    brig_position[1] = -(nbrigsy*brig_height/2.0f + (nbrigsy-1)*marginy/2.0f);
    for(GLuint jj=0; jj < nbrigsy; jj++) {
      UniformColorCollisionObject * brig_object = new UniformColorCollisionObject();
      brig_object->set_program(uniprog);
      brig_object->set_preferred_bounding_volume(BoundingGeometryType::OBB);
      brig_object->set_shape(brig);

      brig_object->set_ambient_color(glm::vec4(0.1,0.0,0.0,1.0));
      brig_object->set_diffusive_color(glm::vec4(0.5,0.0,0.0,1.0));
      brig_object->set_scale(1.0);
      brig_object->set_position(glm::vec4(brig_position,1));
      brig_object->set_moment_of_inertia_and_mass();
      brig_object->toggle_force(ForceType::GRAVITY,GL_TRUE);
      world->add_child(brig_object);
      brig_position = brig_position + glm::vec3(0,brig_height+marginy,0);
    }
    brig_position = brig_position + glm::vec3(brig_width+marginx,0,0);
  }
  
  UniformColorCollisionObject * ball_object = new UniformColorCollisionObject();
  ball_object->set_program(uniprog);
  ball_object->set_preferred_bounding_volume(BoundingGeometryType::SPHERE);
  ball_object->set_shape(ball);

  ball_object->set_ambient_color(glm::vec4(0.0,0.0,0.1,1.0));
  ball_object->set_diffusive_color(glm::vec4(0.0,0.0,0.5,1.0));
  ball_object->set_scale(0.6);
  ball_object->set_position(glm::vec4(-3,3,2,1));
  ball_object->set_moment_of_inertia_and_mass();
  world->add_child(ball_object);

  ball_object->apply_impulse(glm::vec3(0,-10,-0.5*64),ball_object->get_center_of_mass()+glm::vec3(ball_object->get_position()));

  UniformColorCollisionObject * ball_object_2 = new UniformColorCollisionObject();
  ball_object_2->set_program(uniprog);
  ball_object_2->set_preferred_bounding_volume(BoundingGeometryType::SPHERE);
  ball_object_2->set_shape(ball);

  ball_object_2->set_ambient_color(glm::vec4(0.0,0.0,0.1,1.0));
  ball_object_2->set_diffusive_color(glm::vec4(0.5,0.0,0.5,1.0));
  ball_object_2->set_scale(2);
  ball_object_2->set_position(glm::vec4(3,-3,20,1));
  ball_object_2->set_moment_of_inertia_and_mass();
  world->add_child(ball_object_2);

  ball_object_2->apply_impulse(glm::vec3(0,1000,-5000),ball_object->get_center_of_mass()+glm::vec3(ball_object->get_position()));


  Light * light2 = new Light();
  Tetrahedron * tetr = new Tetrahedron();
  light2->set_shape(tetr);
  light2->set_position(glm::vec4(0,0,10,1));
  light2->set_program(prog);
  light2->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_bouncing_object(World * world) {
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * text_program = new Program("texture_material.vert","text.frag");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  CollisionDetector * collision_detector = new CollisionDetector();
  collision_detector->set_bounding_geometry_object_program(wire_frame);
  PhysicsManager * pm = new PhysicsManager();
  world->add_simulation_manager(collision_detector);
  world->set_physcis_manager(pm);
  Force * gravity = new ConstantForce(ForceType::GRAVITY,glm::vec3(0,-5,0));
  //pm->add_force(gravity);

  Force * point_gravity = new PointForce(ForceType::GRAVITY,glm::vec3(0,-50,-20),20000,10000);
  //pm->add_force(point_gravity);

  Cuboid * co_tetr = new Cuboid(10,10,10);
  ParametricSphere * sphere = new ParametricSphere(glm::vec3(0,0,0),5,51);
  //Cube * co_tetr = new Cube(8);
  
  /*
  TextObject *text_object = new TextObject();
  text_object->set_program(text_program);
  text_object->set_text("0");
  text_object->set_position(glm::vec4(0,0,20,1));
  world->add_child(text_object);
  */
  FPSTextObject * fps_text = new FPSTextObject();
  fps_text->set_fps_logger(&fps_logger);
  fps_text->set_program(text_program);
  fps_text->set_position(glm::vec4(0,0,-5,1));
  //fps_text->set_render_flag_to_all(TRANSPARENT);
  //fps_text->set_text("AaBb");
  //fps_text->set_text("kkalakukko");
  //fps_text->set_text(std::to_string(fps_logger.get_fps()));
  world->add_child(fps_text);
  // FPSTextObject * fps_text2 = new FPSTextObject();
  // fps_text2->set_fps_logger(&fps_logger);
  // fps_text2->set_program(text_program);
  // fps_text2->set_position(glm::vec4(-5,0,-5,1));
  // fps_text2->set_text("initialized..");
  // world->add_child(fps_text2);
  fps_text->set_render_flag_to_all(TRANSPARENT);
  //fps_text->set_to_default_text();

  
  UniformColorCollisionObject * collision_object2 = new UniformColorCollisionObject();
  collision_object2->set_program(uniprog);
  collision_object2->set_preferred_bounding_volume(BoundingGeometryType::OBB);
  collision_object2->set_shape(co_tetr);

  collision_object2->set_ambient_color(glm::vec4(0.0,1.0,0.0,1.0));
  collision_object2->set_scale(10.0);
  collision_object2->set_moment_of_inertia_and_mass();
  world->add_child(collision_object2);

  UniformColorCollisionObject * collision_object = new UniformColorCollisionObject();
  collision_object->set_program(uniprog);
  collision_object->set_ambient_color(glm::vec4(0.0,0,1.0,1.0));
  collision_object->set_preferred_bounding_volume(BoundingGeometryType::SPHERE);
  collision_object->set_shape(sphere);
  collision_object->set_scale(1.0);
  collision_object->set_coefficient_of_restitution(1.0);
  collision_object->toggle_force(ForceType::GRAVITY,GL_TRUE);
  collision_object->set_moment_of_inertia_and_mass();

  std::shared_ptr<MaterialColorCollisionCallbackFunction> callback_function(new MaterialColorCollisionCallbackFunction());
  collision_object->add_collision_callback_function_and_register(callback_function);
  world->add_child(collision_object);

  UniformColorCollisionObject * collision_object3 = new UniformColorCollisionObject();
  collision_object3->set_program(uniprog);
  collision_object3->set_ambient_color(glm::vec4(0.0,0,1.0,1.0));
  collision_object3->set_preferred_bounding_volume(BoundingGeometryType::SPHERE);
  collision_object3->set_shape(sphere);
  collision_object3->set_scale(1.0);
  collision_object3->set_coefficient_of_restitution(0.9);
  collision_object3->toggle_force(ForceType::GRAVITY,GL_TRUE);
  collision_object3->set_moment_of_inertia_and_mass();
  world->add_child(collision_object3);

  Force * com_gravity = new CenterOfMassForce(ForceType::GRAVITY,collision_object3,15000);
  pm->add_force(com_gravity);
  Force * com_gravity2 = new CenterOfMassForce(ForceType::GRAVITY,collision_object,15000);
  pm->add_force(com_gravity2);
  


  //collision_object->set_orientation(glm::fquat(std::cos(PI/2),std::sin(PI/2),std::sin(PI/2),std::sin(PI/2)*0.1));




  glm::vec4 position1(20,60,-20,1);
  glm::vec4 position2(0,-70,-20,1);
  glm::vec4 position3(-20,100,-20,1);

  glm::vec3 impulse11(0,5000,0);
  glm::vec3 impulse11_disp(0,0,0);
  glm::vec3 impulse12(0,0,0);
  glm::vec3 impulse12_disp(0,0,0);

  glm::vec3 impulse21(0,0,0);
  glm::vec3 impulse21_disp(0,0,0);

  glm::vec3 impulse31(0,-5000,0);
  glm::vec3 impulse31_disp(0,0,0);

  collision_object->set_position(position1);
  collision_object2->set_position(position2);
  collision_object3->set_position(position3);
  collision_object->apply_impulse(impulse11,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse11_disp);
  collision_object->apply_impulse(impulse12,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse12_disp);
  collision_object2->apply_impulse(impulse21,collision_object2->get_center_of_mass()+glm::vec3(collision_object2->get_position()) + impulse21_disp);
  collision_object3->apply_impulse(impulse31,collision_object3->get_center_of_mass()+glm::vec3(collision_object3->get_position()) + impulse31_disp);
  Light * light2 = new Light();
  Tetrahedron * tetr = new Tetrahedron();
  light2->set_shape(tetr);
  light2->set_position(glm::vec4(0,0,-20,1));
  light2->set_program(prog);
  light2->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  RenderingFunction * rdf = new OITRenderingFunction();
  mv->set_rendering_function(rdf);
  return mv;
}

Camera * populate_world_two_objects(World * world) {
  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  CollisionDetector * collision_detector = new CollisionDetector();
  collision_detector->set_bounding_geometry_object_program(wire_frame);
  PhysicsManager * pm = new PhysicsManager();
  world->add_simulation_manager(collision_detector);
  world->set_physcis_manager(pm);
  Force * gravity = new ConstantForce(ForceType::GRAVITY,glm::vec3(0,-1,0));
  pm->add_force(gravity);

  //ParametricSphere * co_tetr = new ParametricSphere(glm::vec3(0,0,0),1,11);
  //ParametricPlot * co_tetr = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),51,51,GL_TRUE,GL_TRUE);
  //ParametricPlot * co_tetr = new ParametricPlot(ParametricFunctions::parametric_shell,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),51,51,GL_FALSE,GL_TRUE);
  //ParametricPlot * co_tetr = new ParametricSphere(glm::vec3(0,0,0),6,51);
  Cuboid * co_tetr = new Cuboid(10,10,10);
  //Cube * co_tetr = new Cube(8);

 UniformColorCollisionObject * collision_object2 = new UniformColorCollisionObject();
  collision_object2->set_program(uniprog);
  collision_object2->set_preferred_bounding_volume(BoundingGeometryType::OBB);
  collision_object2->set_shape(co_tetr);

  collision_object2->set_ambient_color(glm::vec4(0.0,1.0,0.0,1.0));
  collision_object2->set_scale(1.0);
  collision_object2->set_moment_of_inertia_and_mass();
  world->add_child(collision_object2);

  UniformColorCollisionObject * collision_object = new UniformColorCollisionObject();
  collision_object->set_program(uniprog);
  collision_object->set_ambient_color(glm::vec4(0.0,0,1.0,1.0));
  collision_object->set_preferred_bounding_volume(BoundingGeometryType::OBB);
  collision_object->set_shape(co_tetr);
  collision_object->set_scale(1.0);

  //collision_object->set_orientation(glm::fquat(std::cos(PI/2),std::sin(PI/2),std::sin(PI/2),std::sin(PI/2)*0.1));
  collision_object->set_moment_of_inertia_and_mass();
  
  std::shared_ptr<MaterialColorCollisionCallbackFunction> callback_function(new MaterialColorCollisionCallbackFunction());
  collision_object->add_collision_callback_function_and_register(callback_function);
  world->add_child(collision_object);


  glm::vec4 position1(-15,4.7,-20,1);
  glm::vec4 position2(15,-4.7,-20,1);

  glm::vec3 impulse11(2000,0,0);
  glm::vec3 impulse11_disp(0,0,0);
  glm::vec3 impulse12(0,0,0);
  glm::vec3 impulse12_disp(0,0,0);

  glm::vec3 impulse21(-2000,0,0);
  glm::vec3 impulse21_disp(0,0,0);

  collision_object->set_position(position1);
  collision_object2->set_position(position2);
  collision_object->apply_impulse(impulse11,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse11_disp);
  collision_object->apply_impulse(impulse12,collision_object->get_center_of_mass()+glm::vec3(collision_object->get_position()) + impulse12_disp);
  collision_object2->apply_impulse(impulse21,collision_object2->get_center_of_mass()+glm::vec3(collision_object2->get_position()) + impulse21_disp);




  Light * light2 = new Light();
  Tetrahedron * tetr = new Tetrahedron();
  light2->set_shape(tetr);
  light2->set_position(glm::vec4(0,0,-20,1));
  light2->set_program(prog);
  light2->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_ring(World * world) {
    Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");
  CollisionDetector * collision_detector = new CollisionDetector();
  collision_detector->set_bounding_geometry_object_program(wire_frame);
  PhysicsManager * pm = new PhysicsManager();
  world->add_simulation_manager(collision_detector);
  world->set_physcis_manager(pm);
  //ParametricPlot * co_tetr = new ParametricPlot(ParametricFunctions::parametric_torus,glm::vec2(0.0,PI2),glm::vec2(0.0,PI2),51,51,GL_TRUE,GL_TRUE);
  Cuboid * co_tetr = new Cuboid(4,80,4);
  GLuint Nco =5;
  for(GLuint ii = 0; ii < Nco; ii++) {
    UniformColorCollisionObject * col_obj = new UniformColorCollisionObject();
    col_obj->set_program(uniprog);
    //col_obj->set_program(wire_frame);
    col_obj->set_ambient_color(glm::vec4(0.1,0.1,0.1,1.0));
    col_obj->set_diffusive_color(glm::vec4(0.5*std::sin(2*PI*(ii/static_cast<GLfloat>(Nco-1))) + 0.5,0.0,0.5*std::cos(2*PI*(ii/static_cast<GLfloat>(Nco-1))) + 0.5,1.0));
    col_obj->set_preferred_bounding_volume(BoundingGeometryType::AABB);
    col_obj->set_shape(co_tetr);
    col_obj->set_position(glm::vec4(300*std::cos(2*PI*ii/GLfloat(Nco)),300*std::sin(2*PI*ii/GLfloat(Nco)),-250,1));
    col_obj->set_coefficient_of_restitution(1.0);
    GLfloat scale = (1 + 5*ii/(static_cast<GLfloat>(Nco)-1))*0.1;
    //GLfloat scale = 4.0;
    if(Nco == 1) {
      scale = 1;
    }
    col_obj->set_scale(scale);
    col_obj->set_moment_of_inertia_and_mass();
    col_obj->apply_impulse(-glm::vec3(std::pow(scale,3)*20000*std::cos(2*PI*ii/GLfloat(Nco)),std::pow(scale,3)*20000*std::sin(2*PI*ii/GLfloat(Nco)),0),col_obj->get_center_of_mass()+glm::vec3(col_obj->get_position()));
    world->add_child(col_obj);
  }

  Light * light2 = new Light();
  Tetrahedron * tetr = new Tetrahedron();
  light2->set_shape(tetr);
  light2->set_position(glm::vec4(0,0,-20,1));
  light2->set_program(prog);
  light2->set_intensity(glm::vec4(1.0,1.0,1.0,1));
  world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  world->add_child(mv);
  return mv;
}

Camera * populate_world_geom_test(World * world) {
  //Program * prog = new Program("uniform_material.vert","uniform_material.frag");
  //Program * prog = new Program("uniform_material.vert","simple_prog.frag","wire.geom");
  Program * prog = new Program("uniform_material.vert","single_color.frag","simple_prog.geom");


  ParametricSphere * ball = new ParametricSphere(glm::vec3(0,0,0),1,3);


  std::vector<GLfloat> vertex_data = {-1,0,0,1,
				      1,0,0,1,
				      0,1,0,1,
				      -4,0,0,1,
				      -2,0,0,1,
				      -3,1,0,1};
  std::vector<GLuint> index_data = {0,1,2};
  std::vector<GLuint> adjacency_data = {0,1,2,3,4,5};
  IndexedShape * ts = new IndexedShape(vertex_data,index_data,adjacency_data);
   
   UniformMaterial * sphere_object = new UniformMaterial();
   GeometryShaderTestRF * rfun = new GeometryShaderTestRF();

   sphere_object->set_position(glm::vec4(0,0,-10,1));
   sphere_object->set_program(prog);
   sphere_object->set_shape(ball);
   //sphere_object->set_shape(ts);
   world->add_child(sphere_object);
   
   Camera * mv = new FlyingCamera();
   mv->set_position(glm::vec4(0,0,7.0,1));
   mv->add_action(new KeyAction(-1,-1));
   mv->set_program(prog);
   world->add_child(mv);

   mv->set_rendering_function(rfun);
   return mv;

}

Camera * populate_world_shadows(World * world) {

  Program * uniprog = new Program("uniform_material.vert","uniform_material.frag");
  //Program * ambient = new Program("shadow_volumes.vert","shadow_volumes.frag","shadow_volumes.geom");
  Program * ambient = new Program("uniform_material.vert","ambient.frag");
  Program * shadow_volumes_prog = new Program("shadow_volumes.vert","shadow_volumes.frag","shadow_volumes.geom");
  //Program * lightning_prog = new Program("uniform_material.vert","lightning.frag","shadow_volumes.geom");
  //Program * lightning_prog = new Program("uniform_material.vert","ambient.frag","shadow_volumes.geom");
    Program * lightning_prog = new Program("uniform_material.vert","uniform_material.frag");
  Program * wire_frame = new Program("texture_material.vert","single_color.frag","wire.geom");
  Program * prog = new Program("simple_prog.vert","simple_prog.frag");

  ShadowVolumesRenderingFunction * rfun = new ShadowVolumesRenderingFunction();

  Light * light1 = new Light();
  Tetrahedron * tetr = new Tetrahedron();
  light1->set_shape(tetr);
  light1->set_position(glm::vec4(-10,0,-20,1));
  light1->set_program(prog);
  light1->set_intensity(glm::vec4(0.0,0.0,100.0,1));
  light1->set_kl(20.0);
  //light1->set_render_flag(SHADOWCASTER);
  Light * light2 = new Light();
  light2->set_shape(tetr);
  light2->set_position(glm::vec4(10,0,-20,1));
  light2->set_program(prog);
  light2->set_intensity(glm::vec4(100.0,0.0,0.0,1));
  light2->set_kl(20.0);
  //light2->set_render_flag(SHADOWCASTER);
  //light1->disable();
  //light2->disable();

  ParametricSphere * ball = new ParametricSphere(glm::vec3(0,0,0),1,101);

  std::vector<GLfloat> vertex_data = {-1,0,0,1,
				      1,0,0,1,
				      0,1,0,1};
  std::vector<GLuint> index_data = {0,1,2,0,2,1};
  std::vector<GLuint> adjacency_data = {0,1,2,0,1,2,0,2,1,0,2,1};
  IndexedShape * ts = new IndexedShape(vertex_data,index_data,adjacency_data);

  

  
  //ball->create_adjacency_data();
  UniformMaterial * sphere_object = new UniformMaterial();
  UniformMaterial * sphere_object2 = new UniformMaterial();

  //sphere_object->set_shape(ts);

  sphere_object->set_shape(ball);
  //sphere_object->set_shape(tetr);
  sphere_object->set_program(ambient,0);
  sphere_object->set_program(shadow_volumes_prog,1);
  sphere_object->set_program(lightning_prog,2);
  sphere_object->set_ambient_color(glm::vec4(0.2,0.2,0.2,1.0));
  sphere_object->set_diffusive_color(glm::vec4(1,1,1,1));
  //sphere_object->set_specular_color(glm::vec4(50,50,50,1));
  sphere_object->set_position(glm::vec4(0,0,-10,1));
  sphere_object->set_render_flag(SHADOWABLE|SHADOWCASTER);
  sphere_object->add_action(new OscillatorAction(1,12,glm::vec3(0,0,1),-1,-1));
  sphere_object->set_scale(glm::vec3(0.5,0.5,0.5));
  world->add_child(sphere_object);

  sphere_object2->set_shape(ball);
  //sphere_object2->set_shape(tetr);
  sphere_object2->set_program(ambient,0);
  sphere_object2->set_program(shadow_volumes_prog,1);
  sphere_object2->set_program(lightning_prog,2);
  sphere_object2->set_ambient_color(glm::vec4(0.2,0.2,0.2,1.0));
  sphere_object2->set_diffusive_color(glm::vec4(1,1,1,1));
  //sphere_object2->set_specular_color(glm::vec4(50,50,50,1));
  sphere_object2->set_position(glm::vec4(-5,0,-5,1));
  sphere_object2->set_render_flag(SHADOWABLE|SHADOWCASTER);
  world->add_child(sphere_object2);
  
  world->add_child(light1);
  world->add_child(light2);
  Camera * mv = new FlyingCamera();
  mv->set_position(glm::vec4(0,0,7.0,1));
  mv->add_action(new KeyAction(-1,-1));
  mv->set_program(prog);
  mv->set_rendering_function(rfun);
  world->add_child(mv);
  return mv;
}

int main() {
  init_GL();
  //fps_logger = FPSLogger(10);
  kt = new KeyTable();

  World * world = new World();
  world->set_projection(ProjectionBuffer::create_perspective_projection_matrix(-1*aspect_ratio,1*aspect_ratio,-1,1,1,2000,500));
  std::cout << "aspect ratio: " << aspect_ratio << "From main.cpp/main." << std::endl;

  // Camera * mv = populate_world_ring(world);
  // Camera * mv = populate_world_two_objects(world);
  //Camera * mv = populate_obb_test(world);
  //Camera * mv = populate_world_wall_of_brigs(world);
  //Camera * mv = populate_world_bouncing_object(world);
  //Camera * mv = populate_world_junk(world);
  //Camera * mv = populate_world_triangle_line_intersection(world);
  //Camera * mv = populate_world_circumsphere(world);
  //Camera * mv = populate_world_farthest_point_to_triangle(world);
  //Camera * mv = populate_world_convex_hull(world);
  //Camera * mv = populate_world_nvidia_test(world);
  Camera * mv = populate_world_shadows(world);
  //Camera * mv = populate_world_geom_test(world);

  glClearColor(0.0,0.0,0.0,0.0);
  glViewport(0,0,SCREEN_W,SCREEN_H); // TODO! Should this really be here in the loop?
  
  SDL_GL_MakeCurrent(window, NULL); // Free context so that it can be claimed by some other thread.
  std::thread simulation_thread(simulation_loop);
  std::thread drawing_thread(drawing_loop,mv);
  simulation_thread.join();
  drawing_thread.join();
  
  return 0;
}
