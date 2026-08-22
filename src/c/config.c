#include <pebble.h>
#include "config.h"

#define STORAGE_KEY_TEXT_ALIGN 0

static ConfigChanged change_callback = NULL;

static void app_message_inbox_received(DictionaryIterator *iterator, void *context) {
  Tuple *next_tuple = dict_read_first(iterator);
  while (NULL != next_tuple) {
    if (next_tuple->key == MESSAGE_KEY_TEXT_ALIGN) {
      persist_write_int(STORAGE_KEY_TEXT_ALIGN, atoi(next_tuple->value->cstring));
    }
    next_tuple = dict_read_next(iterator);
  }
  change_callback();
}

void config_open() {
  app_message_register_inbox_received(app_message_inbox_received);
  app_message_open(APP_MESSAGE_INBOX_SIZE_MINIMUM, APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
}

void config_close() {
  app_message_deregister_callbacks();
}

ConfigChanged config_register_changed(ConfigChanged callback) {
  ConfigChanged old_callback = change_callback;
  change_callback = callback;
  return old_callback;
}

uint8_t config_get_text_align() {
  // persist_read_int() returns 0 when not set
  return persist_read_int(STORAGE_KEY_TEXT_ALIGN);
}
