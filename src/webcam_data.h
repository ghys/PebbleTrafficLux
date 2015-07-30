#pragma once
  
struct WebcamsMainMenuEntry {
  int section_id;
  char *title;
  char *subtitle;
  int command;
};

struct Webcam {
  int mainmenu_id;
  char *title;
  char *subtitle;
  int webcam_id;
};

static char *webcams_main_menu_section_names[2] = {
  "Webcam Hotspots",
  "Autoroutes"
};

static int webcams_main_menu_section_items_count[2] = { 5, 6 };

#define NB_MENU_ITEMS  3
static struct WebcamsMainMenuEntry webcams_main_menu_items[] = {
  { 0, "Cx. Gasperich", "A1 & A6 & A3", 0 },
  { 0, "Cx. Cessange", "A6 & A4", 0 },
  { 0, "Cx. Bettembourg", "A3 & A13", 0 },
  { 0, "Jct. Foetz", "A4 & A13", 0 },
  { 0, "Jct. Lankelz", "A4 & A13", 0 },
  { 1, "A1", "Lux. - Trier", 0 },
  { 1, "A3", "Lux. - France", 0 },
  { 1, "A4", "Lux. - Esch/Alz.", 0 },
  { 1, "A6", "Lux. - Belgique", 0 },
  { 1, "A7", "Lux. - Nord", 0 },
  { 1, "A13", "Coll. Sud - Sarre", 0 }
};

static struct Webcam webcams[] = {
  // HOTSPOTS
  // Cx. Gasperich
  { 0, "W (A6>A1/A3)", "dir. Trier/Metz", 42 },
  { 0, "E (merge A1<A3)", "dir. Trier/Tunnel Howald", 46 },
  { 0, "S (A3<A1/A6)", "dir. France/Dudelange", 41 },
  { 0, "N (A3<Hesper)", "dir. Luxembourg", 47 },
  // Cx. Cessange
  { 1, "W (split A6>A4)", "dir. Gasperich", 30 },
  { 1, "E (merge A6<A4)", "dir. Gasperich", 37 },
  { 1, "S (merge A4<A6)", "dir. Luxembourg", 31 },
  { 1, "N (split A4>A6)", "dir. Esch", 120 },
  // Cx. Bettembourg
  { 2, "S (split A3>A13)", "dir. Luxembourg", 63 },
  { 2, "S (merge A3<A13)", "dir. Gasperich", 66 },
  { 2, "W (A13)", "dir. Esch/Foetz", 167 },
  { 2, "E (A13)", "dir. Hellange/Frisange", 200 },
  // Jct. Foetz
  { 3, "N (split A4>A13)", "dir. Esch", 137 },
  { 3, "S (merge A4<A13)", "dir. Esch", 139 },
  // Jct. Lankelz
  { 4, "N (split A4<A13)", "dir. Luxembourg", 140 },
  { 4, "W (merge A13<A4)", "dir. Pétange", 142 },
  // END
  { 99, "", "", 0 }
};
