/* Copyright 2017 Cedric Mesnil <cslashm@gmail.com>, Ledger SAS
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */



#include "os.h"
#include "cx.h"
#include "gpg_types.h"
#include "gpg_api.h"
#include "gpg_vars.h"

#include "gpg_ux_msg.h"
#include "os_io_seproxyhal.h"
#include "usbd_ccid_impl.h"
#include "string.h"
#include "glyphs.h"


/* ----------------------------------------------------------------------- */
/* ---                        NanoS  UI layout                         --- */
/* ----------------------------------------------------------------------- */

#define PICSTR(x)     ((char*)PIC(x))

#define TEMPLATE_TYPE     PICSTR(C_TEMPLATE_TYPE)
#define TEMPLATE_KEY      PICSTR(C_TEMPLATE_KEY)
#define INVALID_SELECTION PICSTR(C_INVALID_SELECTION)
#define OK                PICSTR(C_OK)
#define NOK               PICSTR(C_NOK)
#define WRONG_PIN         PICSTR(C_WRONG_PIN)
#define RIGHT_PIN         PICSTR(C_RIGHT_PIN)
#define PIN_CHANGED       PICSTR(C_PIN_CHANGED)
#define PIN_DIFFERS       PICSTR(C_PIN_DIFFERS)
#define PIN_USER          PICSTR(C_PIN_USER)
#define PIN_ADMIN         PICSTR(C_PIN_ADMIN)
#define VERIFIED          PICSTR(C_VERIFIED)
#define NOT_VERIFIED      PICSTR(C_NOT_VERIFIED)
#define ALLOWED           PICSTR(C_ALLOWED)
#define NOT_ALLOWED       PICSTR(C_NOT_ALLOWED)
#define DEFAULT_MODE      PICSTR(C_DEFAULT_MODE)



const ux_menu_entry_t ui_menu_template[];
void ui_menu_template_display(unsigned int value);
const bagl_element_t*  ui_menu_template_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element);
void ui_menu_tmpl_set_action(unsigned int value) ;
const ux_menu_entry_t ui_menu_tmpl_key[];
void ui_menu_tmpl_key_action(unsigned int value);
const ux_menu_entry_t ui_menu_tmpl_type[];
void ui_menu_tmpl_type_action(unsigned int value);

const ux_menu_entry_t ui_menu_seed[];
void ui_menu_seed_display(unsigned int value);
const bagl_element_t*  ui_menu_seed_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) ;
void ui_menu_seed_action(unsigned int value) ;

const ux_menu_entry_t ui_menu_reset[] ;
void ui_menu_reset_action(unsigned int value);

const ux_menu_entry_t ui_menu_slot[];
void ui_menu_slot_display(unsigned int value);
const bagl_element_t*  ui_menu_slot_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element);
void ui_menu_slot_action(unsigned int value);

const ux_menu_entry_t ui_menu_settings[] ;

const ux_menu_entry_t ui_menu_main[];
void ui_menu_main_display(unsigned int value) ;
const bagl_element_t* ui_menu_main_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element);

const bagl_element_t ui_pinconfirm_nanos[];
void  ui_menu_pinconfirm_action(unsigned int value);
unsigned int ui_pinconfirm_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int ui_pinconfirm_prepro(const bagl_element_t* element) ;


const bagl_element_t ui_pinentry_nanos[];
void ui_menu_pinentry_display(unsigned int value) ;
void  ui_menu_pinentry_action(unsigned int value);
unsigned int ui_pinentry_nanos_button(unsigned int button_mask, unsigned int button_mask_counter);
unsigned int ui_pinentry_prepro(const bagl_element_t* element) ;
static unsigned int validate_pin();

/* ------------------------------- Helpers  UX ------------------------------- */
void ui_CCID_reset(void) {
  io_usb_ccid_set_card_inserted(0);
  io_usb_ccid_set_card_inserted(1);
}

void ui_info(const char* msg1, const char* msg2, const void *menu_display, unsigned int value) {
  os_memset(&G_gpg_vstate.ui_dogsays[0], 0, sizeof(ux_menu_entry_t));
  G_gpg_vstate.ui_dogsays[0].callback = menu_display;
  G_gpg_vstate.ui_dogsays[0].userid   = value;
  G_gpg_vstate.ui_dogsays[0].line1    = msg1;
  G_gpg_vstate.ui_dogsays[0].line2    = msg2;

  os_memset(&G_gpg_vstate.ui_dogsays[1],0, sizeof(ux_menu_entry_t));
  UX_MENU_DISPLAY(0, G_gpg_vstate.ui_dogsays, NULL);
};

