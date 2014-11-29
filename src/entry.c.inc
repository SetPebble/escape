//  file: entry.c.inc

//  constants

const GPathInfo kTriangle = {
  .num_points = 3,
  .points = (GPoint[]) {{0, -4}, {-8, 5}, {8, 5}}
};

//  variables

static Window* window_entry;
static Layer* layer_entry;
static GPath* path_triangle;

//  functions

void entry_update(Layer* layer, GContext* context) {
  //  input
  Input* inp = inputs + input;
  //  set color
  graphics_context_set_fill_color(context, GColorBlack);
  //  entry mode
  switch (inp->entry) {
    case entry_digits:
     {//  background
      graphics_fill_rect(context, GRect(1 + 36 * n_position, 30, 34, 64), 5, GCornersAll);
      //  triangles
      gpath_move_to(path_triangle, GPoint(19 + 36 * n_position, 20));
      gpath_rotate_to(path_triangle, 0);
      gpath_draw_filled(context, path_triangle);
      gpath_move_to(path_triangle, GPoint(19 + 36 * n_position, 104));
      gpath_rotate_to(path_triangle, TRIG_MAX_ANGLE / 2);
      gpath_draw_filled(context, path_triangle);
      //  loop through digits
      int value = inp->value;
      for (int i = 4;  i-- > 0;  value /= 10) {
        //  display [0,9]
        snprintf(str_temp, sizeof(str_temp), "%c", '0' + (value % 10));
        graphics_context_set_text_color(context, (i == n_position) ? GColorWhite : GColorBlack);
        graphics_draw_text(context, str_temp, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49),
                           GRect(1 + 36 * i, 30, 34, 49 + 20), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
      }
      //  determine success;  possibly set state
      bool b_correct = (inp->value == inp->correct);
      //  success or failure icon  (fail:33x40, success:44x40)
      GBitmap* bitmap_icon = gbitmap_create_with_resource(b_correct ? RESOURCE_ID_CHECK_SUCCESS : RESOURCE_ID_CHECK_FAIL);
      graphics_draw_bitmap_in_rect(context, bitmap_icon, b_correct ? GRect(50, 122, 44, 40) : GRect(56, 122, 33, 40));
      gbitmap_destroy(bitmap_icon);
     }break;
  }
}

void entry_button(ClickRecognizerRef recognizer, void* context) {
  Input* inp = inputs + input;
  //  get button
  ButtonId button = click_recognizer_get_button_id(recognizer);
  switch (inp->entry) {
    case entry_digits:
      if (button != BUTTON_ID_SELECT) {
        //  calculate new digit value
        int value = inputs[input].value;
        int digits[4];
        digits[0] = value % 10;
        digits[1] = (value / 10) % 10;
        digits[2] = (value / 100) % 10;
        digits[3] = (value / 1000) % 10;
        digits[3 - n_position] = (digits[3 - n_position] + ((button == BUTTON_ID_UP) ? 1 : 9) + 10) % 10;
        inp->value = digits[0] + (digits[1] * 10) + (digits[2] * 100) + (digits[3] * 1000);
        //  determine success;  possibly set state
        bool b_correct = (inp->value == inp->correct);
        states[inp->changes].value = b_correct;
        if (b_correct && inp->message)
          toast_show(inp->message);
      } else
        n_position = (n_position + 1) % 4;
      //  redraw
      layer_mark_dirty(layer_entry);
      break;
  }
}

void entry_config_provider(void* context) {
  //  set select handlers
  window_single_click_subscribe(BUTTON_ID_SELECT, entry_button);
  window_single_click_subscribe(BUTTON_ID_UP, entry_button);
  window_single_click_subscribe(BUTTON_ID_DOWN, entry_button);
}

void entry_load(Window* window) {
  //  wind
  path_triangle = gpath_create(&kTriangle);
  //  entry layer
  layer_entry = layer_create(GRect(0, 0, 144, 168));
  layer_set_update_proc(layer_entry, entry_update);
  layer_add_child(window_get_root_layer(window), layer_entry);
}

void entry_unload(Window* window) {
  //  free layers and objects
  layer_destroy(layer_entry);
  gpath_destroy(path_triangle);
  //  free window
  window_destroy(window_entry);
}

void entry_launch(int n_input) {
  //  initialize entry window
  window_entry = window_escape((WindowHandlers) {
    .load = entry_load,
    .unload = entry_unload
  }, entry_config_provider);
  //  remember input
  input = n_input;
  //  check success (for reentries)
  Input* inp = inputs + input;
  states[inp->changes].value = (inp->value == inp->correct);
}
