#include <pebble.h>

//  constants

#define  kDebug  1
#define  LENGTH  96                         //  length of room tile in pixels
#define  ROWBYTES  LENGTH / 8               //  bytes per row
const int kRoomXTiles = 17;                 //  number of horizontal tiles
const int kRoomYTiles = 6;                  //  number of vertical tiles
const int kRoomMaxWidth = 17 * LENGTH;      //  room width in pixels
const int kRoomMaxHeight = 6 * LENGTH;      //  room height in pixels
const int kMaxFraction = 16;                //  used for partial location increment
const int kAltitudeThreshhold = (30 * TRIG_MAX_ANGLE) / 360;
const int kAltitudeRange = (120 * TRIG_MAX_ANGLE) / 360;
const int kTiltThreshhold = 400;            //  tilt threshhold
const int kObjectMargin = 12;               //  selectable area around object
const int kScrollMax = 4000;                //  maximum size
const int kScrollHorizontalPadding = 4;     //  pixel padding
const int kScrollVerticalPadding = 4;       //  pixel padding
const int kTiltMaximum = 400;               //  tilt maximum
const int kTiltIncrement = 10;              //  tilt increment
const int kScreenWidth = 144;               //  screen width
const int kGaussianThreshhold3 = 11;        //  threshhold for gaussian filter (3-tap)
const int kGaussianThreshhold5 = 200;       //  threshhold for gaussian filter (5-tap)
const int kScreenHeight = 168;              //  screen height
const int kZoomMax = 2;                     //  maximum zoom level
const uint32_t kRooms[] = {
  RESOURCE_ID_ROOM_0_0, RESOURCE_ID_ROOM_1_0, RESOURCE_ID_ROOM_2_0, RESOURCE_ID_ROOM_3_0, RESOURCE_ID_ROOM_4_0,
  RESOURCE_ID_ROOM_5_0, RESOURCE_ID_ROOM_6_0, RESOURCE_ID_ROOM_7_0, RESOURCE_ID_ROOM_8_0, RESOURCE_ID_ROOM_9_0,
  RESOURCE_ID_ROOM_10_0, RESOURCE_ID_ROOM_11_0, RESOURCE_ID_ROOM_12_0, RESOURCE_ID_ROOM_13_0, RESOURCE_ID_ROOM_14_0,
  RESOURCE_ID_ROOM_15_0, RESOURCE_ID_ROOM_16_0,
  RESOURCE_ID_ROOM_0_1, RESOURCE_ID_ROOM_1_1, RESOURCE_ID_ROOM_2_1, RESOURCE_ID_ROOM_3_1, RESOURCE_ID_ROOM_4_1,
  RESOURCE_ID_ROOM_5_1, RESOURCE_ID_ROOM_6_1, RESOURCE_ID_ROOM_7_1, RESOURCE_ID_ROOM_8_1, RESOURCE_ID_ROOM_9_1,
  RESOURCE_ID_ROOM_10_1, RESOURCE_ID_ROOM_11_1, RESOURCE_ID_ROOM_12_1, RESOURCE_ID_ROOM_13_1, RESOURCE_ID_ROOM_14_1,
  RESOURCE_ID_ROOM_15_1, RESOURCE_ID_ROOM_16_1,
  RESOURCE_ID_ROOM_0_2, RESOURCE_ID_ROOM_1_2, RESOURCE_ID_ROOM_2_2, RESOURCE_ID_ROOM_3_2, RESOURCE_ID_ROOM_4_2,
  RESOURCE_ID_ROOM_5_2, RESOURCE_ID_ROOM_6_2, RESOURCE_ID_ROOM_7_2, RESOURCE_ID_ROOM_8_2, RESOURCE_ID_ROOM_9_2,
  RESOURCE_ID_ROOM_10_2, RESOURCE_ID_ROOM_11_2, RESOURCE_ID_ROOM_12_2, RESOURCE_ID_ROOM_13_2, RESOURCE_ID_ROOM_14_2,
  RESOURCE_ID_ROOM_15_2, RESOURCE_ID_ROOM_16_2,
  RESOURCE_ID_ROOM_0_3, RESOURCE_ID_ROOM_1_3, RESOURCE_ID_ROOM_2_3, RESOURCE_ID_ROOM_3_3, RESOURCE_ID_ROOM_4_3,
  RESOURCE_ID_ROOM_5_3, RESOURCE_ID_ROOM_6_3, RESOURCE_ID_ROOM_7_3, RESOURCE_ID_ROOM_8_3, RESOURCE_ID_ROOM_9_3,
  RESOURCE_ID_ROOM_10_3, RESOURCE_ID_ROOM_11_3, RESOURCE_ID_ROOM_12_3, RESOURCE_ID_ROOM_13_3, RESOURCE_ID_ROOM_14_3,
  RESOURCE_ID_ROOM_15_3, RESOURCE_ID_ROOM_16_3,
  RESOURCE_ID_ROOM_0_4, RESOURCE_ID_ROOM_1_4, RESOURCE_ID_ROOM_2_4, RESOURCE_ID_ROOM_3_4, RESOURCE_ID_ROOM_4_4,
  RESOURCE_ID_ROOM_5_4, RESOURCE_ID_ROOM_6_4, RESOURCE_ID_ROOM_7_4, RESOURCE_ID_ROOM_8_4, RESOURCE_ID_ROOM_9_4,
  RESOURCE_ID_ROOM_10_4, RESOURCE_ID_ROOM_11_4, RESOURCE_ID_ROOM_12_4, RESOURCE_ID_ROOM_13_4, RESOURCE_ID_ROOM_14_4,
  RESOURCE_ID_ROOM_15_4, RESOURCE_ID_ROOM_16_4,
  RESOURCE_ID_ROOM_0_5, RESOURCE_ID_ROOM_1_5, RESOURCE_ID_ROOM_2_5, RESOURCE_ID_ROOM_3_5, RESOURCE_ID_ROOM_4_5,
  RESOURCE_ID_ROOM_5_5, RESOURCE_ID_ROOM_6_5, RESOURCE_ID_ROOM_7_5, RESOURCE_ID_ROOM_8_5, RESOURCE_ID_ROOM_9_5,
  RESOURCE_ID_ROOM_10_5, RESOURCE_ID_ROOM_11_5, RESOURCE_ID_ROOM_12_5, RESOURCE_ID_ROOM_13_5, RESOURCE_ID_ROOM_14_5,
  RESOURCE_ID_ROOM_15_5, RESOURCE_ID_ROOM_16_5 };