/* ------------------------------ PIN CONFIRM UX ----------------------------- */

const bagl_element_t ui_pinconfirm_nanos[] = {
  // type             userid    x    y    w    h    str   rad  fill              fg        bg     font_id                   icon_id  
  { {BAGL_RECTANGLE,  0x00,     0,   0, 128,  32,    0,    0,  BAGL_FILL,  0x000000, 0xFFFFFF,    0,                         0}, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL},

  { {BAGL_ICON,       0x00,    3,   12,   7,   7,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_CROSS  }, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL },
    
  { {BAGL_ICON,       0x00,  117,   13,   8,   6,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_CHECK  }, 
     NULL, 
     0, 
     0, 0, 
     NULL, NULL, NULL },

  { {BAGL_LABELINE,   0x01,    0,   12, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_EXTRABOLD_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    G_gpg_vstate.menu,  
    0, 
    0, 0, 
    NULL, NULL, NULL },
  { {BAGL_LABELINE,   0x02,    0,   26, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_EXTRABOLD_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    G_gpg_vstate.menu,  
    0, 
    0, 0, 
    NULL, NULL, NULL },
};

void ui_menu_pinconfirm_display(unsigned int value) {
 UX_DISPLAY(ui_pinconfirm_nanos, (void*)ui_pinconfirm_prepro);   
}

unsigned int ui_pinconfirm_prepro(const  bagl_element_t* element) {
  if (element->component.userid == 1) {
    if ((G_gpg_vstate.io_p2 == 0x81) || (G_gpg_vstate.io_p2 == 0x82) || (G_gpg_vstate.io_p2 == 0x83)) {
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Confirm PIN");
      return 1;
    } 
  }
  if (element->component.userid == 2) {
    if ((G_gpg_vstate.io_p2 == 0x81) || (G_gpg_vstate.io_p2 == 0x82)|| (G_gpg_vstate.io_p2 == 0x83)) {
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s %x", G_gpg_vstate.io_p2 == 0x83?"Admin":"User",G_gpg_vstate.io_p2);
      return 1;
    }
  }
  snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Please Cancel");
  return 1;
}

unsigned int ui_pinconfirm_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  unsigned int sw;

  sw = 0x6985;
  switch(button_mask) {
  case BUTTON_EVT_RELEASED|BUTTON_LEFT: // CANCEL
    gpg_set_pin_verified(G_gpg_vstate.io_p2&0x0F,0);
    sw = 0x6985;            
    break;

  case BUTTON_EVT_RELEASED|BUTTON_RIGHT:  // OK
    gpg_set_pin_verified(G_gpg_vstate.io_p2&0x0F,1);
    sw = 0x9000;            
    break;
  default:
    return 0;
  }
  gpg_io_discard(0);
  gpg_io_insert_u16(sw);
  gpg_io_do(IO_RETURN_AFTER_TX);
  ui_menu_main_display(0);
  return 0;
}
    
/* ------------------------------- PIN ENTRY UX ------------------------------ */


const bagl_element_t ui_pinentry_nanos[] = {
  // type             userid    x    y    w    h    str   rad  fill              fg        bg     font_id                   icon_id  
  { {BAGL_RECTANGLE,  0x00,     0,   0, 128,  32,    0,    0,  BAGL_FILL,  0x000000, 0xFFFFFF,    0,                         0}, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL},

  { {BAGL_ICON,       0x00,    3,   12,   7,   7,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_DOWN  }, 
    NULL, 
    0, 
    0, 0, 
    NULL, NULL, NULL },
    
  { {BAGL_ICON,       0x00,  117,   13,   8,   6,    0,    0,         0,   0xFFFFFF, 0x000000,    0,                          BAGL_GLYPH_ICON_UP  }, 
     NULL, 
     0, 
     0, 0, 
     NULL, NULL, NULL },

  { {BAGL_LABELINE,   0x01,    0,   12, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_EXTRABOLD_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    G_gpg_vstate.menu,  
    0, 
    0, 0, 
    NULL, NULL, NULL },
  { {BAGL_LABELINE,   0x02,    0,   26, 128,  32,    0,    0,         0,   0xFFFFFF, 0x000000,    BAGL_FONT_OPEN_SANS_REGULAR_11px|BAGL_FONT_ALIGNMENT_CENTER, 0  }, 
    G_gpg_vstate.menu,  
    0, 
    0, 0, 
    NULL, NULL, NULL },
};

static const char C_pin_digit[] = {'0','1','2','3','4','5','6','7','8','9','C','A','V'};

void ui_menu_pinentry_display(unsigned int value) {
  if (value == 0) {
    os_memset(G_gpg_vstate.ux_pinentry, 0, sizeof(G_gpg_vstate.ux_pinentry));
    G_gpg_vstate.ux_pinentry[0]=1;
    G_gpg_vstate.ux_pinentry[1]=5;
  }
  UX_DISPLAY(ui_pinentry_nanos, (void*)ui_pinentry_prepro); 
}

unsigned int ui_pinentry_prepro(const  bagl_element_t* element) {
  if (element->component.userid == 1) {
    if (G_gpg_vstate.io_ins == 0x24) { 
      switch  (G_gpg_vstate.io_p1) {
      case 0:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Current %s PIN",  (G_gpg_vstate.io_p2 == 0x83)?"Admin":"User");
        break;
      case 1:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "New %s PIN",  (G_gpg_vstate.io_p2 == 0x83)?"Admin":"User");
        break;
      case 2:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Confirm %s PIN",  (G_gpg_vstate.io_p2 == 0x83)?"Admin":"User");
        break;
        default:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "WAT %s PIN",  (G_gpg_vstate.io_p2 == 0x83)?"Admin":"User");
        break;
      }
    } else {
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s PIN",  (G_gpg_vstate.io_p2 == 0x83)?"Admin":"User");
    }
  }
  else if (element->component.userid == 2) {
    unsigned int i;
    G_gpg_vstate.menu[0] = ' ';
    for (i = 1; i<= G_gpg_vstate.ux_pinentry[0]; i++) {
      G_gpg_vstate.menu[i] = C_pin_digit[G_gpg_vstate.ux_pinentry[i]]; 
    }
    for (; i<= GPG_MAX_PW_LENGTH;i++) {
      G_gpg_vstate.menu[i] = '-';
    }
     G_gpg_vstate.menu[i] = 0;
  }
  
  return 1;
}

