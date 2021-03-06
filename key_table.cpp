#include "key_table.hpp"

const GLuint KeyTable::symbols[] = {SDLK_UNKNOWN,
				    SDLK_BACKSPACE,
				    SDLK_TAB,
				    SDLK_RETURN,
				    SDLK_ESCAPE,
				    SDLK_SPACE,
				    SDLK_EXCLAIM,
				    SDLK_QUOTEDBL,
				    SDLK_HASH,
				    SDLK_DOLLAR,
				    //			    SDKL_PERCENT, // Not declared in this scope :(
				    SDLK_AMPERSAND,
				    SDLK_QUOTE,
				    SDLK_LEFTPAREN,
				    SDLK_RIGHTPAREN,
				    SDLK_ASTERISK,
				    SDLK_PLUS,
				    SDLK_COMMA,
				    SDLK_MINUS,
				    SDLK_PERIOD,
				    SDLK_SLASH,
				    SDLK_0,
				    SDLK_1,
				    SDLK_2,
				    SDLK_3,
				    SDLK_4,
				    SDLK_5,
				    SDLK_6,
				    SDLK_7,
				    SDLK_8,
				    SDLK_9,
				    SDLK_COLON,
				    SDLK_SEMICOLON,
				    SDLK_LESS,
				    SDLK_EQUALS,
				    SDLK_GREATER,
				    SDLK_QUESTION,
				    SDLK_AT,
				    SDLK_LEFTBRACKET,
				    SDLK_BACKSLASH,
				    SDLK_RIGHTBRACKET,
				    SDLK_CARET,
				    SDLK_UNDERSCORE,
				    SDLK_BACKQUOTE,
				    SDLK_a,
				    SDLK_b,
				    SDLK_c,
				    SDLK_d,
				    SDLK_e,
				    SDLK_f,
				    SDLK_g,
				    SDLK_h,
				    SDLK_i,
				    SDLK_j,
				    SDLK_k,
				    SDLK_l,
				    SDLK_m,
				    SDLK_n,
				    SDLK_o,
				    SDLK_p,
				    SDLK_q,
				    SDLK_r,
				    SDLK_s,
				    SDLK_t,
				    SDLK_u,
				    SDLK_v,
				    SDLK_w,
				    SDLK_x,
				    SDLK_y,
				    SDLK_z,
				    SDLK_DELETE,
				    SDLK_CAPSLOCK,
				    SDLK_F1,
				    SDLK_F2,
				    SDLK_F3,
				    SDLK_F4,
				    SDLK_F5,
				    SDLK_F6,
				    SDLK_F7,
				    SDLK_F8,
				    SDLK_F9,
				    SDLK_F10,
				    SDLK_F11,
				    SDLK_F12,
				    //SDLK_PRINTSCREEN,
				    //SDLK_SCROLLLOCK,
				    SDLK_PAUSE,
				    SDLK_INSERT,
				    SDLK_HOME,
				    //SDLK_PAGUP,
				    SDLK_END,
				    SDLK_PAGEDOWN,
				    SDLK_RIGHT,
				    SDLK_LEFT,
				    SDLK_DOWN,
				    SDLK_UP
				    
};

std::unordered_map<GLuint,GLubyte> KeyTable::keys;
GLboolean KeyTable::initialized = GL_FALSE;

KeyTable::KeyTable() {
  if(!initialized) {
    initialized = GL_TRUE;
    for(GLuint i = 0; i < nbr_keys; i++) {
      keys[symbols[i]] = 0; // Key not pressed.
    }
  }
}

void KeyTable::set_status(GLuint key,GLubyte status) {
  keys[key] = status;
}

GLubyte KeyTable::get_status(GLuint key) {
  return keys[key];
}

void KeyTable::toggle(GLuint key) {
  auto value = keys[key];
  if(value) {
    keys[key] = 0;
  } else {
    keys[key] = 1;
  }
}