enum { section_inventory = 0, section_actions, section_settings, section_count };
enum { menu_backlight = 0, menu_tilt, menu_count };
enum { action_info = 0, action_reset, action_count };
const char* kNoInventory = "no items collected";
//const char* kNothingToSelect = "nothing to select here";
const char* kNothingToSelect = "why is this text uncentered?";
const char* kInfoExplanation = "help text";
const char* kResetExplanation = "clear items, timer";
const char* kSettingsHeaders[] = { "Inventory", "Actions", "Settings" };
const char* kActionsOptions[] = { "Instructions", "Reset" };
const char* kSettingsOptions[] = { "Backlight", "Tilt" };
const char* kMenuValuesBacklight[] = { "normal", "always on" };
const char* kMenuValuesTilt[] = { "flat", "45 degrees", "upright" };
const char* kInfoContents = "Here is the help text...";
const uint32_t kActionsIcons[] = { RESOURCE_ID_ACTION_INFO, RESOURCE_ID_ACTION_RESET };
const uint32_t kSettingsIcons[] = { RESOURCE_ID_SETTING_BACKLIGHT, RESOURCE_ID_SETTING_TILT };

//  objects

typedef struct {
  char* name, * description;
  uint32_t image, icon;
  GPoint location;
  GSize size;
  bool visible, selectable, inventory;
} Object;
enum { obj_screwdriver };
static Object objects[] = {
    { "screwdriver", "flat-head",
      RESOURCE_ID_OBJECT_SCREWDRIVER, RESOURCE_ID_ICON_SCREWDRIVER,
      { 656, 400 }, { 36, 12 },
      true, true, false
    }
};

