//  file: draw.c.inc

//  constants

const int kGaussianThreshhold3 = 11;        //  threshhold for gaussian filter (3-tap)
const int kGaussianThreshhold5 = 200;       //  threshhold for gaussian filter (5-tap)

//  variables

static uint8_t pixels[ROWBYTES * LENGTH], pvalues[LENGTH + 4], gauss[LENGTH * 5];
static GBitmap bitmap_chunk;

//  functions

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