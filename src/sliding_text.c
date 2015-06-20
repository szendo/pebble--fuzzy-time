#include <pebble.h>
#include "num2words.h"
#include "fontmap.h"

typedef enum {
  MOVING_IN,
  IN_FRAME,
  PREPARE_TO_MOVE,
  MOVING_OUT
} SlideState;

typedef struct {
  char *text;
  FontType type;
} RowData;

typedef struct {
  Layer *layer;
  SlideState state; // animation state
  char *next_string; // what to say in the next phase of animation
  FontType next_type;

  int left_pos;
  int right_pos;
  int still_pos;

  int movement_delay;
  int delay_count;
} SlidingRow;

typedef struct {
  SlidingRow rows[3];
  int last_minute;

  Window *window;
  Animation *animation;

  struct SlidingTextRenderState {
    // double buffered string storage
    char line0[2][32];
    char line1[2][32];
    char line2[2][32];
    uint8_t next_line;
  } render_state;

} SlidingTextData;

SlidingTextData *s_data;

static void update_proc_sliding_row(Layer *layer, GContext *ctx) {
  RowData *row_data = (RowData*)layer_get_data(layer);
  if (row_data && row_data->text) {
    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    draw_text(ctx, row_data->text, row_data->type);  
  }
}

static void init_sliding_row(SlidingTextData *data, SlidingRow *row, GRect pos, int delay) {
  row->layer = layer_create_with_data(pos, sizeof(RowData));
  layer_set_update_proc(row->layer, update_proc_sliding_row);
  RowData* row_data = (RowData*)layer_get_data(row->layer);
  row_data->text = NULL;
  row_data->type = LIGHT_TYPE;

  row->state = IN_FRAME;
  row->next_string = NULL;
  row->next_type = LIGHT_TYPE;

  row->left_pos = -pos.size.w;
  row->right_pos = pos.size.w;
  row->still_pos = pos.origin.x;

  row->movement_delay = delay;
  row->delay_count = 0;

  data->last_minute = -1;
}

static void slide_in_text(SlidingTextData *data, SlidingRow *row, char* new_text, FontType new_type) {
  (void) data;

  RowData *row_data = (RowData*)layer_get_data(row->layer);
  if (row_data->text) {
    row->next_string = new_text;
    row->next_type = new_type;
    row->state = PREPARE_TO_MOVE;
  } else {
    row_data->text = new_text;
    row_data->type = new_type;
    GRect frame = layer_get_frame(row->layer);
    frame.origin.x = row->right_pos;
    layer_set_frame(row->layer, frame);
    row->state = MOVING_IN;
  }
}


static bool update_sliding_row(SlidingTextData *data, SlidingRow *row) {

  GRect frame = layer_get_frame(row->layer);
  bool something_changed = true;
  switch (row->state) {
    case PREPARE_TO_MOVE:
      frame.origin.x = row->still_pos;
      row->delay_count++;
      if (row->delay_count > row->movement_delay) {
        row->state = MOVING_OUT;
        row->delay_count = 0;
      }
    break;

    case MOVING_IN: {
      int speed = abs(frame.origin.x - row->still_pos) / 3 + 1;
      frame.origin.x -= speed;
      if (frame.origin.x <= row->still_pos) {
        frame.origin.x = row->still_pos;
        row->state = IN_FRAME;
      }
    }
    break;

    case MOVING_OUT: {
      int speed = abs(frame.origin.x - row->still_pos) / 3 + 1;
      frame.origin.x -= speed;

      if (frame.origin.x <= row->left_pos) {
        frame.origin.x = row->right_pos;
        row->state = MOVING_IN;
        RowData *row_data = (RowData*)layer_get_data(row->layer);
        row_data->text = row->next_string;
        row_data->type = row->next_type;
        row->next_string = NULL;
      }
    }
    break;

    case IN_FRAME:
    default:
      something_changed = false;
      break;
  }
  if (something_changed) {
    layer_set_frame(row->layer, frame);
  }
  return something_changed;
}