//  rules

typedef struct {
  GRect rect;
} Rule;
static Rule rules[] = {
  //  screwdriver area
  { { { 632, 376 }, { 88, 36 } } }
};

//  variables

static Window* window, * window_settings, * window_info;
enum Setting { setting_backlight = 1, setting_tilt };
static enum { backlight_normal = 0, backlight_on, backlight_count } backlight;
static enum { tilt_flat = 0, tilt_45, tilt_upright, tilt_count } tilt;
static Layer* layer_screen;
static MenuLayer* menu_settings;
//static InverterLayer* inverter_layer;
static uint8_t pixels[ROWBYTES * LENGTH], pvalues[LENGTH + 4], gauss[LENGTH * 5];
static GBitmap bitmap_chunk;
static GPoint pt_location, pt_fraction;
static int n_zoom;
static char str_temp[40];
static MenuIndex menu_index;
static TextLayer* text_content;
static ScrollLayer* scroll_content;

//  includes

//#include "app_sync.c.inc"     //  communication
#include "toast.c.inc"     //  toast messages

//  unpack and draw object

void draw_object(GContext* context, uint32_t resource, GPoint pt, int n_zoom) {
  memset(pixels, '\0', sizeof(pixels));
  ResHandle res_handle = resource_get_handle(resource);
  if (res_handle) {
    //  get size
    int n_size = resource_size(res_handle);
    if (n_size) {
      //  allocate buffer
      uint8_t* buffer = malloc(n_size);
      if (buffer) {
        //  load resource
        resource_load(res_handle, buffer, n_size);
        //  get object size
        register uint8_t* src = buffer;
        int width = *(src++);
        int height = *(src++);
        int xbytes = (width + 7) / 8;
        //  unpack image
        int x = 0, y = 0;
        while ((src < buffer + n_size) && (y < height)) {
          int count = *src;
          if (*src >= 128) {
            //  repeat sequence
            count = 257 - count;    //  2's complement conversion + 1
            for (int i = 0;  i < count;  i++) {
              pixels[ROWBYTES * y + x] = src[1];
              if (++x >= xbytes) {
                x = 0;
                y++;
                if (y >= height)
                  break;
              }
            }
            src += 2;
          } else {
            //  literal
            count++;
            for (int i = 0;  i < count;  i++) {
              pixels[ROWBYTES * y + x] = src[i + 1];
              if (++x >= xbytes) {
                x = 0;
                y++;
                if (y >= height)
                  break;
              }
            }
            src += 1 + count;
          }
        }
        //  zoom
        if (n_zoom) {
          switch (n_zoom) {
            case 1:   //  zoom out level 1
              //  get initial row - loop through columns and gather values from bits
              for (x = 0;  x < width;  x++)
                pvalues[x + 1] = (pixels[x / 8] & (1 << (x % 8))) ? 1 : 0;
              //  extend
              pvalues[0] = pvalues[1];
              pvalues[1 + LENGTH] = pvalues[LENGTH];
              //  horizontal filter
              for (x = 0;  x < width;  x++)
                gauss[x] = pvalues[x] + 2 * pvalues[x + 1] + pvalues[x + 2];
              //  copy first row
              memcpy(gauss + ROWBYTES * 2, gauss, LENGTH);
              //  loop through rows
              for (int y = 0;  y < height;  y++) {
                //  check row
                if (y < height - 1) {
                  //  get next row - gather values from bits
                  for (x = 0;  x < width;  x++)
                    pvalues[x + 1] = (pixels[ROWBYTES * (y + 1) + x / 8] & (1 << (x % 8))) ? 1 : 0;
                  //  extend
                  pvalues[0] = pvalues[1];
                  pvalues[1 + LENGTH] = pvalues[LENGTH];
                  //  horizontal filter
                  for (x = 0;  x < width;  x++)
                    gauss[LENGTH * ((y + 1) % 3) + x] = pvalues[x] + 2 * pvalues[x + 1] + pvalues[x + 2];
                } else
                  memcpy(gauss + LENGTH * ((y + 1) % 3), gauss + LENGTH * y, LENGTH);
                //  skip every other row
                if (!(y % 2)) {
                  //  clear pixel row
                  memset(pixels + ROWBYTES * (y / 2), '\0', ROWBYTES / 2);
                  //  vertical filter
                  for (x = 0;  x < width;  x += 2)
                    if (gauss[LENGTH * ((y + 2) % 3) + x] + 2 * gauss[LENGTH * (y % 3) + x] + gauss[LENGTH * ((y + 1) % 3) + x] > kGaussianThreshhold3)
                      pixels[ROWBYTES * (y / 2) + (x / 16)] |= 1 << ((x / 2) % 8);
                }
              }
#ifdef  SIMPLE_ZOOM_OUT_HORRIBLE_RESULTS
              for (y = 0;  y < height;  y += 2) {
                uint16_t* src1 = (uint16_t*) (pixels + (ROWBYTES * y));
                uint8_t* dest1 = pixels + (ROWBYTES * y / 2);
                for (x = 0;  x < width / 2;  x++)
                  *(dest1++) = *(src1++) / 2;
                  // *(dest1++) = (*(src1++) & *(src1a++)) / 2;
              }
#endif
              break;
            case 2:   //  zoom out level 2
              //  loop through first two rows
              for (y = 0;  y < 2;  y++) {
                //  get initial two rows - loop through columns and gather values from bits
                for (x = 0;  x < width;  x++)
                  pvalues[x + 2] = (pixels[ROWBYTES * y + (x / 8)] & (1 << (x % 8))) ? 1 : 0;
                //  extend
                pvalues[0] = pvalues[1] = pvalues[2];
                pvalues[LENGTH + 3] = pvalues[LENGTH + 2] = pvalues[LENGTH + 1];
                //  horizontal 5-tap filter
                for (x = 0;  x < width;  x++)
                  gauss[LENGTH * y + x] = pvalues[x] + 4 * pvalues[x + 1] + 6 * pvalues[x + 2] + 4 * pvalues[x + 3] + pvalues[x + 4];
              }
              //  copy first two rows
              memcpy(gauss + ROWBYTES * 3, gauss, LENGTH);
              memcpy(gauss + ROWBYTES * 4, gauss, LENGTH);
              //  loop through rows
              for (int y = 0;  y < height;  y++) {
                //  check row
                if (y < height - 2) {
                  //  get +2 row - gather values from bits
                  for (x = 0;  x < width;  x++)
                    pvalues[x + 2] = (pixels[ROWBYTES * (y + 2) + x / 8] & (1 << (x % 8))) ? 1 : 0;
                  //  extend
                  pvalues[0] = pvalues[1] = pvalues[2];
                  pvalues[LENGTH + 3] = pvalues[LENGTH + 2] = pvalues[LENGTH + 1];
                  //  horizontal 5-tap filter
                  for (x = 0;  x < width;  x++)
                    gauss[LENGTH * ((y + 2) % 5) + x] = pvalues[x] + 4 * pvalues[x + 1] + 6 * pvalues[x + 2] + 4 * pvalues[x + 3] + pvalues[x + 4];
                } else
                  memcpy(gauss + LENGTH * ((y + 2) % 5), gauss + LENGTH * ((y + 1) % 5), LENGTH);
                //  only display every 4th row
                if (!(y % 4)) {
                  //  clear pixel row
                  memset(pixels + ROWBYTES * (y / 4), '\0', ROWBYTES / 4);
                  //  vertical filter
                  for (x = 0;  x < width;  x += 4)
                    if (gauss[LENGTH * ((y + 3) % 5) + x] + 4 * gauss[LENGTH * ((y + 4) % 5) + x] + 6 * gauss[LENGTH * (y % 5) + x] + 4 * gauss[LENGTH * ((y + 1) % 5) + x] + gauss[LENGTH * ((y + 2) % 5) + x] > kGaussianThreshhold5)
                      pixels[ROWBYTES * (y / 4) + (x / 32)] |= 1 << ((x / 4) % 8);
                }
              }
              break;
          }
          width /= (1 << n_zoom);
          height /= (1 << n_zoom);
        }
        //  set bounds size
        bitmap_chunk.bounds.size.w = width;
        bitmap_chunk.bounds.size.h = height;
        //  draw bitmap
        graphics_draw_bitmap_in_rect(context, &bitmap_chunk, GRect(pt.x, pt.y, width, height));
        //  free buffer
        free(buffer);
      }
    }
  }
}

