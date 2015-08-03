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
  { 1, "A4", "Lux. - Esch/Alzette", 0 },
  { 1, "A6", "Lux. - Belgique", 0 },
  { 1, "A7", "Lux. - Nord", 0 },
  { 1, "A13", "Coll. Sud - Sarre", 0 }
};

static struct Webcam webcams[] = {
  // HOTSPOTS
  // Cx. Gasperich
  { 0, "W (A6>A1/A3)", "dir. Trier/Metz", 42 },
  { 0, "E (merge A1<A3)", "dir. Trier/tunnel Howald", 46 },
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
  // AUTOROUTES
  // A1
  { 5, "Gasperich E", "dir. tunnel Howald", 46 },
  { 5, "Tunnel Howald S", "dir. Trier", 70 },
  { 5, "Tunnel Howald N", "dir. Gasperich", 77 },
  { 5, "Howald N", "dir. Sandweiler", 81 },
  { 5, "Sandweiler S", "dir. Trier", 84 },
  { 5, "Tunnel Cents S", "dir. Kirchberg", 86 },
  { 5, "Tunnel Cents N", "dir. Gasperich", 91 },
  { 5, "Jct. A1/A7", "dir. Kirchberg", 95 },
  { 5, "Senningerberg", "dir. Kirchberg", 98 },
  { 5, "Jct. A1/A7", "dir. Senningerberg", 100 },
  { 5, "Senningerberg", "dir. Cargo Center", 100 },
  { 5, "Münsbach S", "dir. Cargo Center", 101 },
  { 5, "Münsbach N", "dir. Cargo Center", 107 },
  { 5, "Münsbach N", "dir. Flaxweiler", 109 },
  { 5, "Flaxweiler", "dir. Münsbach", 111 },
  { 5, "Potaschbierg", "dir. Mertert", 111 },
  { 5, "Mertert", "dir. Potaschbierg", 111 },
  { 5, "Mertert", "dir. Aire / Trier", 111 },
  // A3
  { 6, "Gasperich N", "dir. Luxembourg", 47 },
  { 6, "Gasperich S", "dir. France", 41 },
  { 6, "Berchem N", "dir. Luxembourg", 51 },
  { 6, "Berchem N", "dir. France", 55 },
  { 6, "Berchem S", "dir. France", 53 },
  { 6, "Bettembourg N", "dir. France", 59 },
  { 6, "Bettembourg S", "dir. Luxembourg", 63 },
  { 6, "Dudelange", "dir. Luxembourg", 66 },
  // A4
  { 7, "Merl", "dir. Luxembourg", 36 },
  { 7, "Cessange N", "dir. Esch", 31 },
  { 7, "Cessange S", "dir. Luxembourg", 120 },
  { 7, "Leudelange N", "dir. Esch", 122 },
  { 7, "Leudelange S", "dir. Luxembourg", 125 },
  { 7, "Pontpierre", "dir. Luxembourg", 133 },
  { 7, "Foetz N", "dir. Esch", 137 },
  { 7, "Foetz S", "dir. Esch", 139 },
  { 7, "Esch-Lallange", "dir. Luxembourg", 140 },
  // A6
  { 8, "Gasperich W", "dir. Trier/Metz", 42 },
  { 8, "Cessange E", "dir. Gasperich", 37 },
  { 8, "Cessange W", "dir. Gasperich", 30 },
  { 8, "Bertrange E", "dir. Belgique", 27 },
  { 8, "Strassen E", "dir. Luxembourg", 21 },
  { 8, "Strassen W", "dir. Luxembourg", 15 },
  { 8, "Mamer E", "dir. Luxembourg", 17 },
  { 8, "Mamer W", "dir. Luxembourg", 11 },
  { 8, "Steinfort E", "dir. Luxembourg", 6 },
  { 8, "Steinfort W", "dir. Belgique", 3 },
  // A7
  { 9, "Lorentzweiler", "dir. Luxembourg", 400 },
  { 9, "Tunnel G'bierg S", "dir. Luxembourg", 402 },
  { 9, "Tunnel G'bierg S", "dir. Ettelbrück", 403 },
  { 9, "Tunnel G'bierg N", "dir. Luxmebourg", 444 },
  { 9, "Tunnel Mersch S", "dir. Ettelbrück", 445 },
  { 9, "Tunnel Mersch N", "dir. Luxembourg", 455 },
  { 9, "Mersch N", "dir. Luxembourg", 458 },
  { 9, "Colmar-Berg S", "dir. Luxembourg", 466 },
  { 9, "Colmar-Berg N", "dir. Luxembourg", 471 },
  // A13
  { 10, "Sanem W", "dir. Pétange", 143 },
  { 10, "Sanem E", "dir. Esch", 145 },
  { 10, "Soleuvre", "dir. Esch", 147 },
  { 10, "Ehlerange W", "dir. Esch", 151 },
  { 10, "Ehlerange E", "dir. Luxembourg", 154 },
  { 10, "Merge A13<A4", "dir. Pétange", 142 },
  { 10, "Schifflange", "dir. Foetz/Esch", 161 },
  { 10, "Kayl E", "dir. Dudelange", 162 },
  { 10, "Dudelange-B.", "dir. cx. Bettembourg", 164 },
  { 10, "Bettembourg W", "dir. Esch", 167 },
  { 10, "Bettembourg E", "dir. Schengen", 200 },
  { 10, "Frisange W", "dir. Schengen", 204 },
  { 10, "Frisange E", "dir. Bettembourg", 215 },
  { 10, "Altwies E", "dir. Bettembourg", 217 },
  { 10, "Mondorf-Altwies", "dir. Bettembourg", 218 },
  { 10, "Mondorf W", "dir. Bettembourg", 230 },
  { 10, "Mondorf W", "dir. Schengen", 231 },
  { 10, "Mondorf E", "dir. Schengen", 235 },
  { 10, "Schengen W", "dir. Deutschland", 236 },
  { 10, "Schengen W", "dir. Bettembourg", 237 },
  { 10, "Schengen E", "dir. Deutschland", 268 },
  { 10, "Schengen E", "dir. Bettembourg", 269 },
  // END
  { 99, "", "", 0 }
};
