#include <pebble.h>
#include "webcam.h"
#include "incidents.h"
#include "temps.h"


static Window *rootmenu;


/* main menu */


static void menuselect_incidents(int index, void *context) {
	init_incidents();
}

static void menuselect_traveltimes(int index, void *context) {
	init_traveltimes();
}

static void menuselect_webcams(int index, void *context) {
	init_webcam();
}


//static SimpleMenuLayer menu_layer;
static SimpleMenuSection menu_sections[2];
static SimpleMenuItem menu_section0_items[3];
static SimpleMenuItem menu_section1_items[1];

static void rootmenu_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds = layer_get_bounds(window_layer);

	menu_section0_items[0] = (SimpleMenuItem) {
		"Incidents", "en cours", NULL, menuselect_incidents
	};
	menu_section0_items[1] = (SimpleMenuItem) {
		"Temps", "de parcours", NULL, menuselect_traveltimes
	};
	menu_section0_items[2] = (SimpleMenuItem) {
		"Webcams", NULL, NULL, menuselect_webcams
	};

	menu_sections[0] = (SimpleMenuSection) {
		"Traffic Info Lux.", menu_section0_items, 3
	};

	menu_section1_items[0] = (SimpleMenuItem) {
		"PebbleTrafficLux", "Data (c) CITA", NULL, NULL
	};

	menu_sections[1] = (SimpleMenuSection) {
		"A propos", menu_section1_items, 1
	};

	SimpleMenuLayer *menu = simple_menu_layer_create(bounds, rootmenu, menu_sections, 2, NULL);
#ifdef PBL_COLOR
  menu_layer_set_highlight_colors(simple_menu_layer_get_menu_layer(menu), GColorCobaltBlue, GColorWhite);
#endif
  layer_add_child(window_layer, (Layer *)menu);

}

static void rootmenu_unload(Window *window) {
	//init_webcam();
}


static void init_menu(void) {
	// Init buffers
	app_message_open(app_message_inbox_size_maximum(), APP_MESSAGE_OUTBOX_SIZE_MINIMUM);
  
	rootmenu = window_create();
	window_set_window_handlers(rootmenu, (WindowHandlers) {
		.load = rootmenu_load,
			.unload = rootmenu_unload,
	});

	window_stack_push(rootmenu, true);
}

/* End main menu */


static void init(void) {
	init_menu();
	//init_webcam();
};

static void deinit(void) {
	//window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
