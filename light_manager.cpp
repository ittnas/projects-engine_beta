#include "light_manager.hpp"


LightManager::LightManager() : Manager(), Buffer("nbr_lights_buffer",buffer_length) {
}

void LightManager::add_light(Light * light) {
  lights.insert(std::make_pair(light,0));
}

void LightManager::remove_light(Light * light) {
  //lights.erase(std::remove(lights.begin(),lights.end(),light),lights.end());
  lights.erase(light);
}

GLuint LightManager::get_nbr_active_lights() {
  GLuint nbr_active_lights = 0;
  for(auto light_pair : lights) {
    if(light_pair.second) {
      nbr_active_lights++;
    }
  }
  return nbr_active_lights;
}

void LightManager::update_light_indices_single_pass() {
  //for(auto lights)
}

GLboolean LightManager::update_light_indices_multi_pass() {
  GLuint light_index = 0;
  for(auto light : lights) {
    if(light.second) {
      light.first->set_light_index(light_index);
      light_index++;
    }
  }
}

void LightManager::mark_active_lights() {
  //DUMMY, all lights are active
  for(std::unordered_map<Light *,GLboolean>::iterator it = lights.begin(); it!= lights.end(); it++) {
    it->second = GL_TRUE;
  }
  /*
  for(auto light_pair : lights) {
    std::cout << light_pair.first << ":" << light_pair.second << std::endl;
  }
  */
}

void LightManager::register_object(Object * object) {
  Light * light = dynamic_cast<Light *>(object);
  if(light != 0) {
    add_light(light);
  }
}

void LightManager::deregister_object(Object * object) {
  Light * light = dynamic_cast<Light *>(object);
  if(light != 0) {
    remove_light(light);
  }
}

void LightManager::update_buffer_phase1(Object * parent) {
  GLuint nbr_active_lights = get_nbr_active_lights();
  glBindBuffer(GL_UNIFORM_BUFFER,get_ubo_pointer());
  glBufferSubData(GL_UNIFORM_BUFFER,0,sizeof(GLuint),&nbr_active_lights);
  glBindBuffer(GL_UNIFORM_BUFFER,0);
  
}

void LightManager::manage() {
  mark_active_lights();
  update_light_indices_multi_pass();
}

const std::unordered_map<Light *,GLboolean> & LightManager::get_lights() const {return lights;}
