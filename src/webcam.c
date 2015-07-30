#include <pebble.h>
#include "webcam.h"
#include "webcam_data.h"


static Window *webcam_window;

static Window *webcams_mainmenu;
static Window *webcams_submenu;


#define KEY_IMAGE   0
#define KEY_INDEX   1
#define KEY_MESSAGE 2
#define KEY_SIZE    3

#define KEY_WEBCAM  9
  
#define CHUNK_SIZE 1500

#define NB_SECTIONS                2
#define MAX_MAINMENUSECTION_ITEMS  8
#define MAX_SUBMENUSECTION_ITEMS  20

static int webcams_current_submenu_idx = 0;
static int webcam_current_id = 0;

static SimpleMenuSection webcams_mainmenu_sections[NB_SECTIONS];
static SimpleMenuItem webcams_mainmenu_section_items[NB_SECTIONS][MAX_MAINMENUSECTION_ITEMS];

static SimpleMenuSection webcams_submenu_sections[1];
static SimpleMenuItem webcams_submenu_section_items[MAX_SUBMENUSECTION_ITEMS];


static BitmapLayer  *image_layer;
static GBitmap      *image = NULL;
static uint8_t      *data_image = NULL;
static uint32_t     data_size;

static TextLayer    *text_layer;

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

static void webcam_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	image_layer = bitmap_layer_create(bounds);
	bitmap_layer_set_alignment(image_layer, GAlignCenter);
	layer_add_child(window_layer, bitmap_layer_get_layer(image_layer));

	text_layer = text_layer_create(GRect(0, bounds.size.h - 16, bounds.size.w, 16));
	text_layer_set_text(text_layer, "Please wait...");
	text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_layer, GColorClear);
	layer_add_child(window_layer, text_layer_get_layer(text_layer));


	DictionaryIterator *iter;
	app_message_outbox_begin(&iter);
	dict_write_int(iter, KEY_WEBCAM, &webcam_current_id, sizeof(int), true);
	dict_write_end(iter);
	app_message_outbox_send();  

}

static void webcam_window_unload(Window *window) {
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
  
  //window_destroy(webcams_mainmenu);
}

/* Webcams sub menu */

static void webcam_submenu_select(int index, void *context) {
  int first = 0;
  while (webcams[first].mainmenu_id < webcams_current_submenu_idx) first++;

  webcam_current_id = webcams[first + index].webcam_id;
  APP_LOG(APP_LOG_LEVEL_DEBUG, "webcam selection first=%d index=%d webcamid=%d", first, index, webcam_current_id);

  
	app_message_register_inbox_received(cb_in_received_handler);
  
	webcam_window = window_create();
	//window_set_click_config_provider(webcams_mainmenu, click_config_provider);
	window_set_window_handlers(webcam_window, (WindowHandlers) {
		.load = webcam_window_load,
		.unload = webcam_window_unload,
	});
#ifdef PBL_SDK_2
	window_set_fullscreen(webcam_window, true);
#endif
	window_stack_push(webcam_window, true);
}  


static void webcam_submenu_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);
  
  int count = 0;
  for (int i = 0; webcams[i].mainmenu_id <= webcams_current_submenu_idx; i++) {
    if (webcams[i].mainmenu_id == webcams_current_submenu_idx) {
      webcams_submenu_section_items[count++] = (SimpleMenuItem) {
        webcams[i].title,
        webcams[i].subtitle,
        NULL,
        webcam_submenu_select
      };
    }

    webcams_submenu_sections[0] = (SimpleMenuSection) {
      webcams_main_menu_items[webcams_current_submenu_idx].title,
      webcams_submenu_section_items,
      count
    };
  }

	SimpleMenuLayer *menu = simple_menu_layer_create(bounds, webcams_submenu, webcams_submenu_sections, 1, NULL);
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(menu), GColorChromeYellow, GColorWhite);
  menu_layer_set_normal_colors(simple_menu_layer_get_menu_layer(menu), GColorPastelYellow, GColorBlack);
#endif
	layer_add_child(window_layer, (Layer *)menu);
}

static void webcam_submenu_unload(Window *window) {
	//init_webcam();
}

/* Webcams Main menu */

static void webcam_mainmenu_select(int index, void *context) {
  webcams_current_submenu_idx = index;
	webcams_submenu = window_create();
	//window_set_click_config_provider(webcams_mainmenu, click_config_provider);
	window_set_window_handlers(webcams_submenu, (WindowHandlers) {
		.load = webcam_submenu_load,
		.unload = webcam_submenu_unload,
	});
	window_stack_push(webcams_submenu, true);
}  


static void webcam_mainmenu_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

  int k = 0;
  for (int i = 0; i < NB_SECTIONS; i++) {
    for (int j = 0; j < webcams_main_menu_section_items_count[i]; j++) {
      webcams_mainmenu_section_items[i][j] = (SimpleMenuItem) {
        webcams_main_menu_items[k].title,
        webcams_main_menu_items[k].subtitle,
        NULL,
        webcam_mainmenu_select
      };
      k++;
    }
    
    webcams_mainmenu_sections[i] = (SimpleMenuSection) {
      webcams_main_menu_section_names[i],
      webcams_mainmenu_section_items[i],
      webcams_main_menu_section_items_count[i]
    };
  }

	SimpleMenuLayer *menu = simple_menu_layer_create(bounds, webcams_mainmenu, webcams_mainmenu_sections, NB_SECTIONS, NULL);
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(menu), GColorTiffanyBlue, GColorWhite);
  menu_layer_set_normal_colors(simple_menu_layer_get_menu_layer(menu), GColorCeleste, GColorBlack);
#endif
  layer_add_child(window_layer, (Layer *)menu);
}

static void webcam_mainmenu_unload(Window *window) {
	//init_webcam();
}


/* Init */


void init_webcam(void) {
	//webcam_app_message_init();
	webcams_mainmenu = window_create();
	//window_set_click_config_provider(webcams_mainmenu, click_config_provider);
	window_set_window_handlers(webcams_mainmenu, (WindowHandlers) {
		.load = webcam_mainmenu_load,
		.unload = webcam_mainmenu_unload,
	});
	window_stack_push(webcams_mainmenu, true);
}


