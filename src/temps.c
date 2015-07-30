#include <pebble.h>
#include "temps.h"

static Window *traveltimes;

static Window *traveltime_details;

static TextLayer    *traveltimes_loading_layer;

static int nb_traveltimes = 0;
static int current_traveltime_idx = 0;
static ScrollLayer *traveltime_details_scroll_layer;
static TextLayer *traveltime_details_text_layer;
static TextLayer *traveltime_from_text_layer;
static TextLayer *traveltime_name_text_layer;
static TextLayer *traveltime_bkg_text_layer;

static SimpleMenuItem *traveltimes_menu_items = NULL;
static SimpleMenuSection traveltimes_menu_section;


#define KEY_TRAVELTIMES_REQ   20
#define KEY_TRAVELTIMES_SIZE  21

#define KEY_FIRST_TRAVELTIMES_NAME   3000
#define KEY_FIRST_TRAVELTIMES_TIMES  4000



// Setup the scroll layer on window load
// We do this here in order to be able to get the max used text size
static void traveltime_details_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect max_text_bounds = GRect(10, 90, bounds.size.w - 20, 60);
  GRect from_text_bounds = GRect(10, 10, bounds.size.w - 20, 30);
  GRect name_text_bounds = GRect(10, 30, bounds.size.w - 20, 40);
  GRect bkg_text_bounds = GRect(0, 70, bounds.size.w, bounds.size.h);
  
  // Initialize the scroll layer
  traveltime_details_scroll_layer = scroll_layer_create(bounds);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(traveltime_details_scroll_layer, window);

  // Initialize the text layers
  traveltime_from_text_layer = text_layer_create(from_text_bounds);
  text_layer_set_text(traveltime_from_text_layer, "Depuis");
  text_layer_set_font(traveltime_from_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  traveltime_name_text_layer = text_layer_create(name_text_bounds);
  text_layer_set_text(traveltime_name_text_layer, traveltimes_menu_items[current_traveltime_idx].title);
  text_layer_set_font(traveltime_name_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  
  traveltime_details_text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(traveltime_details_text_layer, traveltimes_menu_items[current_traveltime_idx].subtitle);
  text_layer_set_font(traveltime_details_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(traveltime_details_text_layer);
  //text_layer_set_size(traveltime_details_text_layer, max_size);
  scroll_layer_set_content_size(traveltime_details_scroll_layer, GSize(bounds.size.w, max_size.h + 104));

  traveltime_bkg_text_layer = text_layer_create(bkg_text_bounds);
  
#ifdef PBL_COLOR
  text_layer_set_background_color(traveltime_bkg_text_layer, GColorBlack);
  text_layer_set_background_color(traveltime_from_text_layer, GColorIslamicGreen);
  text_layer_set_background_color(traveltime_name_text_layer, GColorIslamicGreen);
  text_layer_set_background_color(traveltime_details_text_layer, GColorBlack);
  text_layer_set_text_color(traveltime_from_text_layer, GColorWhite);
  text_layer_set_text_color(traveltime_name_text_layer, GColorWhite);
  text_layer_set_text_color(traveltime_details_text_layer, GColorYellow);
#endif
  
  // Add the layers for display
  scroll_layer_add_child(traveltime_details_scroll_layer, text_layer_get_layer(traveltime_bkg_text_layer));
  scroll_layer_add_child(traveltime_details_scroll_layer, text_layer_get_layer(traveltime_from_text_layer));
  scroll_layer_add_child(traveltime_details_scroll_layer, text_layer_get_layer(traveltime_name_text_layer));
  scroll_layer_add_child(traveltime_details_scroll_layer, text_layer_get_layer(traveltime_details_text_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(traveltime_details_scroll_layer));
}

static void traveltime_details_window_unload(Window *window) {
  text_layer_destroy(traveltime_bkg_text_layer);
  text_layer_destroy(traveltime_details_text_layer);
  text_layer_destroy(traveltime_from_text_layer);
  text_layer_destroy(traveltime_name_text_layer);
  scroll_layer_destroy(traveltime_details_scroll_layer);
}

static void show_traveltime_details(int index, void *context) {
  current_traveltime_idx = index;
  traveltime_details = window_create();
#ifdef PBL_COLOR
  window_set_background_color(traveltime_details, GColorIslamicGreen);
#endif
	window_set_window_handlers(traveltime_details, (WindowHandlers) {
		.load = traveltime_details_window_load,
	  .unload = traveltime_details_window_unload,
	});

	window_stack_push(traveltime_details, true);
}

static void cb_traveltimes_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "traveltimes app message callback");
  
 	Tuple *size_tuple = dict_find(iter, KEY_TRAVELTIMES_SIZE);
  if (!size_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "no KEY_TRAVELTIMES_SIZE in dict?");
    nb_traveltimes = 0;
    
    return;
  }
  
  nb_traveltimes = size_tuple->value->int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "size=%d", nb_traveltimes);

  traveltimes_menu_items = malloc(nb_traveltimes * sizeof(SimpleMenuItem));
  
  for (int i = 0; i < nb_traveltimes; i++) {
    Tuple *message_tuple = dict_find(iter, i + KEY_FIRST_TRAVELTIMES_NAME);
    Tuple *times_tuple = dict_find(iter, i + KEY_FIRST_TRAVELTIMES_TIMES);
    //char *message = malloc(strlen(message_tuple->value->cstring)+1);
    //urldecode2(message, message_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "name=%s", message_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "times=%s", times_tuple->value->cstring);
    traveltimes_menu_items[i].title =  message_tuple->value->cstring;
    traveltimes_menu_items[i].subtitle = times_tuple->value->cstring;
    traveltimes_menu_items[i].icon = NULL;
    traveltimes_menu_items[i].callback = show_traveltime_details;
    //free(message);
  }
  
  traveltimes_menu_section = (SimpleMenuSection) {
    "Temps de parcours", traveltimes_menu_items, nb_traveltimes
  };
  
	Layer *window_layer = window_get_root_layer(traveltimes);
	GRect bounds = layer_get_bounds(window_layer);
  
	SimpleMenuLayer *menu = simple_menu_layer_create(bounds, traveltimes, &traveltimes_menu_section, 1, NULL);
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(menu), GColorIslamicGreen, GColorWhite);
#endif
	layer_add_child(window_layer, (Layer *)menu);
  
}