//  screen redraw

void screen_update(Layer* layer, GContext* context) {
  //  set color
  graphics_context_set_fill_color(context, GColorWhite);
  //  zoom factor and length
  int n_factor = (1 << n_zoom);
  int n_length = LENGTH / n_factor;
  //  calculate corners
  GPoint pt_topleft = GPoint(pt_location.x / n_factor - kScreenWidth / 2, pt_location.y / n_factor - kScreenHeight / 2),
         pt_botright = GPoint(pt_topleft.x + kScreenWidth - 1, pt_topleft.y + kScreenHeight - 1);
  //  figure out which block this is
  GPoint pt_start = GPoint(pt_topleft.x / n_length, pt_topleft.y / n_length),
         pt_end = GPoint(pt_botright.x / n_length, pt_botright.y / n_length);
  //  check min/max
  if (pt_start.x < 0)
    pt_start.x = 0;
  else if (pt_start.x >= kRoomXTiles)
    pt_start.x = kRoomXTiles - 1;
  if (pt_start.y < 0)
    pt_start.y = 0;
  else if (pt_start.y >= kRoomYTiles)
    pt_start.y = kRoomYTiles - 1;
  if (pt_end.x < 0)
    pt_end.x = 0;
  else if (pt_end.x >= kRoomXTiles)
    pt_end.x = kRoomXTiles - 1;
  if (pt_end.y < 0)
    pt_end.y = 0;
  else if (pt_end.y >= kRoomYTiles)
    pt_end.y = kRoomYTiles - 1;
  //  remember the remainder
  GPoint pt_remainder = GPoint(pt_start.x * n_length - pt_topleft.x, pt_start.y * n_length - pt_topleft.y);
  //  clear edges
  if (pt_remainder.y > 0)
    graphics_fill_rect(context, GRect(0, 0, kScreenWidth, pt_remainder.y), 0, GCornerNone);
  if (pt_remainder.y + (pt_end.y - pt_start.y) * n_length < kScreenHeight)
    graphics_fill_rect(context, GRect(0, pt_remainder.y + (pt_end.y - pt_start.y) * n_length, kScreenWidth, kScreenHeight - (pt_remainder.y + (pt_end.y - pt_start.y) * n_length)), 0, GCornerNone);
  if (pt_remainder.x > 0)
    graphics_fill_rect(context, GRect(0, 0, pt_remainder.x, kScreenHeight), 0, GCornerNone);
  if (pt_remainder.x + (pt_end.x - pt_start.x) * n_length < kScreenWidth)
    graphics_fill_rect(context, GRect(pt_remainder.x + (pt_end.x - pt_start.x) * n_length, 0, kScreenHeight - (pt_remainder.x + (pt_end.x - pt_start.x) * n_length), kScreenHeight), 0, GCornerNone);
  //  loop through blocks
  GPoint pt;
  for (pt.y = pt_start.y;  pt.y <= pt_end.y;  pt.y++)
    for (pt.x = pt_start.x;  pt.x <= pt_end.x;  pt.x++) {
      //  figure out which object
      int n_img = kRoomXTiles * pt.y + pt.x;
      if ((n_img >= 0) && (n_img < (int) ARRAY_LENGTH(kRooms)))
        draw_object(context, kRooms[n_img], GPoint(pt_remainder.x + (pt.x - pt_start.x) * n_length, pt_remainder.y + (pt.y - pt_start.y) * n_length), n_zoom);
    }
  //  overlay objects
  for (int i = 0;  i < (int) ARRAY_LENGTH(objects);  i++) {
    Object* obj = objects + i;
    if (obj->visible) {
      //  calculate starting point
      pt = GPoint(obj->location.x / n_factor - pt_topleft.x, obj->location.y / n_factor - pt_topleft.y);
      //APP_LOG(APP_LOG_LEVEL_DEBUG, "%s  pt=%d,%d", obj->name, pt.x, pt.y);
      //  check object against screen boundary
      if ((pt.x + obj->size.w > 0) && (pt.x < kScreenWidth) &&
          (pt.y + obj->size.h > 0) && (pt.y < kScreenHeight))
        //  draw the object
        draw_object(context, obj->image, pt, n_zoom);
    }
  }
}

