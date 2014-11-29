//  file: window.c.inc

//  functions

Window* window_escape(WindowHandlers window_handlers, ClickConfigProvider config_provider) {
  //  initialize help window
  Window* window = window_create();
  window_set_fullscreen(window, true);
  window_set_background_color(window, GColorWhite);
  window_set_window_handlers(window, window_handlers);
  if (config_provider)
    window_set_click_config_provider(window, config_provider);
  window_stack_push(window, true);
  return window;
}
