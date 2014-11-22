#include <pebble.h>

//  constants

#define  kDebug  1
#define  LENGTH  96
#define  ROWBYTES  LENGTH / 8
const int kRoomXTiles = 17;
const int kRoomYTiles = 6;
const int kRoomMaxWidth = 17 * LENGTH;
const int kRoomMaxHeight = 6 * LENGTH;
const int kMaxFraction = 16;
const int kAltitudeThreshhold = (45 * TRIG_MAX_ANGLE) / 360;
const int kTiltThreshhold = 400;      //  tilt threshhold
const int kTiltMaximum = 400;         //  tilt maximum
const int kTiltIncrement = 10;        //  tilt increment
const int kScreenWidth = 144;         //  screen width
const int kGaussianThreshhold3 = 11;  //  threshhold for gaussian filter (3-tap)
const int kGaussianThreshhold5 = 200; //  threshhold for gaussian filter (5-tap)
const int kScreenHeight = 168;        //  screen height
const int kZoomMax = 2;               //  maximum zoom level
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

//  variables

static Window* window;
static Layer* layer_screen;
//static InverterLayer* inverter_layer;
static uint8_t pixels[ROWBYTES * LENGTH], pvalues[LENGTH + 4], gauss[LENGTH * 5];
static GBitmap bitmap_chunk;
static GPoint pt_location, pt_fraction;
static int n_zoom;

//  includes

//#include "app_sync.c.inc"     //  communication

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
  graphics_context_set_fill_color(context, GColorWhite);
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
    int n_altitude = atan2_lookup(-accel_data->y, -accel_data->z);
    if ((n_altitude < kAltitudeThreshhold) || (n_altitude >= TRIG_MAX_ANGLE - kAltitudeThreshhold)) {
      n_altitude = (n_altitude < TRIG_MAX_ANGLE / 2)
                     ? kAltitudeThreshhold - n_altitude
                     : kAltitudeThreshhold + (TRIG_MAX_ANGLE - n_altitude);
      pt_fraction.y += (kTiltIncrement * n_altitude * kMaxFraction) / (2 * kAltitudeThreshhold);
      while (pt_fraction.y >= kMaxFraction) {
        if (pt_location.y < kRoomMaxHeight - 168 / 2) {
          pt_location.y++;
          b_refresh = true;
        }
        pt_fraction.y -= kMaxFraction;
      }
    } else if ((n_altitude > TRIG_MAX_ANGLE / 4) && (n_altitude < TRIG_MAX_ANGLE / 4 + 2 * kAltitudeThreshhold)) {
      n_altitude -= TRIG_MAX_ANGLE / 4;
      pt_fraction.y -= (kTiltIncrement * n_altitude * kMaxFraction) / (2 * kAltitudeThreshhold);
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

void handler_up(ClickRecognizerRef recognizer, void* context) {
  if (n_zoom > 0) {
    n_zoom--;
    layer_mark_dirty(layer_screen);
  }
}

void handler_select(ClickRecognizerRef recognizer, void* context) {
}

void handler_down(ClickRecognizerRef recognizer, void* context) {
  if (n_zoom < kZoomMax) {
    n_zoom++;
    layer_mark_dirty(layer_screen);
  }
}

void config_provider(void* context) {
  //  set select handlers
  window_single_click_subscribe(BUTTON_ID_SELECT, handler_select);
  window_single_click_subscribe(BUTTON_ID_UP, handler_up);
  window_single_click_subscribe(BUTTON_ID_DOWN, handler_down);
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
  //  weather
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
  pt_location = GPoint(kRoomMaxWidth / 2, kRoomMaxHeight / 2);
  pt_fraction = GPoint(kMaxFraction / 2, kMaxFraction / 2);
  n_zoom = 1;
  memset(pixels, '\0', sizeof(pixels));
  memset(gauss, '\0', sizeof(gauss));
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
  window_set_click_config_provider(window, config_provider);
  window_stack_push(window, true);
  //  backlight
#if  kDebug
  light_enable(true);
#endif
  //  main event loop
  app_event_loop();
  //  backlight off
#if  kDebug
  light_enable(false);
#endif
  //  destroy window
  window_destroy(window);
}