//  info window

void info_load(Window* window) {
  //  contents
  scroll_content = scroll_layer_create(GRect(0, 0, 144, 168));
  scroll_layer_set_content_size(scroll_content, GSize(144, kScrollMax));
  text_content = text_layer_create(GRect(kScrollHorizontalPadding, 0, 144 - 2 * kScrollHorizontalPadding, kScrollMax));
  text_layer_set_background_color(text_content, GColorWhite);
  text_layer_set_text_color(text_content, GColorBlack);
  text_layer_set_font(text_content, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(text_content, GTextAlignmentLeft);
  scroll_layer_add_child(scroll_content, text_layer_get_layer(text_content));
  scroll_layer_set_click_config_onto_window(scroll_content, window);
  //  trim text layer and scroll content to fit text box
  text_layer_set_text(text_content, kInfoContents);
  //  calculate text size
  GSize max_size = text_layer_get_content_size(text_content);
  max_size.h += kScrollVerticalPadding;
  GRect rect = layer_get_frame(scroll_layer_get_layer(scroll_content));
  if (max_size.h < rect.size.h)
    max_size.h = rect.size.h;
  //  trim text layer and scroll content to fit text box
  text_layer_set_size(text_content, max_size);
  scroll_layer_set_content_size(scroll_content, GSize(144, max_size.h));
  layer_add_child(window_get_root_layer(window), scroll_layer_get_layer(scroll_content));
}

void info_unload(Window* window) {
  //  free layers
  text_layer_destroy(text_content);
  scroll_layer_destroy(scroll_content);
}

//  menu handlers

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
          //  initialize help window
          window_info = window_create();
          window_set_fullscreen(window_info, true);
          window_set_background_color(window_info, GColorWhite);
          window_set_window_handlers(window_info, (WindowHandlers) {
            .load = info_load,
            .unload = info_unload
          });
          window_stack_push(window_info, true);
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

//  accelerometer

static void accel_callback(AccelData* accel_data, uint32_t num_samples) {
  if (num_samples) {
    //  refresh flag
    bool b_refresh = false;
    //  tilt horizontally
    AccelData accel = *accel_data;
    if (accel.x < -kTiltThreshhold) {
      accel.x = -accel.x - kTiltThreshhold;
      if (accel.x > kTiltMaximum)
        accel.x = kTiltMaximum;
      pt_fraction.x -= (kTiltIncrement * accel.x * kMaxFraction) / kTiltMaximum;
      while (pt_fraction.x < 0) {
        if (pt_location.x > 144 / 2) {
          pt_location.x--;
          b_refresh = true;
        }
        pt_fraction.x += kMaxFraction;
      }
    } else if (accel.x > kTiltThreshhold) {
      accel.x -= kTiltThreshhold;
      if (accel.x > kTiltMaximum)
        accel.x = kTiltMaximum;
      pt_fraction.x += (kTiltIncrement * accel.x * kMaxFraction) / kTiltMaximum;
      while (pt_fraction.x >= kMaxFraction) {
        if (pt_location.x < kRoomMaxWidth - 144 / 2) {
          pt_location.x++;
          b_refresh = true;
        }
        pt_fraction.x -= kMaxFraction;
      }
    }
    //  altitude
    int n_altitude = (TRIG_MAX_ANGLE + atan2_lookup(-accel_data->y, -accel_data->z) - (tilt * TRIG_MAX_ANGLE) / 8) % TRIG_MAX_ANGLE;
    if (n_altitude > TRIG_MAX_ANGLE / 2)
      n_altitude = n_altitude - TRIG_MAX_ANGLE;
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", (n_altitude * 360) / TRIG_MAX_ANGLE);
    if ((n_altitude > kAltitudeThreshhold) && (n_altitude < kAltitudeThreshhold + kAltitudeRange)) {
      n_altitude -= kAltitudeThreshhold;
      pt_fraction.y += (kTiltIncrement * n_altitude * kMaxFraction) / kAltitudeRange;
      while (pt_fraction.y >= kMaxFraction) {
        if (pt_location.y < kRoomMaxHeight - 168 / 2) {
          pt_location.y++;
          b_refresh = true;
        }
        pt_fraction.y -= kMaxFraction;
      }
    } else if ((n_altitude < -kAltitudeThreshhold) && (n_altitude < kAltitudeThreshhold + kAltitudeRange)) {
      n_altitude += kAltitudeThreshhold;
      pt_fraction.y += (kTiltIncrement * n_altitude * kMaxFraction) / kAltitudeRange;
      while (pt_fraction.y < 0) {
        if (pt_location.y > 168 / 2) {
          pt_location.y--;
          b_refresh = true;
        }
        pt_fraction.y += kMaxFraction;
      }
    }
    //  refresh screen
    if (b_refresh)
      layer_mark_dirty(layer_screen);
  }
}

//  tick callback

static void tick_callback(struct tm* tick_time, TimeUnits units_changed) {
  //  update time
  if (units_changed & SECOND_UNIT) {
    //  TODO:  update screen timer
  }
  //  get remote settings
  //if (!units_changed)
  //  sync_schedule(NULL, kSyncSettingsDelay);
  //  retry interval
}

//  buttons

void button_up(ClickRecognizerRef recognizer, void* context) {
  if (n_zoom > 0) {
    n_zoom--;
    layer_mark_dirty(layer_screen);
  }
}

void button_select(ClickRecognizerRef recognizer, void* context) {
  int i, n_margin = kObjectMargin / (1 << n_zoom);
  //  check for selectable objects
  for (i = 0;  i < (int) ARRAY_LENGTH(objects);  i++) {
    Object* obj = objects + i;
    if (obj->visible && obj->selectable && !obj->inventory &&
        (pt_location.x >= obj->location.x - n_margin) && (pt_location.x < obj->location.x + obj->size.w + n_margin) &&
        (pt_location.y >= obj->location.y - n_margin) && (pt_location.y < obj->location.y + obj->size.h + n_margin)) {
      //  select object
      obj->visible = false;
      obj->inventory = true;
      layer_mark_dirty(layer_screen);
      //  message
      snprintf(str_temp, sizeof(str_temp), "you now have a %s", obj->name);
      toast_show(str_temp);
      APP_LOG(APP_LOG_LEVEL_DEBUG, "object %s selected", obj->name);
      return;
    }
  }
  //  check rules
  for (int i = 0;  i < (int) ARRAY_LENGTH(rules);  i++) {
    Rule* rule = rules + i;
    if ((pt_location.x >= rule->rect.origin.x) && (pt_location.x < rule->rect.origin.x + rule->rect.size.w) &&
        (pt_location.y >= rule->rect.origin.y) && (pt_location.y < rule->rect.origin.y + rule->rect.size.h)) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "rule %d triggered", i);
      //  TODO:
      return;
    }
  }
  //  nothing selected
  APP_LOG(APP_LOG_LEVEL_DEBUG, "nothing at %d,%d", pt_location.x, pt_location.y);
  toast_show(kNothingToSelect);
}

