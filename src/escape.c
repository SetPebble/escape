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
const int kTiltMaximum = 400;               //  tilt maximum
const int kTiltIncrement = 10;              //  tilt increment
const int kScreenWidth = 144;               //  screen width
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
typedef enum { entry_digits = 0, entry_fruit } Entry;
const char* kNothingToSelect = "nothing to select here";
const char* kInfoExplanation = "help text";

//  states

enum { state_none = -1, state_picture, state_safe };
typedef struct {
  bool value;
} State;
static State states[] = {
  //  picture (closed, swung open)
  { false },
  //  safe  (locked, unlocked)
  { false }
};

//  input screens

enum { input_none = -1, input_safe = 0 };
typedef struct __attribute__((__packed__)) {
  Entry entry;
  uint16_t value, correct;
  int8_t changes;
  const char* message;
} Input;
static Input inputs[] = {
    //{ entry_digits, 0, 1234, state_safe, "the safe is open" }
    { entry_fruit, 0, 1234, state_safe, "the safe is open" }
};

//  objects

enum { obj_screwdriver = 0 };
typedef struct __attribute__((__packed__)) {
  const char* name, * description;
  uint32_t image, icon;
  GPoint location;
  GSize size;
  int8_t dependency;
  bool visible, selectable, inventory;
} Object;
static Object objects[] = {
    //  screwdriver (selectable object)
    { "screwdriver", "flat-head",
      RESOURCE_ID_OBJECT_SCREWDRIVER, RESOURCE_ID_ICON_SCREWDRIVER,
      { 656, 400 }, { 36, 12 },
      state_none, true, true, false
    },
    //
    { "picture", NULL,
      RESOURCE_ID_OBJECT_PICTURE, 0,
      { 124, 176 }, { 44, 40 },
      state_picture, true, false, false
    },
    //
    { "safe", NULL,
      RESOURCE_ID_OBJECT_SAFE, 0,
      { 108, 176 }, { 60, 40 },
      state_safe, true, false, false
    }
};

//  rules

enum { rule_placeholder = 0 };
typedef struct __attribute__((__packed__)) {
  GRect rect;
  int8_t dependency, changes, input;
  const char* message;
} Rule;
static Rule rules[] = {
  //  rule to input safe combination (before picture)  (
  { { { 124, 176}, { 36, 40 } }, state_picture, state_safe, input_safe, NULL },
  //  rule to open/close picture
  { { { 124, 176}, { 44, 40 } }, state_none, state_picture, input_none, "you've found a safe" }
};

//  variables

static Window* window;
enum Setting { setting_backlight = 1, setting_tilt };
static enum { backlight_normal = 0, backlight_on, backlight_count } backlight;
static enum { tilt_flat = 0, tilt_45, tilt_upright, tilt_count } tilt;
static Entry entry;
static Layer* layer_screen;
static uint8_t input;
static GPoint pt_location, pt_fraction;
static int n_zoom, n_position;
static char str_temp[40];
static TextLayer* text_content;
static ScrollLayer* scroll_content;

//  includes

//#include "app_sync.c.inc"     //  communication
#include "window.c.inc"     //  create window
#include "draw.c.inc"       //  draw packbit image
#include "toast.c.inc"      //  toast messages
#include "info.c.inc"       //  info messages
#include "entry.c.inc"      //  entry window
#include "menu.c.inc"       //  entry window

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
      //  check dependency
      if ((obj->dependency >= 0) && !states[obj->dependency].value)
        continue;
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
  /*  DEBUG  */
  /*
  entry_launch(input_safe);
  return;
  */

  int i, n_margin = kObjectMargin / (1 << n_zoom);
  //  check for selectable objects
  for (i = 0;  i < (int) ARRAY_LENGTH(objects);  i++) {
    Object* obj = objects + i;
    if (obj->selectable &&
        (pt_location.x >= obj->location.x - n_margin) && (pt_location.x < obj->location.x + obj->size.w + n_margin) &&
        (pt_location.y >= obj->location.y - n_margin) && (pt_location.y < obj->location.y + obj->size.h + n_margin)) {
      //  selectable object
      if (obj->visible && !obj->inventory) {
        //  select object
        obj->visible = false;
        obj->inventory = true;
        //  redraw scene
        layer_mark_dirty(layer_screen);
        //  message
        snprintf(str_temp, sizeof(str_temp), "you now have a %s", obj->name);
        toast_show(str_temp);
        APP_LOG(APP_LOG_LEVEL_DEBUG, "object %s selected", obj->name);
        return;
      }
    }
  }
  //  check rules
  for (i = 0;  i < (int) ARRAY_LENGTH(rules);  i++) {
    Rule* rule = rules + i;
    if ((pt_location.x >= rule->rect.origin.x) && (pt_location.x < rule->rect.origin.x + rule->rect.size.w) &&
        (pt_location.y >= rule->rect.origin.y) && (pt_location.y < rule->rect.origin.y + rule->rect.size.h)) {
      //  check for state changes
      if (rule->changes >= 0) {
        //  check dependency
        if ((rule->dependency >= 0) && !states[rule->dependency].value)
          continue;
        //  see if rule has an input
        if (rule->input >= 0) {
          //  check input state
          Input* inp = inputs + rule->input;
          if (!states[inp->changes].value) {
            //  launch entry screen for input
            entry_launch(rule->input);
            return;
          }
        }
        //  changes state value
        State* state = states + rule->changes;
        state->value = !state->value;
        //  message
        if (state->value && rule->message)
          toast_show(rule->message);
        //  redraw scene
        layer_mark_dirty(layer_screen);
      }
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
  window_settings = window_escape((WindowHandlers) {
    .load = menu_load,
    .unload = menu_unload
  }, NULL);
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
}

//  main function

int main(void) {
  //  initialize variables
  backlight = backlight_normal;
  tilt = tilt_flat;
  pt_location = GPoint(kRoomMaxWidth / 2, kRoomMaxHeight / 2);
  pt_fraction = GPoint(kMaxFraction / 2, kMaxFraction / 2);
  n_zoom = 1;
  n_position = 0;
  memset(pixels, '\0', sizeof(pixels));
  memset(gauss, '\0', sizeof(gauss));
  memset(&menu_index, '\0', sizeof(menu_index));
  memset(&states, '\0', sizeof(states));
  entry = entry_digits;
  input = -1;
  //  modules
  toast_init();
  //  create window
  window = window_escape((WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  }, config_provider);
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
