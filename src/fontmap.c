#include <pebble.h>
#include "fontmap.h"

typedef struct {
  int16_t width;
  int16_t kern_begin;
  int16_t kern_end;
  int16_t load_offset;
  GBitmap *bitmap;
} LetterData;

typedef struct {
  int32_t res_id;
  GBitmap *bitmap;
  int16_t height;
  int16_t space_width;
  LetterData letters[];
} FontData;

static FontData s_roboto_bold_params = {
  .res_id = RESOURCE_ID_IMAGE_ROBOTO_BOLD,
  .bitmap = NULL,
  .height = 41,
  .space_width = 10,
  .letters = {
    {23,0,3,3, NULL}, // a
    {24,0,3,3, NULL}, // b
    {22,0,3,3, NULL}, // c
    {24,0,3,3, NULL}, // d
    {23,0,3,3, NULL}, // e
    {16,0,4,3, NULL}, // f
    {24,0,3,3, NULL}, // g
    {24,0,3,3, NULL}, // h
    {11,0,3,3, NULL}, // i
    {13,2,3,5, NULL}, // j
    {23,0,4,4, NULL}, // k
    {11,0,3,5, NULL}, // l (+2 offset: midnight)
    {34,0,3,3, NULL}, // m (-2 width: midnight)
    {24,0,3,3, NULL}, // n
    {24,0,3,3, NULL}, // o
    {24,0,3,3, NULL}, // p
    {24,0,3,3, NULL}, // q
    {15,0,3,3, NULL}, // r
    {22,0,3,3, NULL}, // s
    {14,0,3,3, NULL}, // t
    {24,0,3,3, NULL}, // u
    {21,0,3,3, NULL}, // v
    {31,0,3,3, NULL}, // w
    {22,0,3,3, NULL}, // x
    {21,1,3,4, NULL}, // y
    {21,0,3,3, NULL}, // z
    {10,0,3,3, NULL}  // '
  }
};

static FontData s_roboto_light_params = {
  .res_id = RESOURCE_ID_IMAGE_ROBOTO_LIGHT,
  .bitmap = NULL,
  .height = 41,
  .space_width = 8,
  .letters = {
    {23,0,2,2, NULL}, // a
    {23,0,2,2, NULL}, // b
    {22,0,2,2, NULL}, // c
    {23,0,2,2, NULL}, // d
    {22,0,2,2, NULL}, // e
    {15,0,3,2, NULL}, // f
    {23,0,2,2, NULL}, // g
    {23,0,2,2, NULL}, // h
    { 9,0,2,4, NULL}, // i
    {12,2,2,2, NULL}, // j
    {21,0,2,2, NULL}, // k
    { 9,0,2,2, NULL}, // l
    {37,0,2,2, NULL}, // m
    {23,0,2,2, NULL}, // n
    {24,0,2,2, NULL}, // o
    {23,0,2,2, NULL}, // p
    {23,0,2,2, NULL}, // q
    {14,0,2,2, NULL}, // r
    {21,0,2,2, NULL}, // s
    {14,0,2,2, NULL}, // t
    {23,0,2,2, NULL}, // u
    {20,0,2,2, NULL}, // v
    {32,0,2,2, NULL}, // w
    {20,0,2,2, NULL}, // x
    {20,0,2,2, NULL}, // y
    {20,0,2,2, NULL}, // z
    { 7,0,2,2, NULL}  // '
  }
};

static void init_font(FontData *font_data) {
  font_data->bitmap = gbitmap_create_with_resource(font_data->res_id);
  int16_t offset = 0;
  for (int i = 0; i < 27; i++) {
    LetterData *letter = &(font_data->letters[i]);
    letter->bitmap = gbitmap_create_as_sub_bitmap(font_data->bitmap, GRect(offset - letter->kern_begin, 0, letter->width, font_data->height));
    offset += letter->width -letter->kern_end - letter->kern_begin + letter->load_offset;
  }
}

static void deinit_font(FontData *font_data) {
  if (font_data->bitmap != NULL) {
    for(int i = 0; i < 27; i++) {
      LetterData *letter = &(font_data->letters[i]);
      if (letter->bitmap != NULL) {
        gbitmap_destroy(letter->bitmap);
        letter->bitmap = NULL;
      }
    }
    gbitmap_destroy(font_data->bitmap);
    font_data->bitmap = NULL;
  }
}

void init_fonts() {
  init_font(&s_roboto_bold_params);
  init_font(&s_roboto_light_params);
}

void deinit_fonts() {
  deinit_font(&s_roboto_bold_params);
  deinit_font(&s_roboto_light_params);
}

void draw_text(GContext *ctx, char* text, FontType font_type, GPoint draw_offset) {
  FontData *font_data = ((font_type == BOLD_TYPE) ? (&s_roboto_bold_params) : (&s_roboto_light_params));
  int16_t offset = draw_offset.x;
  for (char *t = text; *t != '\0'; t++) {
    if (*t >= 'a' && *t <= 'z') {
      LetterData *letter = &(font_data->letters[*t - 'a']);
      graphics_draw_bitmap_in_rect(ctx, letter->bitmap, GRect(offset - letter->kern_begin, draw_offset.y, letter->width, font_data->height));
      offset += letter->width - letter->kern_end;
    } else if (*t == '\'') {
      LetterData *letter = &(font_data->letters[26]);
      graphics_draw_bitmap_in_rect(ctx, letter->bitmap, GRect(offset - letter->kern_begin, draw_offset.y, letter->width, font_data->height));
      offset += letter->width - letter->kern_end;
    } else {
      offset += font_data->space_width;
    }
  }
}

int16_t measure_text(char* text, FontType font_type) {
  FontData *font_data = ((font_type == BOLD_TYPE) ? (&s_roboto_bold_params) : (&s_roboto_light_params));
  int16_t offset = 0;
  int16_t last_kern_end = 0;
  for (char *t = text; *t != '\0'; t++) {
    if (*t >= 'a' && *t <= 'z') {
      LetterData *letter = &(font_data->letters[*t - 'a']);
      offset += letter->width - letter->kern_end;
      last_kern_end = letter->kern_end;
    } else if (*t == '\'') {
      LetterData *letter = &(font_data->letters[26]);
      offset += letter->width - letter->kern_end;
      last_kern_end = letter->kern_end;
    } else {
      offset += font_data->space_width;
      last_kern_end = 0;
    }
  }
  return offset + last_kern_end;
}