unsigned int ui_pinentry_nanos_button(unsigned int button_mask, unsigned int button_mask_counter) {
  unsigned int offset = G_gpg_vstate.ux_pinentry[0];
  unsigned m_pinentry;
  char digit ;

  m_pinentry = 1;

  switch(button_mask) {
  case BUTTON_EVT_RELEASED|BUTTON_LEFT: // Down
  if (G_gpg_vstate.ux_pinentry[offset]) {
    G_gpg_vstate.ux_pinentry[offset]--;
  } else {
    G_gpg_vstate.ux_pinentry[offset] = sizeof(C_pin_digit)-1;
  }
  ui_menu_pinentry_display(1);
  break;
  
  case BUTTON_EVT_RELEASED|BUTTON_RIGHT:  //up
    G_gpg_vstate.ux_pinentry[offset]++;
    if (G_gpg_vstate.ux_pinentry[offset] == sizeof(C_pin_digit)) {
      G_gpg_vstate.ux_pinentry[offset] = 0;
    }
    ui_menu_pinentry_display(1);
    break;
  
  case BUTTON_EVT_RELEASED|BUTTON_LEFT|BUTTON_RIGHT: 
     digit = C_pin_digit[G_gpg_vstate.ux_pinentry[offset]];
    //next digit
    if ((digit >= '0') && (digit <= '9')) {
      offset ++;
      G_gpg_vstate.ux_pinentry[0] = offset;
      if (offset == GPG_MAX_PW_LENGTH+1) {
          validate_pin();         
      } else {
        G_gpg_vstate.ux_pinentry[offset] = 5;
        ui_menu_pinentry_display(1);
      }
    } 
    //cancel digit
    else if (digit == 'C') {
      if (offset > 1) {       
        offset--;
        G_gpg_vstate.ux_pinentry[0] = offset;
      }
      ui_menu_pinentry_display(1);
    } 
    //validate pin
    else if (digit == 'V') {
      G_gpg_vstate.ux_pinentry[0] = offset-1;
      validate_pin();
    }
    //cancel input without check
    else { //(digit == 'A') 
      gpg_io_discard(0);
      gpg_io_insert_u16(SW_CONDITIONS_NOT_SATISFIED);
      gpg_io_do(IO_RETURN_AFTER_TX);
      ui_menu_main_display(0);
    } 
  }
  return 0; 
}
// >= 0
static unsigned int validate_pin() {
  unsigned int offset, len, sw;

  for (offset = 1; offset< G_gpg_vstate.ux_pinentry[0];offset++) {
    G_gpg_vstate.menu[offset] = C_pin_digit[G_gpg_vstate.ux_pinentry[offset]]; 
  }

  if (G_gpg_vstate.io_ins == 0x20) {    
    if (gpg_check_pin(G_gpg_vstate.io_p2&0x0F, (unsigned char*)(G_gpg_vstate.menu+1), G_gpg_vstate.ux_pinentry[0])) {
      sw = SW_OK;
    } else {
      sw = SW_CONDITIONS_NOT_SATISFIED;
    }
    gpg_io_discard(1);
    gpg_io_insert_u16(sw);
    gpg_io_do(IO_RETURN_AFTER_TX);
    if (sw == SW_CONDITIONS_NOT_SATISFIED) {
      ui_info(WRONG_PIN, NULL, ui_menu_main_display, 0);
    } else {
      ui_info(RIGHT_PIN, NULL, ui_menu_main_display, 0);
    }
    return 0;
  }

  if (G_gpg_vstate.io_ins == 0x24) {
    if (G_gpg_vstate.io_p1 <= 2) {
      gpg_io_insert_u8(G_gpg_vstate.ux_pinentry[0]);
      gpg_io_insert((unsigned char*)(G_gpg_vstate.menu+1), G_gpg_vstate.ux_pinentry[0]);
      G_gpg_vstate.io_p1++;
    }
    if (G_gpg_vstate.io_p1 == 3) {
      if (!gpg_check_pin(G_gpg_vstate.io_p2&0x0F, G_gpg_vstate.work.io_buffer+1, G_gpg_vstate.work.io_buffer[0])) {
        gpg_io_discard(1);
        gpg_io_insert_u16(SW_CONDITIONS_NOT_SATISFIED);
        gpg_io_do(IO_RETURN_AFTER_TX);
        ui_info(WRONG_PIN, NULL, ui_menu_main_display, 0);       
        return 0;
      }
      offset =  1+G_gpg_vstate.work.io_buffer[0];
      len = G_gpg_vstate.work.io_buffer[offset];
      if ((len !=  G_gpg_vstate.work.io_buffer[offset+ 1+len]) ||
          (os_memcmp(G_gpg_vstate.work.io_buffer+offset + 1, G_gpg_vstate.work.io_buffer+offset+ 1+len+1, len) != 0)) {
        gpg_io_discard(1);
        gpg_io_insert_u16(SW_CONDITIONS_NOT_SATISFIED);
        gpg_io_do(IO_RETURN_AFTER_TX);
        ui_info(PIN_DIFFERS, NULL, ui_menu_main_display, 0);
      } else {
        gpg_change_pin(G_gpg_vstate.io_p2&0x0F, G_gpg_vstate.work.io_buffer+offset+ 1, len);
        gpg_io_discard(1);
        gpg_io_insert_u16(SW_OK);      
        gpg_io_do(IO_RETURN_AFTER_TX);
        ui_info(PIN_CHANGED, NULL, ui_menu_main_display, 0);
      }
      return 0;
    } else {
      ui_menu_pinentry_display(0);
    }
  }
  return 0;
}
/* ------------------------------- template UX ------------------------------- */

