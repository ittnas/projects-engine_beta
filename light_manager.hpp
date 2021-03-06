#ifndef LIGHT_MANAGER_HPP
#define LIGHT_MANAGER_HPP

#include <vector>
#include <algorithm>

#include "light.hpp"
#include "gl_constants.hpp"
#include "manager.hpp"
#include "buffer.hpp"

class LightManager : public Manager, public Buffer  {
private:
  std::unordered_map<Light *,GLboolean> lights;
  //NbrLightsBuffer * nbr_lights_buffer;
  static const GLuint buffer_length = 4; //Just one int
protected:
  virtual void mark_active_lights(); // You should override this to modify the lightning_manager.
public:
  LightManager();
  virtual ~LightManager() {}
  virtual void register_object(Object * object);
  virtual void deregister_object(Object * object);
  void manage(); // Not virtual any more. This is a basic behaviour of a light_manager and cannot be overridden.
  void add_light(Light * light);
  void remove_light(Light * light);
  GLuint get_nbr_active_lights();
  void update_light_indices_single_pass();
  GLboolean update_light_indices_multi_pass();
  virtual void update_buffer_phase1(Object * parent);
  const std::unordered_map<Light *,GLboolean> & get_lights() const;
};

#endif
