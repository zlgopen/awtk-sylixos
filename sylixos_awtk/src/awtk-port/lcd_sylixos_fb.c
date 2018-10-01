/**
 * File:   lcd_sylixos_fb.h
 * Author: AWTK Develop Team
 * Brief:  sylixos framebuffer lcd
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * License file for more details.
 *
 */

/**
 * History:
 * ================================================================
 * 2018-10-01 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include "base/lcd.h"
#include "base/mem.h"
#include "lcd/lcd_mem_rgb565.h"
#include "lcd/lcd_mem_bgra8888.h"
#include "lcd/lcd_mem_rgba8888.h"

typedef struct _fb_info_t {
  int fd;
  void* bits;
  LW_GM_SCRINFO fix;
  LW_GM_VARINFO var;
} fb_info_t;

#define fb_width(fb)  ((fb)->var.GMVI_ulXResVirtual)
#define fb_height(fb) ((fb)->var.GMVI_ulYResVirtual)
#define fb_bpp(fb)    ((fb)->var.GMVI_ulBitsPerPixel)
#define fb_size(fb)   ((fb)->fix.GMSI_stMemSize)
#define fb_stride(fb) ((fb)->fix->GMSI_stMemSizePerLine)

static int fb_open(fb_info_t* fb, const char* filename) {
  fb->fd = open(filename, O_RDWR);
  if (fb->fd < 0) {
    return -1;
  }

  if(ioctl(fb->fd, LW_GM_GET_SCRINFO, &fb->fix) < 0) goto fail;
  if(ioctl(fb->fd, LW_GM_GET_VARINFO, &fb->var) < 0) goto fail;

  fb->bits = mmap(LW_NULL, fb->fix.GMSI_stMemSize, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);

  if (fb->bits == MAP_FAILED) {
    log_debug("map framebuffer failed.\n");
    goto fail;
  }

  memset(fb->bits, 0xff, fb_size(fb));
  printf("w=%d h=%d bbp=%d\n", fb_width(fb), fb_height(fb), fb_bpp(fb));

  return 0;
fail:
  log_debug("%s is not a framebuffer.\n", filename);
  close(fb->fd);

  return -1;
}

static void fb_close(fb_info_t* fb) {
  if (fb != NULL) {
    munmap(fb->bits, fb_size(fb));
    close(fb->fd);
  }

  return;
}

static fb_info_t s_fb;

lcd_t* lcd_sylixos_fb_create(const char* filename) {
  lcd_t* lcd = NULL;
  fb_info_t* fb = &s_fb;
  return_value_if_fail(filename != NULL, NULL);

  if (fb_open(fb, filename) == 0) {
    int w = fb_width(fb);
    int h = fb_height(fb);
    int bpp = fb_bpp(fb);
    int size = fb_size(fb);
    uint8_t* online_fb = (uint8_t*)(fb->bits);
    uint8_t* offline_fb = (uint8_t*)malloc(size);

    if(offline_fb == NULL) {
      fb_close(fb);

      return NULL;
    }

    if (bpp == 16) {
      lcd = lcd_mem_rgb565_create_double_fb(w, h, online_fb, offline_fb);
    } else if (bpp == 32) {
      lcd = lcd_mem_bgra8888_create_double_fb(w, h, online_fb, offline_fb);
    } else if (bpp == 24) {
      assert(!"not supported framebuffer format.");
    } else {
      assert(!"not supported framebuffer format.");
    }
  }

  return lcd;
}