#define LABEL_SIG      "Signature"
#define LABEL_AUT      "Authentication"
#define LABEL_DEC      "Decryption"

#define LABEL_RSA2048  "RSA 2048"
#define LABEL_RSA3072  "RSA 3072"
#define LABEL_RSA4096  "RSA 4096"
#define LABEL_NISTP256 "NIST P256"
#define LABEL_BPOOLR1  "Brainpool R1"
#define LABEL_Ed25519  "Ed25519"

const ux_menu_entry_t ui_menu_template[] = {
  {ui_menu_tmpl_key,           NULL,  -1, NULL,          "Choose key...",   NULL, 0, 0},
  {ui_menu_tmpl_type,          NULL,  -1, NULL,          "Choose type...",  NULL, 0, 0},
  {NULL,    ui_menu_tmpl_set_action,  -1, NULL,          "Set template",    NULL, 0, 0},
  {ui_menu_settings,           NULL,   0, &C_badge_back, "Back",            NULL, 61, 40},
  UX_MENU_END
};

void ui_menu_template_display(unsigned int value) {
   UX_MENU_DISPLAY(value, ui_menu_template, ui_menu_template_preprocessor);
}

const bagl_element_t* ui_menu_template_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
  if(element->component.userid==0x20) {
    if (entry == &ui_menu_template[0]) {
      switch(G_gpg_vstate.ux_key) {
      case 1: 
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s", LABEL_SIG);
        break;
      case 2: 
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s", LABEL_DEC);
        break;
      case 3: 
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s", LABEL_AUT);
        break;
      default:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Choose key...");
        break;
      }
      element->text = G_gpg_vstate.menu;    
    }
    if (entry == &ui_menu_template[1]) {
      switch(G_gpg_vstate.ux_type) {
      case 2048:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_RSA2048);
        break;
      case 3072:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_RSA3072);
        break;
      case 4096:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_RSA4096);
        break;
      case CX_CURVE_SECP256R1:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_NISTP256);
        break;
      case CX_CURVE_BrainPoolP256R1:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_BPOOLR1);
        break;
      case CX_CURVE_Ed25519:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu)," %s", LABEL_Ed25519);
        break;
      default:
        snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Choose type...");
        break;
      } 
      element->text = G_gpg_vstate.menu;
    }
  }
  return element;      
}




