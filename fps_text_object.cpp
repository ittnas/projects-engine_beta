#include "fps_text_object.hpp"

FPSTextObject::FPSTextObject() : TextObject(),fps_logger(NULL), previous_update(-1000) {
  //set_text_action = new SetTextAction();
  //add_action(set_text_action);
}

void FPSTextObject::set_fps_logger(FPSLogger * fps_logger) {
  this->fps_logger = fps_logger;
}

void FPSTextObject::set_to_default_text() {
  if(fps_logger != NULL) {
    set_text("FPS: " + std::to_string(fps_logger->get_fps()));
    //set_text("FPS");
  } else {
    set_text("FPS: not defined");
  }
}

FPSTextObject::~FPSTextObject() {
}

void FPSTextObject::action_phase_function() {
  
}

void FPSTextObject::post_render_function(GLint tick) {
  //std::cout << "Tick: " << tick << ", previous_update: " << previous_update << ", difference: " << tick-previous_update <<  std::endl;
  
  if(tick - previous_update > 0) {
    previous_update = tick;
    set_to_default_text();
  }
  
}

// Apparently set text action is not used. However, it might be useful in the future.
SetTextAction::SetTextAction(GLint stop, GLint start) : Action(stop,start) {}

void SetTextAction::perform_action(Object * target, GLint time) {
  TextObject * to = dynamic_cast<TextObject *>(target);
  to->set_to_default_text();
  //to->set_position(to->get_position() + glm::vec4(0.01,0,0,0));
}
