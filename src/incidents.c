#include <pebble.h>
#include "incidents.h"

static Window *incidents;


static TextLayer    *incidents_loading_layer;


#define KEY_INCIDENTS_REQ   10
#define KEY_INCIDENTS_SIZE  11

#define KEY_INCIDENTS_FIRST_

static void cb_incidents_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incidents app message callback");
  
 	Tuple *size_tuple = dict_find(iter, KEY_INCIDENTS_SIZE);
  int size = size_tuple->value->int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "size=%d", size);
}

static void incidents_load(Window *window) {

	Layer *window_layer = window_get_root_layer(incidents);
	GRect bounds = layer_get_bounds(window_layer);
  
	incidents_loading_layer = text_layer_create(GRect(0, bounds.size.h - 16, bounds.size.w, 16));
	text_layer_set_text(incidents_loading_layer, "Loading...");
	text_layer_set_font(incidents_loading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(incidents_loading_layer, GTextAlignmentCenter);
	text_layer_set_background_color(incidents_loading_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(incidents_loading_layer));
  
	DictionaryIterator *iter;
	uint8_t value = 1;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_INCIDENTS_REQ, &value, 1, true);
	dict_write_end(iter);
	app_message_outbox_send();

}

static void incidents_unload(Window *window) {
	text_layer_destroy(incidents_loading_layer);
  
  app_message_register_inbox_received(NULL);
}


void init_incidents(void) {
	// Register message handlers
	app_message_register_inbox_received(cb_incidents_received_handler);
	// Init buffers
	app_message_open(app_message_inbox_size_maximum(), APP_MESSAGE_OUTBOX_SIZE_MINIMUM);

  incidents = window_create();
	window_set_window_handlers(incidents, (WindowHandlers) {
		.load = incidents_load,
	  .unload = incidents_unload,
	});

	window_stack_push(incidents, true);
}