void  ui_menu_tmpl_set_action(unsigned int value) {
  LV(attributes,GPG_KEY_ATTRIBUTES_LENGTH);
  gpg_key_t* dest;
  const char* err;

  err = NULL;
  
  os_memset(&attributes, 0, sizeof(attributes));
  switch (G_gpg_vstate.ux_type) {
  case 2048:
  case 3072:
  case 4096:
    attributes.value[0] = 0x01;
    attributes.value[1] = (G_gpg_vstate.ux_type>>8) &0xFF;
    attributes.value[2] = G_gpg_vstate.ux_type&0xFF;
    attributes.value[3] = 0x00;
    attributes.value[4] = 0x20;
    attributes.value[5] = 0x01;
    attributes.length = 6;
    break;

  case CX_CURVE_SECP256R1:
    if (G_gpg_vstate.ux_key == 2) {
      attributes.value[0] = 18; //ecdh
    } else {
      attributes.value[0] = 19; //ecdsa
   }
   os_memmove(attributes.value+1, C_OID_SECP256R1, sizeof(C_OID_SECP256R1));
   attributes.length = 1+sizeof(C_OID_SECP256R1);
   break;

  case CX_CURVE_BrainPoolP256R1:
    if (G_gpg_vstate.ux_key == 2) {
      attributes.value[0] = 18; //ecdh
    } else {
      attributes.value[0] = 19; //ecdsa
    }
    os_memmove(attributes.value+1, C_OID_BRAINPOOL256R1, sizeof(C_OID_BRAINPOOL256R1));
    attributes.length = 1+sizeof(C_OID_BRAINPOOL256R1);
    break;

  case CX_CURVE_Ed25519: 
    if (G_gpg_vstate.ux_key == 2) {
      attributes.value[0] = 18; //ecdh
      os_memmove(attributes.value+1, C_OID_cv25519, sizeof(C_OID_cv25519));
      attributes.length = 1+sizeof(C_OID_cv25519);
    } else {
      attributes.value[0] = 22; //eddsa
      os_memmove(attributes.value+1, C_OID_Ed25519, sizeof(C_OID_Ed25519));
      attributes.length = 1+sizeof(C_OID_Ed25519);
    }
    break;

    default:
    err = TEMPLATE_TYPE;
    goto ERROR;
  }

  dest = NULL;
  switch(G_gpg_vstate.ux_key) {
  case 1:
    dest = &G_gpg_vstate.kslot->sig;
    break;
  case 2:
    dest = &G_gpg_vstate.kslot->dec;
    break;
  case 3:
    dest = &G_gpg_vstate.kslot->aut;
    break;
  default:
    err = TEMPLATE_KEY;
    goto ERROR;
  }
 
  gpg_nvm_write(dest, NULL, sizeof(gpg_key_t));
  gpg_nvm_write(&dest->attributes, &attributes, sizeof(attributes));
  ui_info(OK, NULL, ui_menu_template_display, 0);
  return;

ERROR:
  ui_info(INVALID_SELECTION, err, ui_menu_template_display, 0);

}


const ux_menu_entry_t ui_menu_tmpl_key[] = {
  {NULL,                 ui_menu_tmpl_key_action,   1,          NULL, LABEL_SIG, NULL, 0, 0},
  {NULL,                 ui_menu_tmpl_key_action,   2,          NULL, LABEL_DEC, NULL, 0, 0},
  {NULL,                 ui_menu_tmpl_key_action,   3,          NULL, LABEL_AUT, NULL, 0, 0},
  {ui_menu_template, NULL,                          0, &C_badge_back,    "Back", NULL, 61, 40},
  UX_MENU_END
};

void ui_menu_tmpl_key_action(unsigned int value) {
  G_gpg_vstate.ux_key = value;
  ui_menu_template_display(0);
}


