/**
 * File:   input_thread.c
 * Author: AWTK Develop Team
 * Brief:  thread to read mouse events
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
 * 2018-10-02 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <mouse.h>
#include "base/mem.h"
#include "base/keys.h"
#include "base/thread.h"
#include "input_thread.h"

typedef struct _run_info_t {
  int fd;
  int32_t x;
  int32_t y;
  bool_t pressed;
  int32_t max_x;
  int32_t max_y;
  void* dispatch_ctx;
  input_dispatch_t dispatch;

  event_queue_req_t req;
} run_info_t;

static ret_t input_dispatch(run_info_t* info) {
  ret_t ret = info->dispatch(info->dispatch_ctx, &(info->req));
  info->req.event.type = EVT_NONE;

  return ret;
}

static ret_t input_dispatch_one_event(run_info_t* info) {
    mouse_event_notify      mse_event;
    event_queue_req_t* req = &(info->req);
    int ret = read(info->fd, (void *)&mse_event, sizeof(mouse_event_notify));
    int x = mse_event.xmovement*info->max_x/936;
    int y = (936-mse_event.ymovement)*info->max_y/936;

    if (ret < 0) {
        fprintf(stderr, "read mouse event error, abort.\n");
        return RET_FAIL;
    }
    if (ret < sizeof(mouse_event_notify)) {
        fprintf(stderr, "read mouse event invalid, continue.\n");
        return RET_FAIL;
    }

    info->x = x;
    info->y = y;
    req->pointer_event.x = x;
    req->pointer_event.y = y;

    if (mse_event.kstat & MOUSE_LEFT) {
        if(info->pressed) {
            req->pointer_event.e.type = EVT_POINTER_MOVE;
        } else {
            req->pointer_event.e.type = EVT_POINTER_DOWN;
        }
        info->pressed = TRUE;
    } else {
        if(info->pressed) {
            req->pointer_event.e.type = EVT_POINTER_UP;
        } else {
            req->pointer_event.e.type = EVT_POINTER_MOVE;
        }
        info->pressed = FALSE;
    }

    input_dispatch(info);

    return RET_OK;
}

void* input_run(void* ctx) {
  run_info_t* info = (run_info_t*)ctx;

  while (input_dispatch_one_event(info) == RET_OK)
    ;

  close(info->fd);
  TKMEM_FREE(info);

  return NULL;
}

static run_info_t* info_dup(run_info_t* info) {
  run_info_t* new_info = TKMEM_ZALLOC(run_info_t);

  *new_info = *info;

  return new_info;
}

thread_t* input_thread_run(const char* filename, input_dispatch_t dispatch, void* ctx,
                           int32_t max_x, int32_t max_y) {
  run_info_t info;
  thread_t* thread = NULL;
  return_value_if_fail(filename != NULL && dispatch != NULL, NULL);

  memset(&info, 0x00, sizeof(info));

  info.max_x = max_x;
  info.max_y = max_y;
  info.dispatch_ctx = ctx;
  info.dispatch = dispatch;
  info.fd = open(filename, O_RDONLY);

  return_value_if_fail(info.fd >= 0, NULL);

  thread = thread_create(input_run, info_dup(&info));
  if (thread != NULL) {
    thread_start(thread);
  } else {
    close(info.fd);
  }

  return thread;
}
