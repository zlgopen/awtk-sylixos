/**
 * File:   main_loop_sylixos.c
 * Author: AWTK Develop Team
 * Brief:  sylixos implemented main_loop interface
 *
 * Copyright (c) 2018 - 2018  Guangzhou ZHIYUAN Electronics Co.,Ltd.
 *
 * this program is distributed in the hope that it will be useful,
 * but without any warranty; without even the implied warranty of
 * merchantability or fitness for a particular purpose.  see the
 * license file for more details.
 *
 */

/**
 * history:
 * ================================================================
 * 2018-10-01 li xianjing <xianjimli@hotmail.com> created
 *
 */

#include <SylixOS.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "base/idle.h"
#include "base/timer.h"
#include "base/font_manager.h"
#include "base/window_manager.h"
#include "main_loop/main_loop_simple.h"

#include "input_thread.h"
#include "lcd_sylixos_fb.h"
#include "main_loop_sylixos.h"

#define FB_DEVICE_FILENAME "/dev/fb0"
#define MOUSE_DEV_NAME      "/dev/input/xmse"

static ret_t main_loop_sylixos_destroy(main_loop_t* l) {
  main_loop_simple_t* loop = (main_loop_simple_t*)l;

  main_loop_simple_reset(loop);

  return RET_OK;
}

main_loop_t* main_loop_init(int w, int h) {
  main_loop_simple_t* loop = NULL;
  lcd_t* lcd = lcd_sylixos_fb_create(FB_DEVICE_FILENAME);

  return_value_if_fail(lcd != NULL, NULL);
  loop = main_loop_simple_init(lcd->w, lcd->h);

  loop->base.destroy = main_loop_sylixos_destroy;
  canvas_init(&(loop->base.canvas), lcd, font_manager());

  input_thread_run(MOUSE_DEV_NAME, main_loop_queue_event, loop, lcd->w, lcd->h);

  return (main_loop_t*)loop;
}