const ux_menu_entry_t ui_menu_tmpl_type[] = {
  {NULL,             ui_menu_tmpl_type_action,   2048,                     NULL,   LABEL_RSA2048,   NULL,  0,  0},
  {NULL,             ui_menu_tmpl_type_action,   3072,                     NULL,   LABEL_RSA3072,   NULL,  0,  0},
  {NULL,             ui_menu_tmpl_type_action,   4096,                     NULL,   LABEL_RSA4096,   NULL,  0,  0},
  {NULL,             ui_menu_tmpl_type_action,   CX_CURVE_SECP256R1,       NULL,   LABEL_NISTP256,  NULL,  0,  0},
  {NULL,             ui_menu_tmpl_type_action,   CX_CURVE_BrainPoolP256R1, NULL,   LABEL_BPOOLR1,   NULL,  0,  0},  
  {NULL,             ui_menu_tmpl_type_action,   CX_CURVE_Ed25519,         NULL,   LABEL_Ed25519,   NULL,  0,  0},
  {ui_menu_template,                    NULL,    1,              &C_badge_back,   "Back",          NULL, 61, 40},
  UX_MENU_END
};

void ui_menu_tmpl_type_action(unsigned int value) {
  G_gpg_vstate.ux_type = value;
   ui_menu_template_display(1);
}

/* --------------------------------- SEED UX --------------------------------- */

const ux_menu_entry_t ui_menu_seed[] = {
  #if GPG_KEYS_SLOTS != 3
  #error menu definition not correct for current value of GPG_KEYS_SLOTS
  #endif
  {NULL,                NULL, 0, NULL,          "",        NULL, 0, 0},
  {NULL, ui_menu_seed_action, 1, NULL,          "Set on",  NULL, 0, 0},
  {NULL, ui_menu_seed_action, 0, NULL,          "Set off", NULL, 0, 0},
  {ui_menu_settings,    NULL, 1, &C_badge_back, "Back",    NULL, 61, 40},
  UX_MENU_END
};

void ui_menu_seed_display(unsigned int value) {
  UX_MENU_DISPLAY(value, ui_menu_seed, ui_menu_seed_preprocessor);
}

const bagl_element_t* ui_menu_seed_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
  if(element->component.userid==0x20) {
     if (entry == &ui_menu_seed[0]) {
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "< %s >", G_gpg_vstate.seed_mode?"ON":"OFF");
      element->text = G_gpg_vstate.menu;
     }
  }
  return element;
}

void ui_menu_seed_action(unsigned int value) {
  G_gpg_vstate.seed_mode = value;
  ui_menu_seed_display(0);
}


/* ------------------------------- PIN MODE UX ------------------------------ */
const ux_menu_entry_t ui_menu_pinmode[];
void ui_menu_pinmode_display(unsigned int value);
const bagl_element_t*  ui_menu_pinmode_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element);
void ui_menu_pinmode_action(unsigned int value);

const ux_menu_entry_t ui_menu_pinmode[] = {
  {NULL,       NULL,                    -1,                      NULL,  "Choose:",       NULL, 0, 0},
  {NULL,       ui_menu_pinmode_action,  0x8000|PIN_MODE_HOST,    NULL,  "Host",          NULL, 0, 0},
  {NULL,       ui_menu_pinmode_action,  0x8000|PIN_MODE_SCREEN,  NULL,  "On Screen",     NULL, 0, 0},
  {NULL,       ui_menu_pinmode_action,  0x8000|PIN_MODE_CONFIRM, NULL,  "Confirm only",  NULL, 0, 0},
  {NULL,       ui_menu_pinmode_action,  0x8000|PIN_MODE_TRUST,   NULL,  "Trust",         NULL, 0, 0},
  {NULL,       ui_menu_pinmode_action,  128,                     NULL,  "Set Default",   NULL, 0, 0},
  {ui_menu_settings,             NULL,    1,            &C_badge_back,  "Back",          NULL, 61, 40},
  UX_MENU_END
};

void ui_menu_pinmode_display(unsigned int value) {
   UX_MENU_DISPLAY(value, ui_menu_pinmode, ui_menu_pinmode_preprocessor);
}

const bagl_element_t*  ui_menu_pinmode_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
  if (element->component.userid==0x20) {
    if ((entry->userid >= (0x8000|PIN_MODE_HOST)) && (entry->userid<=(0x8000|PIN_MODE_TRUST))) {
      unsigned char id = entry->userid&0x7FFFF;
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "%s  %s  %s", 
               (char*)PIC(entry->line1), 
               id == N_gpg_pstate->config_pin[0] ? "#" : " ", /* default */ 
               id == G_gpg_vstate.pinmode        ? "+" : " "  /* selected*/);  
      element->text = G_gpg_vstate.menu;  
      element->component.height = 32;
    }
  }
  return element;
}