void button_down(ClickRecognizerRef recognizer, void* context) {
  if (n_zoom < kZoomMax) {
    n_zoom++;
    layer_mark_dirty(layer_screen);
  }
}

void button_long(ClickRecognizerRef recognizer, void* context) {
  //  initialize settings window
  window_settings = window_create();
  window_set_fullscreen(window_settings, true);
  window_set_background_color(window_settings, GColorWhite);
  window_set_window_handlers(window_settings, (WindowHandlers) {
    .load = menu_load,
    .unload = menu_unload
  });
  window_stack_push(window_settings, true);
}

void config_provider(void* context) {
  //  set select handlers
  window_single_click_subscribe(BUTTON_ID_SELECT, button_select);
  window_single_click_subscribe(BUTTON_ID_UP, button_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, button_down);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, button_long, NULL);
  window_long_click_subscribe(BUTTON_ID_UP, 0, button_long, NULL);
  window_long_click_subscribe(BUTTON_ID_DOWN, 0, button_long, NULL);
}

//  window functions

static void window_load(Window* window) {
  //  get root layer
  Layer* layer_root = window_get_root_layer(window);
  //  object bitmap
  bitmap_chunk = (GBitmap) {
    .addr = pixels,
    .bounds = GRect(0, 0, LENGTH, LENGTH),
    .info_flags = 1,
    .row_size_bytes = ROWBYTES,  // 4*ceiling(LENGTH/32)
  };
  //  screen
  layer_screen = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(layer_screen, screen_update);
  layer_add_child(layer_root, layer_screen);
  //  inverter layer
  //inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  //layer_add_child(layer_root, inverter_layer_get_layer(inverter_layer));
  //layer_set_hidden(inverter_layer_get_layer(inverter_layer), screen == screen_normal);
  //  tick service
  tick_timer_service_subscribe(SECOND_UNIT, tick_callback);
  //  accelerometer subscription
  accel_data_service_subscribe(1, accel_callback);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
}

