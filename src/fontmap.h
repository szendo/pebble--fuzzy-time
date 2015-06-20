#pragma once
 
typedef enum {
  BOLD_TYPE,
  LIGHT_TYPE
} FontType;

void init_fonts();
void deinit_fonts();
void draw_text(GContext *ctx, char* text, FontType font_type);