void ui_menu_pinmode_action(unsigned int value) {
  unsigned char s;
  value = value&0x7FFF;
  if (value == 128) {
    if (G_gpg_vstate.pinmode != N_gpg_pstate->config_pin[0]) {
      if (G_gpg_vstate.pinmode == PIN_MODE_TRUST) {
       ui_info(DEFAULT_MODE, NOT_ALLOWED, ui_menu_pinmode_display,0);
       return;
      }
      //set new mode
      s = G_gpg_vstate.pinmode;
      gpg_nvm_write(&N_gpg_pstate->config_pin[0], &s,1);
      //disactivate pinpad if any
      if (G_gpg_vstate.pinmode == PIN_MODE_HOST) {
        s = 0; 
      }  else {
        s = 3;
      }
      USBD_CCID_activate_pinpad(s);
    }
  }
  else {
    switch (value) {
    case PIN_MODE_HOST:
    case PIN_MODE_SCREEN:
      if (!gpg_is_pin_verified(PIN_ID_PW1)) {
        ui_info(PIN_USER, NOT_VERIFIED, ui_menu_pinmode_display,0);
       return;
      }
      break;
    
    case PIN_MODE_CONFIRM:
    case PIN_MODE_TRUST:
      if (!gpg_is_pin_verified(PIN_ID_PW3)) {
        ui_info(PIN_ADMIN, NOT_VERIFIED, ui_menu_pinmode_display,0);
        return;
      }
      break;
    default:
        ui_info(INVALID_SELECTION, NULL, ui_menu_pinmode_display,0);
        return;
    }
    G_gpg_vstate.pinmode = value;
  }
  // redisplay first entry of the idle menu
  ui_menu_pinmode_display(0);
}
/* -------------------------------- RESET UX --------------------------------- */

const ux_menu_entry_t ui_menu_reset[] = {
  #if GPG_KEYS_SLOTS != 3
  #error menu definition not correct for current value of GPG_KEYS_SLOTS
  #endif
  {NULL,   NULL,                 0, NULL,          "Really Reset ?", NULL, 0, 0},
  {NULL,   ui_menu_main_display, 0, &C_badge_back, "Oh No!",         NULL, 61, 40},
  {NULL,   ui_menu_reset_action, 0, NULL,          "Yes!",           NULL, 0, 0},
  UX_MENU_END
};

void ui_menu_reset_action(unsigned int value) {
  unsigned char magic[4];
  magic[0] = 0; magic[1] = 0; magic[2] = 0; magic[3] = 0;
  gpg_nvm_write(N_gpg_pstate->magic, magic, 4);
  gpg_init();
  ui_CCID_reset();
  ui_menu_main_display(0);
}
/* ------------------------------- SETTINGS UX ------------------------------- */

const ux_menu_entry_t ui_menu_settings[] = {
  {NULL,    ui_menu_template_display,     0, NULL,          "Key template", NULL, 0, 0},
  {NULL,        ui_menu_seed_display,     0, NULL,          "Seed mode",    NULL, 0, 0},
  {NULL,     ui_menu_pinmode_display,     0, NULL,          "PIN mode",     NULL, 0, 0},
  {ui_menu_reset,               NULL,     0, NULL,          "Reset",        NULL, 0, 0},
  {NULL,        ui_menu_main_display,     2, &C_badge_back, "Back",         NULL, 61, 40},
  UX_MENU_END
};


/* --------------------------------- SLOT UX --------------------------------- */

#if GPG_KEYS_SLOTS != 3
#error menu definition not correct for current value of GPG_KEYS_SLOTS
#endif

const ux_menu_entry_t ui_menu_slot[] = {
  {NULL,             NULL,                     -1, NULL,          "Choose:",     NULL, 0, 0},
  {NULL,             ui_menu_slot_action,   1, NULL,          "",            NULL, 0, 0},
  {NULL,             ui_menu_slot_action,   2, NULL,          "",            NULL, 0, 0},
  {NULL,             ui_menu_slot_action,   3, NULL,          "",            NULL, 0, 0},
  {NULL,             ui_menu_slot_action, 128, NULL,          "Set Default", NULL, 0, 0},
  {NULL,            ui_menu_main_display,   1, &C_badge_back, "Back",        NULL, 61, 40},
  UX_MENU_END
};
void ui_menu_slot_display(unsigned int value) {
   UX_MENU_DISPLAY(value, ui_menu_slot, ui_menu_slot_preprocessor);
}