static void window_unload(Window* window) {
  //  tick unsubscribe
  tick_timer_service_unsubscribe();
  //  accel unsubscribe
  //accel_data_service_unsubscribe();   //  commented out to prevent crashing in firmware 2.7/2.8
  //  free layers
  layer_destroy(layer_screen);
  //  free inverter
  //inverter_layer_destroy(inverter_layer);
}

//  main function

int main(void) {
  //  initialize variables
  backlight = backlight_normal;
  tilt = tilt_flat;
  pt_location = GPoint(kRoomMaxWidth / 2, kRoomMaxHeight / 2);
  pt_fraction = GPoint(kMaxFraction / 2, kMaxFraction / 2);
  n_zoom = 1;
  memset(pixels, '\0', sizeof(pixels));
  memset(gauss, '\0', sizeof(gauss));
  memset(&menu_index, '\0', sizeof(menu_index));
  //  modules
  toast_init();
  //  create window
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorWhite);
  window_set_fullscreen(window, true);
  window_set_click_config_provider(window, config_provider);
  window_stack_push(window, true);
  //  persistence
  for (int i = setting_backlight;  i <= setting_backlight;  i++) {
    if (persist_exists(i)) {
      int value = persist_read_int(i);
      if (value >= 0)
        switch (i) {
          case setting_backlight:
            if (value < backlight_count)
              backlight = value;
            break;
          case setting_tilt:
            if (value < tilt_count)
              tilt = value;
            break;
        }
    }
  }
  //  activate backlight
  if (backlight == backlight_on)
    light_enable(true);
  //  main event loop
  app_event_loop();
  //  reset backlight
  if (backlight == backlight_on) {
    light_enable(false);
    light_enable_interaction();
  }
  //  modules
  toast_deinit();
  //  backlight off
#if  kDebug
  light_enable(false);
#endif
  //  destroy window
  window_destroy(window);
}
