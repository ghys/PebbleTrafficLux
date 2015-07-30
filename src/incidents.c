#include <pebble.h>
#include <stdlib.h>
#include <ctype.h>
#include "incidents.h"

static Window *incidents;

static Window *incident_details;

static TextLayer    *incidents_loading_layer;

static int nb_incidents = 0;
static int current_incident_idx = 0;
static ScrollLayer *incident_details_scroll_layer;
static TextLayer *incident_details_text_layer;
static TextLayer *incident_details_date_layer;

static SimpleMenuItem *incidents_menu_items = NULL;
static SimpleMenuSection incidents_menu_section;


#define KEY_INCIDENTS_REQ   10
#define KEY_INCIDENTS_SIZE  11

#define KEY_INCIDENTS_FIRST_MESSAGE  1000
#define KEY_INCIDENTS_FIRST_DATE     2000


static void urldecode2(char *dst, const char *src)
{
        char a, b;
        while (*src) {
                if ((*src == '%') &&
                    ((a = src[1]) && (b = src[2])) &&
                    (isxdigit((int)a) && isxdigit((int)b))) {
                        if (a >= 'a')
                                a -= 'a'-'A';
                        if (a >= 'A')
                                a -= ('A' - 10);
                        else
                                a -= '0';
                        if (b >= 'a')
                                b -= 'a'-'A';
                        if (b >= 'A')
                                b -= ('A' - 10);
                        else
                                b -= '0';
                        *dst++ = 16*a+b;
                        src+=3;
                } else {
                        *dst++ = *src++;
                }
        }
        *dst++ = '\0';
}

// Setup the scroll layer on window load
// We do this here in order to be able to get the max used text size
static void incident_details_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);
  GRect date_text_bounds = GRect(0, 0, bounds.size.w, 17);
  GRect max_text_bounds = GRect(10, 20, bounds.size.w - 20, 2000);

  // Initialize the scroll layer
  incident_details_scroll_layer = scroll_layer_create(bounds);

  // This binds the scroll layer to the window so that up and down map to scrolling
  // You may use scroll_layer_set_callbacks to add or override interactivity
  scroll_layer_set_click_config_onto_window(incident_details_scroll_layer, window);

  // Initialize the text layers
  incident_details_text_layer = text_layer_create(max_text_bounds);
  text_layer_set_text(incident_details_text_layer, incidents_menu_items[current_incident_idx].title);
  incident_details_date_layer = text_layer_create(date_text_bounds);
  text_layer_set_text(incident_details_date_layer, incidents_menu_items[current_incident_idx].subtitle);

  // Change the fonts
  text_layer_set_font(incident_details_text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_font(incident_details_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));

#ifdef PBL_COLOR
  text_layer_set_background_color(incident_details_text_layer, GColorMelon);
  text_layer_set_background_color(incident_details_date_layer, GColorRed);
  text_layer_set_text_color(incident_details_date_layer, GColorWhite);
  text_layer_set_text_alignment(incident_details_date_layer, GTextAlignmentCenter);
#endif
  
  // Trim text layer and scroll content to fit text box
  GSize max_size = text_layer_get_content_size(incident_details_text_layer);
  text_layer_set_size(incident_details_text_layer, max_size);
  scroll_layer_set_content_size(incident_details_scroll_layer, GSize(bounds.size.w, max_size.h + 45));

  // Add the layers for display
  scroll_layer_add_child(incident_details_scroll_layer, text_layer_get_layer(incident_details_text_layer));
  scroll_layer_add_child(incident_details_scroll_layer, text_layer_get_layer(incident_details_date_layer));

  layer_add_child(window_layer, scroll_layer_get_layer(incident_details_scroll_layer));
}

static void incident_details_window_unload(Window *window) {
  text_layer_destroy(incident_details_text_layer);
  text_layer_destroy(incident_details_date_layer);
  scroll_layer_destroy(incident_details_scroll_layer);
}

static void show_incident_details(int index, void *context) {
  current_incident_idx = index;
  incident_details = window_create();
#ifdef PBL_COLOR
  window_set_background_color(incident_details, GColorMelon);
#endif
	window_set_window_handlers(incident_details, (WindowHandlers) {
		.load = incident_details_window_load,
	  .unload = incident_details_window_unload,
	});

	window_stack_push(incident_details, true);
}

static void cb_incidents_received_handler(DictionaryIterator *iter, void *context) {
	APP_LOG(APP_LOG_LEVEL_DEBUG, "incidents app message callback");
  
 	Tuple *size_tuple = dict_find(iter, KEY_INCIDENTS_SIZE);
  if (!size_tuple) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "no KEY_INCIDENTS_SIZE in dict?");
    nb_incidents = 0;
    
    return;
  } else {
    
  }
  
  nb_incidents = size_tuple->value->int32;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "size=%d", nb_incidents);

  incidents_menu_items = malloc(nb_incidents * sizeof(SimpleMenuItem));
  
  for (int i = 0; i < nb_incidents; i++) {
    Tuple *message_tuple = dict_find(iter, i + KEY_INCIDENTS_FIRST_MESSAGE);
    Tuple *date_tuple = dict_find(iter, i + KEY_INCIDENTS_FIRST_DATE);
    char *message = malloc(strlen(message_tuple->value->cstring)+1);
    urldecode2(message, message_tuple->value->cstring);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "message=%s", message);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "date=%s", date_tuple->value->cstring);
    incidents_menu_items[i].title = message;
    incidents_menu_items[i].subtitle = date_tuple->value->cstring;
    incidents_menu_items[i].icon = NULL;
    incidents_menu_items[i].callback = show_incident_details;
    //free(message);
  }
  
  incidents_menu_section = (SimpleMenuSection) {
    "Incidents", incidents_menu_items, nb_incidents
  };
  
	Layer *window_layer = window_get_root_layer(incidents);
	GRect bounds = layer_get_bounds(window_layer);
  
	SimpleMenuLayer *menu = simple_menu_layer_create(bounds, incidents, &incidents_menu_section, 1, NULL);
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(menu), GColorRed, GColorWhite);
#endif
	layer_add_child(window_layer, (Layer *)menu);
  
}

static void incidents_load(Window *window) {

	Layer *window_layer = window_get_root_layer(incidents);
	GRect bounds = layer_get_bounds(window_layer);
  
	incidents_loading_layer = text_layer_create(GRect(0, bounds.size.h - 16, bounds.size.w, 16));
	text_layer_set_text(incidents_loading_layer, "Loading...");
	text_layer_set_font(incidents_loading_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(incidents_loading_layer, GTextAlignmentCenter);
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
  for (int i = 0; i < nb_incidents; i++) {
    free((char *)incidents_menu_items[i].title);
  }
  if (incidents_menu_items) {
    free(incidents_menu_items);
  }
  app_message_register_inbox_received(NULL);
}


void init_incidents(void) {
	// Register message handlers
	app_message_register_inbox_received(cb_incidents_received_handler);

  incidents = window_create();
  window_set_window_handlers(incidents, (WindowHandlers) {
		.load = incidents_load,
	  .unload = incidents_unload,
	});

	window_stack_push(incidents, true);
}