const bagl_element_t*  ui_menu_slot_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
  unsigned int slot;
  if(element->component.userid==0x20) {
    for (slot = 1; slot <= 3; slot ++) {
      if (entry == &ui_menu_slot[slot]) {
        break;
      }
    }
    if (slot != 4) {
      snprintf(G_gpg_vstate.menu, sizeof(G_gpg_vstate.menu), "Slot %d  %s  %s", 
              slot, 
              slot == N_gpg_pstate->config_slot[1]+1?"#":" ", /* default */ 
              slot == G_gpg_vstate.slot+1?"+":" "            /* selected*/);
      element->text = G_gpg_vstate.menu;
    }
  }
  return element;
}
void ui_menu_slot_action(unsigned int value) {
  unsigned char s;

  if (value == 128) {
    s = G_gpg_vstate.slot;
    gpg_nvm_write(&N_gpg_pstate->config_slot[1], &s,1);
    value = s+1;  
  }
  else {
     s = (unsigned char)(value-1);
     if (s!= G_gpg_vstate.slot) {
       G_gpg_vstate.slot = s;
       G_gpg_vstate.kslot   = &N_gpg_pstate->keys[G_gpg_vstate.slot];
       ui_CCID_reset();
     }
  }
  // redisplay first entry of the idle menu
  ui_menu_slot_display(value);
}
/* --------------------------------- INFO UX --------------------------------- */

#if GPG_KEYS_SLOTS != 3
#error menu definition not correct for current value of GPG_KEYS_SLOTS
#endif

#define STR(x)  #x
#define XSTR(x) STR(x)

const ux_menu_entry_t ui_menu_info[] = {
  {NULL,  NULL,                 -1, NULL,          "OpenPGP Card",             NULL, 0, 0},
  {NULL,  NULL,                 -1, NULL,          "(c) Ledger SAS",           NULL, 0, 0},
  {NULL,  NULL,                 -1, NULL,          "Spec  3.0",                NULL, 0, 0},
  {NULL,  NULL,                 -1, NULL,          "App  " XSTR(GPG_VERSION),  NULL, 0, 0},
  {NULL,  ui_menu_main_display,  3, &C_badge_back, "Back",                     NULL, 61, 40},
  UX_MENU_END
};

#undef STR
#undef XSTR

/* --------------------------------- MAIN UX --------------------------------- */

const ux_menu_entry_t ui_menu_main[] = {
  {NULL,                       NULL,  0, NULL,              "",            NULL, 0, 0},
  {NULL,       ui_menu_slot_display,  0, NULL,              "Select slot", NULL, 0, 0},
  {ui_menu_settings,           NULL,  0, NULL,              "Settings",    NULL, 0, 0},
  {ui_menu_info,               NULL,  0, NULL,              "About",       NULL, 0, 0},
  {NULL,              os_sched_exit,  0, &C_icon_dashboard, "Quit app" ,   NULL, 50, 29},
  UX_MENU_END
};
extern const  uint8_t N_USBD_CfgDesc[];
const bagl_element_t* ui_menu_main_preprocessor(const ux_menu_entry_t* entry, bagl_element_t* element) {
  if (entry == &ui_menu_main[0]) {
    if(element->component.userid==0x20) {      
      unsigned int serial;
      char name[20];
      
      serial = MIN(N_gpg_pstate->name.length,19);
      os_memset(name, 0, 20);
      os_memmove(name, N_gpg_pstate->name.value, serial);
      serial = (N_gpg_pstate->AID[10] << 24) |
               (N_gpg_pstate->AID[11] << 16) |
               (N_gpg_pstate->AID[12] <<  8) |
               (N_gpg_pstate->AID[13] |(G_gpg_vstate.slot+1));
      os_memset(G_gpg_vstate.menu, 0, sizeof(G_gpg_vstate.menu));
      snprintf(G_gpg_vstate.menu,  sizeof(G_gpg_vstate.menu), "< User: %s / SLOT: %d / Serial: %x >", 
               name, G_gpg_vstate.slot+1, serial);
      
      element->component.stroke = 10; // 1 second stop in each way
      element->component.icon_id = 26; // roundtrip speed in pixel/s
      element->text = G_gpg_vstate.menu;
      UX_CALLBACK_SET_INTERVAL(bagl_label_roundtrip_duration_ms(element, 7));
    }
  }
  return element;
}
void ui_menu_main_display(unsigned int value) {
   UX_MENU_DISPLAY(value, ui_menu_main, ui_menu_main_preprocessor);
}

void ui_init(void) {
 ui_menu_main_display(0);
 // setup the first screen changing
  UX_CALLBACK_SET_INTERVAL(1000);
}

void io_seproxyhal_display(const bagl_element_t *element) {
  io_seproxyhal_display_default((bagl_element_t *)element);
}
