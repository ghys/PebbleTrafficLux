#include <pebble.h>
#include "webcam.h"

static Window *window;

static BitmapLayer  *image_layer;
static GBitmap      *image = NULL;
static uint8_t      *data_image = NULL;
static uint32_t     data_size;

static TextLayer    *text_layer;

#define KEY_IMAGE   0
#define KEY_INDEX   1
#define KEY_MESSAGE 2
#define KEY_SIZE    3

#define CHUNK_SIZE 1500

static void cb_in_received_handler(DictionaryIterator *iter, void *context) {
	// Get the bitmap

	Tuple *size_tuple = dict_find(iter, KEY_SIZE);
	if (size_tuple){
		if (data_image) {
			free(data_image);
			data_image = NULL;
		}
		data_size = size_tuple->value->uint32;
		data_image = malloc(data_size);
	}

	Tuple *image_tuple = dict_find(iter, KEY_IMAGE);
	Tuple *index_tuple = dict_find(iter, KEY_INDEX);
	if (index_tuple && image_tuple) {
		int32_t index = index_tuple->value->int32;

		APP_LOG(APP_LOG_LEVEL_DEBUG, "image received index=%ld size=%d", index, image_tuple->length);
		memcpy(data_image + index, &image_tuple->value->uint8, image_tuple->length);

		if (image_tuple->length < CHUNK_SIZE){
			if (image){
				gbitmap_destroy(image);
				image = NULL;
			}
#ifdef PBL_COLOR
			image = gbitmap_create_from_png_data(data_image, data_size);
#else
			image = gbitmap_create_with_data(data_image);
#endif
			bitmap_layer_set_bitmap(image_layer, image);
			text_layer_set_text(text_layer, "");
			layer_mark_dirty(bitmap_layer_get_layer(image_layer));
		}
	}

	Tuple *message_tuple = dict_find(iter, KEY_MESSAGE);
	if (message_tuple){
		text_layer_set_text(text_layer, message_tuple->value->cstring);
	}
}

static void webcam_app_message_init() {
	// Register message handlers
	app_message_register_inbox_received(cb_in_received_handler);
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (image){
		gbitmap_destroy(image);
		image = NULL;
		bitmap_layer_set_bitmap(image_layer, image);
	}

	text_layer_set_text(text_layer, "Updating image...");

	DictionaryIterator *iter;
	uint8_t value = 1;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_IMAGE, &value, 1, true);
	dict_write_end(iter);
	app_message_outbox_send();
}

static void click_config_provider(void *context) {
	window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	image_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

	text_layer = text_layer_create(GRect(0, bounds.size.h - 16, bounds.size.w, 16));
	text_layer_set_text(text_layer, "Press select");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));
}

static void window_unload(Window *window) {
	text_layer_destroy(text_layer);
	bitmap_layer_destroy(image_layer);
	if (image){
		gbitmap_destroy(image);
    image = NULL;
	}
	if (data_image){
		free(data_image);
		data_image = NULL;
	}
  
  window_destroy(window);
}




void init_webcam(void) {
	webcam_app_message_init();
	window = window_create();
	window_set_click_config_provider(window, click_config_provider);
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
			.unload = window_unload,
	});
#ifdef PBL_SDK_2
	window_set_fullscreen(window, true);
#endif
	window_stack_push(window, true);
}


