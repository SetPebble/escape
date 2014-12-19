//  file: toast.c.inc

//  constants

const int kToastShow = 500;
const int kToastHide = 3500;
const int kToastMargin = 10;

//  variables

static PropertyAnimation* animation_toast = NULL;
static TextLayer* text_toast = NULL;
static char str_toast[64];
static GSize size_toast;

//  toast functions

void toast_hidden(struct Animation* animation, bool finished, void* context) {
  animation_toast = NULL;
}

void toast_shown(struct Animation* animation, bool finished, void* context) {
  animation_toast = property_animation_create_layer_frame(text_layer_get_layer(text_toast), NULL, &GRect((144 - size_toast.w) / 2, -size_toast.h, size_toast.w, size_toast.h));
  animation_set_duration(&animation_toast->animation, kToastHide);
  animation_set_curve(&animation_toast->animation, AnimationCurveEaseIn);
  animation_set_handlers(&animation_toast->animation, (AnimationHandlers) {
    .stopped = toast_hidden
  }, NULL);
  animation_schedule(&animation_toast->animation);
}

static void toast_show(const char* message) {
  //  stop any partially-started animation
  if (animation_toast) {
    if (animation_is_scheduled(&animation_toast->animation))
      animation_unschedule(&animation_toast->animation);
    property_animation_destroy(animation_toast);
  }
  //  relocate toast
  layer_set_frame(text_layer_get_layer(text_toast), GRect(0, 0, 144, 168));
  layer_set_bounds(text_layer_get_layer(text_toast), GRect(0, 0, 144, 168));
  //  check parent window
  Window* window_top = window_stack_get_top_window();
#if  kDebug
  //APP_LOG(APP_LOG_LEVEL_DEBUG, "toast_show:  window_top=0x%lx", (long) window_top);
#endif
  if (layer_get_window(text_layer_get_layer(text_toast)) != window_top) {
    layer_remove_from_parent(text_layer_get_layer(text_toast));
    layer_add_child(window_get_root_layer(window_top), text_layer_get_layer(text_toast));
  }
  //  set toast text; get size
  strcpy(str_toast, message);
  text_layer_set_text(text_toast, str_toast);
  size_toast = text_layer_get_content_size(text_toast);
  size_toast.w += 2 * kToastMargin;
  if (size_toast.w > 144)
    size_toast.w = 144;
  size_toast.h += kToastMargin;
  //  setup status display
  layer_set_frame(text_layer_get_layer(text_toast), GRect((144 - size_toast.w) / 2, -size_toast.h, size_toast.w, size_toast.h));
  layer_set_bounds(text_layer_get_layer(text_toast), GRect(0, 0, size_toast.w, size_toast.h));
  //  animate
  animation_toast = property_animation_create_layer_frame(text_layer_get_layer(text_toast), NULL, &GRect((144 - size_toast.w) / 2, 0, size_toast.w, size_toast.h));
  animation_set_duration(&animation_toast->animation, kToastShow);
  animation_set_curve(&animation_toast->animation, AnimationCurveEaseOut);
  animation_set_handlers(&animation_toast->animation, (AnimationHandlers) {
    .stopped = toast_shown
  }, NULL);
  animation_schedule(&animation_toast->animation);
}

static void toast_unload() {
  if (animation_toast) {
    if (animation_is_scheduled(&animation_toast->animation))
      animation_unschedule(&animation_toast->animation);
    property_animation_destroy(animation_toast);
    animation_toast = NULL;
  }
}

static void toast_init() {
  //  status
  text_toast = text_layer_create(GRect(0, 0, 144, 36));
  text_layer_set_text_color(text_toast, GColorWhite);
  text_layer_set_background_color(text_toast, GColorBlack);
  text_layer_set_font(text_toast, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(text_toast, GTextAlignmentCenter);
  text_layer_set_overflow_mode(text_toast, GTextOverflowModeWordWrap);
}

static void toast_deinit() {
  toast_unload();
  text_layer_destroy(text_toast);
}