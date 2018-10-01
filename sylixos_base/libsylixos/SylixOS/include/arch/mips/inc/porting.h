/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: porting.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2017 年 11 月 28 日
**
** 描        述: linux branch, 非对齐处理, FPU 模拟移植头文件.
*********************************************************************************************************/

#ifndef __ARCH_MIPSPORTING_H
#define __ARCH_MIPSPORTING_H

/*********************************************************************************************************
  基本数据类型与定义
*********************************************************************************************************/

typedef BOOL                            bool;

#ifndef true
#define true                            LW_TRUE
#endif

#ifndef false
#define false                           LW_FALSE
#endif

enum {
    VERIFY_READ,
    VERIFY_WRITE,
};

#define __cold
#define __maybe_unused                  __attribute__((unused))

/*********************************************************************************************************
  注意: 始终认为可以访问，如果地址不合法，可能会产生异常嵌套，可以通过异常嵌套时打印的调用栈分析应用
        中有问题的代码
*********************************************************************************************************/

#define access_ok(type, va, len)        1
#define uaccess_kernel()                1
#define get_fs()                        0
#define get_ds()                        1
#define segment_eq(s1, s2)              0

#define put_user(val, va)               (*va = val, 0)
#define __put_user(val, va)             (*va = val, 0)

#define get_user(val, va)               (val = *va, 0)
#define __get_user(val, va)             (val = *va, 0)

#define cond_resched()                  do { } while (0)
#define preempt_disable()               do { } while (0)
#define preempt_enable()                do { } while (0)
#define unreachable()                   do { } while (0)
#define perf_sw_event(a, b, c, d)       do { } while (0)

/*********************************************************************************************************
  最大最小值
*********************************************************************************************************/

#define U8_MAX                          ((u8)~0U)
#define S8_MAX                          ((s8)(U8_MAX>>1))
#define S8_MIN                          ((s8)(-S8_MAX - 1))
#define U16_MAX                         ((u16)~0U)
#define S16_MAX                         ((s16)(U16_MAX>>1))
#define S16_MIN                         ((s16)(-S16_MAX - 1))
#define U32_MAX                         ((u32)~0U)
#define S32_MAX                         ((s32)(U32_MAX>>1))
#define S32_MIN                         ((s32)(-S32_MAX - 1))
#define U64_MAX                         ((u64)~0ULL)
#define S64_MAX                         ((s64)(U64_MAX>>1))
#define S64_MIN                         ((s64)(-S64_MAX - 1))

/*********************************************************************************************************
  打印信息
*********************************************************************************************************/

#define pr_info(args...)                do { } while (0)
#define pr_warn(args...)                do { } while (0)
#define pr_err(args...)                 do { } while (0)
#define pr_debug(args...)               do { } while (0)

#include "./cpu-features.h"                                             /*  包含特性头文件              */

#endif                                                                  /* __ARCH_MIPSPORTING_H         */
/*********************************************************************************************************
  END
*********************************************************************************************************/