static void animation_update(struct Animation *animation, const AnimationProgress time_normalized) {
  SlidingTextData *data = s_data;

  struct SlidingTextRenderState *rs = &data->render_state;

  time_t now = time(NULL);
  struct tm t = *localtime(&now);
  
  int ch = t.tm_hour, cm = t.tm_min;
  
  fuzz_time(&ch, &cm);

  bool something_changed = false;

  if (data->last_minute == -1) {
    something_changed = true;
    
    fuzzy_time_to_words(ch, cm, (char*[3]){rs->line0[rs->next_line], rs->line1[rs->next_line], rs->line2[rs->next_line]});
    slide_in_text(data, &data->rows[0], rs->line0[rs->next_line], cm==0 ? BOLD_TYPE : LIGHT_TYPE);
    slide_in_text(data, &data->rows[1], rs->line1[rs->next_line], LIGHT_TYPE);
    slide_in_text(data, &data->rows[2], rs->line2[rs->next_line], cm==0 ? LIGHT_TYPE : BOLD_TYPE);
    
    rs->next_line = rs->next_line ? 0 : 1;
    data->last_minute = cm;
  } else if (data->last_minute != cm) {
    something_changed = true;
    
    fuzzy_time_to_words(ch, cm, (char*[3]){rs->line0[rs->next_line], rs->line1[rs->next_line], rs->line2[rs->next_line]});

    if (cm != 25 && cm != 40) {
      slide_in_text(data, &data->rows[0], rs->line0[rs->next_line], cm==0 ? BOLD_TYPE : LIGHT_TYPE);
    } else {
      ((RowData*)layer_get_data(data->rows[0].layer))->text = rs->line0[rs->next_line];
    }
    
    if (cm == 0 || cm == 5 || cm == 35) {
      slide_in_text(data, &data->rows[1], rs->line1[rs->next_line], LIGHT_TYPE);
      slide_in_text(data, &data->rows[2], rs->line2[rs->next_line], cm==0 ? LIGHT_TYPE : BOLD_TYPE);
    } else if (cm == 25 || cm == 30 || cm == 40) {
      slide_in_text(data, &data->rows[1], rs->line1[rs->next_line], LIGHT_TYPE);
      ((RowData*)layer_get_data(data->rows[2].layer))->text = rs->line2[rs->next_line];
    } else {
      ((RowData*)layer_get_data(data->rows[1].layer))->text = rs->line1[rs->next_line];
      ((RowData*)layer_get_data(data->rows[2].layer))->text = rs->line2[rs->next_line];      
    }
    rs->next_line = rs->next_line ? 0 : 1;
    data->last_minute = cm;
  }

  for (size_t i = 0; i < ARRAY_LENGTH(data->rows); ++i) {
    something_changed = update_sliding_row(data, &data->rows[i]) || something_changed;
  }

  if (!something_changed) {
    animation_unschedule(data->animation);
  }
}

static void make_animation() {
  s_data->animation = animation_create();
  animation_set_duration(s_data->animation, ANIMATION_DURATION_INFINITE);
                  // the animation will stop itself
  static const struct AnimationImplementation s_animation_implementation = {
    .update = animation_update,
  };
  animation_set_implementation(s_data->animation, &s_animation_implementation);
  animation_schedule(s_data->animation);
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
  if (tick_time->tm_min % 5 == 3) {
    make_animation();
  }
}

static void handle_deinit(void) {
  deinit_fonts();

  tick_timer_service_unsubscribe();
  free(s_data);
}

static void handle_init() {
  init_fonts();
  
  SlidingTextData *data = (SlidingTextData*)malloc(sizeof(SlidingTextData));
  s_data = data;

  data->render_state.next_line = 0;

  data->window = window_create();

  window_set_background_color(data->window, GColorBlack);

  Layer *window_layer = window_get_root_layer(data->window);

  const int16_t width = layer_get_frame(window_layer).size.w;
  init_sliding_row(data, &data->rows[0], GRect(0, 27, width, 41), 0);
  layer_add_child(window_layer, data->rows[0].layer);

  init_sliding_row(data, &data->rows[1], GRect(0, 63, width, 41), 3);
  layer_add_child(window_layer, data->rows[1].layer);

  init_sliding_row(data, &data->rows[2], GRect(0, 99, width, 41), 6);
  layer_add_child(window_layer, data->rows[2].layer);

  layer_mark_dirty(window_layer);

  make_animation();

  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);

  window_stack_push(data->window, true);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
