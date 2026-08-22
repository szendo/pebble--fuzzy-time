#pragma once
#include <stdint.h>

typedef void(* ConfigChanged)();

void config_open();
void config_close();
ConfigChanged config_register_changed(ConfigChanged);

uint8_t  config_get_text_align();