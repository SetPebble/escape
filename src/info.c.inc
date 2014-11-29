//  file: info.c.inc

//  constants

const int kScrollMax = 4000;                //  maximum size
const int kScrollHorizontalPadding = 4;     //  pixel padding
const int kScrollVerticalPadding = 4;       //  pixel padding
const char* kInfoContents = "Here is the help text...";

//  variables

static Window* window_info;
static ScrollLayer* scroll_content;
static TextLayer* text_content;

//  functions

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
  //  free window
  window_destroy(window_info);
}

void info_launch(void) {
  //  initialize help window
  window_info = window_escape((WindowHandlers) {
    .load = info_load,
    .unload = info_unload
  }, NULL);
}