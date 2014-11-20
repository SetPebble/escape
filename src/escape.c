#include <pebble.h>

//  constants

#define  kDebug  0

//  variables

static Window* window;
static Layer* layer_screen;
//static InverterLayer* inverter_layer;

//  includes

//#include "app_sync.c.inc"     //  communication

//  screen redraw

void screen_update(Layer* layer, GContext* context) {
  //  DEBUG:
  graphics_context_set_stroke_color(context, GColorWhite);
  graphics_draw_text(context, "ESCAPE THE ROOM", fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), GRect(0, 44, 144, 64), GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
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

//  window functions

static void window_load(Window* window) {
  //  get root layer
  Layer* layer_root = window_get_root_layer(window);
  //  weather
  layer_screen = layer_create(GRect(0, 0, 144, 98));
  layer_set_update_proc(layer_screen, screen_update);
  layer_add_child(layer_root, layer_screen);
  //  inverter layer
  //inverter_layer = inverter_layer_create(GRect(0, 0, 144, 168));
  //layer_add_child(layer_root, inverter_layer_get_layer(inverter_layer));
  //layer_set_hidden(inverter_layer_get_layer(inverter_layer), screen == screen_normal);
  //  tick service
  tick_timer_service_subscribe(SECOND_UNIT, tick_callback);
}

static void window_unload(Window* window) {
  //  tick unsubscribe
  tick_timer_service_unsubscribe();
  //  free layers
  layer_destroy(layer_screen);
  //  free inverter
  //inverter_layer_destroy(inverter_layer);
}

//  main function

int main(void) {
  //  create splash window
  //  TODO:
  //  create window
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_set_background_color(window, GColorBlack);
  window_set_fullscreen(window, true);
  window_stack_push(window, true);
  //  main event loop
  app_event_loop();
  //  destroy window
  window_destroy(window);
}
