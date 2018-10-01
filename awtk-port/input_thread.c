/**
 * File:   input_thread.c
 * Author: AWTK Develop Team
 * Brief:  thread to read /dev/input/
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
 * 2018-09-07 Li XianJing <xianjimli@hotmail.com> created
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "base/mem.h"
#include "base/keys.h"
#include "base/thread.h"
#include "input_thread.h"

#ifndef EV_SYN
#define EV_SYN 0x00
#endif

typedef struct _run_info_t {
  int fd;
  int32_t max_x;
  int32_t max_y;
  void* dispatch_ctx;
  input_dispatch_t dispatch;

  event_queue_req_t req;
} run_info_t;

static const int32_t s_key_map[0x100] = {};

static int32_t map_key(uint8_t code) {
  return s_key_map[code];
}

static ret_t input_dispatch(run_info_t* info) {
  ret_t ret = info->dispatch(info->dispatch_ctx, &(info->req));
  info->req.event.type = EVT_NONE;

  return ret;
}

static ret_t input_dispatch_one_event(run_info_t* info) {
/*
  struct input_event e;
  event_queue_req_t* req = &(info->req);
  int ret = read(info->fd, &e, sizeof(e));

  if (ret != sizeof(e)) {
    printf("%s:%d read failed(ret=%d, errno=%d)\n", __func__, __LINE__, ret, errno);
  }

  return_value_if_fail(ret == sizeof(e), RET_FAIL);

  switch (e.type) {
    case EV_KEY: {
      if (e.code == BTN_LEFT || e.code == BTN_RIGHT || e.code == BTN_MIDDLE ||
          e.code == BTN_TOUCH) {
        req->event.type = e.value ? EVT_POINTER_DOWN : EVT_POINTER_UP;
      } else {
        req->event.type = e.value ? EVT_KEY_DOWN : EVT_KEY_UP;
        req->key_event.key = map_key(e.code);

        return input_dispatch(info);
      }

      break;
    }
    case EV_ABS: {
      switch (e.code) {
        case ABS_X: {
          req->pointer_event.x = e.value;
          break;
        }
        case ABS_Y: {
          req->pointer_event.y = e.value;
          break;
        }
        default:
          break;
      }

      if (req->event.type == EVT_NONE) {
        req->event.type = EVT_POINTER_MOVE;
      }

      break;
    }
    case EV_REL: {
      switch (e.code) {
        case REL_X: {
          req->pointer_event.x += e.value;

          if (req->pointer_event.x < 0) {
            req->pointer_event.x = 0;
          }
          if (req->pointer_event.x > info->max_x) {
            req->pointer_event.x = info->max_x;
          }
          break;
        }
        case REL_Y: {
          req->pointer_event.y += e.value;
          if (req->pointer_event.y < 0) {
            req->pointer_event.y = 0;
          }
          if (req->pointer_event.y > info->max_y) {
            req->pointer_event.y = info->max_y;
          }
          break;
        }
        default:
          break;
      }

      if (req->event.type == EVT_NONE) {
        req->event.type = EVT_POINTER_MOVE;
      }

      break;
    }
    case EV_SYN: {
      switch (req->event.type) {
        case EVT_POINTER_DOWN:
        case EVT_POINTER_MOVE:
        case EVT_POINTER_UP: {
          return input_dispatch(info);
        }
        default:
          break;
      }
      break;
    }
    default:
      break;
  }
*/
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