static void traveltimes_load(Window *window) {

	Layer *window_layer = window_get_root_layer(traveltimes);
	GRect bounds = layer_get_bounds(window_layer);
  
	traveltimes_loading_layer = text_layer_create(GRect(0, bounds.size.h - 16, bounds.size.w, 16));
	text_layer_set_text(traveltimes_loading_layer, "Loading...");
	text_layer_set_font(traveltimes_loading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(traveltimes_loading_layer, GTextAlignmentCenter);
	text_layer_set_background_color(traveltimes_loading_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(traveltimes_loading_layer));
  
	DictionaryIterator *iter;
	uint8_t value = 1;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_TRAVELTIMES_REQ, &value, 1, true);
	dict_write_end(iter);
	app_message_outbox_send();

}

static void traveltimes_unload(Window *window) {
	text_layer_destroy(traveltimes_loading_layer);
  if (traveltimes_menu_items) {
    free(traveltimes_menu_items);
  }
  app_message_register_inbox_received(NULL);
}


void init_traveltimes(void) {
	// Register message handlers
	app_message_register_inbox_received(cb_traveltimes_received_handler);

  traveltimes = window_create();
	window_set_window_handlers(traveltimes, (WindowHandlers) {
		.load = traveltimes_load,
	  .unload = traveltimes_unload,
	});

	window_stack_push(traveltimes, true);
}
