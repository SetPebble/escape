//  file: menu.c.inc

//  constants

enum { section_inventory = 0, section_actions, section_settings, section_count };
enum { action_info = 0, action_reset, action_count };
enum { menu_backlight = 0, menu_tilt, menu_count };
const char* kNoInventory = "no items collected";
const char* kResetExplanation = "clear items, timer";
const char* kSettingsHeaders[] = { "Inventory", "Actions", "Settings" };
const char* kActionsOptions[] = { "Instructions", "Reset" };
const char* kSettingsOptions[] = { "Backlight", "Tilt" };
const char* kMenuValuesBacklight[] = { "normal", "always on" };
const char* kMenuValuesTilt[] = { "flat", "45 degrees", "upright" };
const uint32_t kActionsIcons[] = { RESOURCE_ID_ACTION_INFO, RESOURCE_ID_ACTION_RESET };
const uint32_t kSettingsIcons[] = { RESOURCE_ID_SETTING_BACKLIGHT, RESOURCE_ID_SETTING_TILT };

//  variables

static Window* window_settings;
static MenuLayer* menu_settings;
static MenuIndex menu_index;

//  functions

uint16_t menu_get_num_sections(MenuLayer* menu, void* data) {
  return section_count;
}

uint16_t menu_get_num_rows(MenuLayer* menu, uint16_t section_index, void* data) {
  if (section_index == section_inventory) {
    int count = 0;
    for (int i = 0;  i < (int) ARRAY_LENGTH(objects);  i++)
      if (objects[i].inventory)
        count++;
    return (count > 0) ? count : 1;
  } else
    return (section_index == section_actions) ? action_count : menu_count;
}

int16_t menu_get_header_height(MenuLayer* menu, uint16_t section_index, void* data) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

void menu_draw_header(GContext* context, const Layer* cell_layer, uint16_t section_index, void* data) {
  menu_cell_basic_header_draw(context, cell_layer, kSettingsHeaders[section_index]);
}

void menu_draw_row(GContext* context, const Layer* cell_layer, MenuIndex* cell_index, void* data) {
  const char* str_title = NULL;
  *str_temp = '\0';
  uint32_t n_icon = 0;
  switch (cell_index->section) {
    case section_inventory:
     {int count = 0;
      for (int i = 0;  i < (int) ARRAY_LENGTH(objects);  i++) {
        Object* obj = objects + i;
        if (obj->inventory)
          if (count++ == cell_index->row) {
            str_title = obj->name;
            if (obj->description)
              strcpy(str_temp, obj->description);
            n_icon = obj->icon;
            break;
          }
      }
      if (!str_title)
        str_title = kNoInventory;
     }break;
    case section_actions:
      str_title = kActionsOptions[cell_index->row];
      n_icon = kActionsIcons[cell_index->row];
      switch (cell_index->row) {
        case action_info:
          strcpy(str_temp, kInfoExplanation);
          break;
        case action_reset:
          strcpy(str_temp, kResetExplanation);
          break;
      }
      break;
    case section_settings:
      str_title = kSettingsOptions[cell_index->row];
      n_icon = kSettingsIcons[cell_index->row];
      switch (cell_index->row) {
        case menu_backlight:
          strcpy(str_temp, kMenuValuesBacklight[backlight]);
          break;
        case menu_tilt:
          strcpy(str_temp, kMenuValuesTilt[tilt]);
          break;
      }
      break;
  }
  if (str_title) {
    GBitmap* bitmap_icon = n_icon ? gbitmap_create_with_resource(n_icon) : NULL;
    menu_cell_basic_draw(context, cell_layer, str_title, str_temp, bitmap_icon);
    if (bitmap_icon)
      gbitmap_destroy(bitmap_icon);
  }
}

void menu_sync_and_persist(int setting, int value) {
  //sync_set_value(setting, value);
  persist_write_int(setting, value);
}

void menu_select(MenuLayer* menu, MenuIndex* cell_index, void* data) {
  switch (cell_index->section) {
    case section_inventory:
     {int count = 0;
      for (int i = 0;  i < (int) ARRAY_LENGTH(objects);  i++) {
        Object* obj = objects + i;
        if (obj->inventory)
          if (count++ == cell_index->row) {
            //  TODO
            break;
          }
      }
     }break;
    case section_actions:
      switch (cell_index->row) {
        case action_info:
          //  launch info window
          info_launch();
          break;
        case action_reset:
          //  TODO:
          break;
      }
      break;
    case section_settings:
      switch (cell_index->row) {
        case menu_backlight:
          backlight = (backlight + 1) % backlight_count;
          if (backlight != backlight_on) {
            light_enable(false);
            light_enable_interaction();
          } else
            light_enable(true);
          menu_sync_and_persist(setting_backlight, backlight);
          break;
        case menu_tilt:
          tilt = (tilt + 1) % tilt_count;
          menu_sync_and_persist(setting_tilt, tilt);
          break;
      }
      break;
  }
  //  redraw
  layer_mark_dirty(menu_layer_get_layer(menu_settings));
}

void menu_load(Window* window) {
  //  settings menu
  menu_settings = menu_layer_create(GRect(0, 0, 144, 168));
  //  set menu callbacks
  menu_layer_set_callbacks(menu_settings, NULL, (MenuLayerCallbacks) {
    .get_num_sections = menu_get_num_sections,
    .get_num_rows = menu_get_num_rows,
    .get_header_height = menu_get_header_height,
    .draw_header = menu_draw_header,
    .draw_row = menu_draw_row,
    .select_click = menu_select
  });
  //  bind the buttons
  menu_layer_set_click_config_onto_window(menu_settings, window);
  //  set default selection
  menu_layer_set_selected_index(menu_settings, menu_index, MenuRowAlignCenter, false);
  //  add to window
  layer_add_child(window_get_root_layer(window), menu_layer_get_layer(menu_settings));
}

void menu_unload(Window* window) {
  //  remember selection
  menu_index = menu_layer_get_selected_index(menu_settings);
  //  free menu layer
  menu_layer_destroy(menu_settings);
  menu_settings = NULL;
  //  free window memory
  window_destroy(window_settings);
}