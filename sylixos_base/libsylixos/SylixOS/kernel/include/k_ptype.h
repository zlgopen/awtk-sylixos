/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: k_ptype.h
**
** 创   建   人: Han.Hui (韩辉)
**
** 文件创建日期: 2007 年 11 月 04 日
**
** 描        述: 这是系统 POSIX 兼容类型定义。

** BUG
2007.11.07  加入 struct timeval 结构,并加入转换为 tick 的内联函数.
2009.07.09  将 off_t 定义为 int64, 全面支持 1TB 文件, IO 系统要做出相应的调整.
2009.12.30  将 pid_t 定义为 long 型.
*********************************************************************************************************/

#ifndef __K_PTYPE_H
#define __K_PTYPE_H

#ifndef __PTYPE

/*********************************************************************************************************
  POSIX BASIC
*********************************************************************************************************/

#ifndef __errno_t_defined
typedef INT         errno_t;
#define __errno_t_defined 1
#endif

#ifndef __error_t_defined
typedef INT         error_t;
#define __error_t_defined 1
#endif

/*********************************************************************************************************
  C99 兼容类型
*********************************************************************************************************/

#ifndef __int8_t_defined
typedef INT8        int8_t;
typedef UINT8       uint8_t;
typedef UINT8       u_int8_t;
#define __int8_t_defined 1
#endif

#ifndef __int16_t_defined
typedef INT16       int16_t;
typedef UINT16      uint16_t;
typedef UINT16      u_int16_t;
#define __int16_t_defined 1
#endif

#ifndef __int32_t_defined
typedef INT         int32_t;
typedef UINT32      uint32_t;
typedef UINT        u_int32_t;
#define __int32_t_defined 1
#endif

#ifndef __int64_t_defined
typedef INT64       int64_t;
typedef UINT64      u_int64_t;
typedef UINT64      uint64_t;
#define __int64_t_defined 1
#endif

#if defined(__PTRDIFF_TYPE__)
typedef signed __PTRDIFF_TYPE__     intptr_t;
typedef unsigned __PTRDIFF_TYPE__   uintptr_t;
#else
typedef LONG        intptr_t;
typedef ULONG       uintptr_t;
#endif

/*********************************************************************************************************
  old Berkeley definitions
*********************************************************************************************************/

typedef CHAR         char_t;
typedef UCHAR        uchar_t;
typedef UINT16       ushort_t;
typedef UINT         uint_t;
typedef ULONG        ulong_t;
typedef LONG         daddr_t;
typedef PCHAR        caddr_t;
typedef LONG         swblk_t;

/*********************************************************************************************************
  address integer type
*********************************************************************************************************/

#ifndef __addr_t_defined
typedef ULONG        addr_t;
#define __addr_t_defined 1
#endif

#ifndef __ioaddr_t_defined
typedef ULONG        ioaddr_t;
#define __ioaddr_t_defined 1
#endif

/*********************************************************************************************************
  CPU ID
*********************************************************************************************************/

#ifndef __cpuid_t_defined
typedef ULONG        cpuid_t;
#define __cpuid_t_defined 1
#endif

/*********************************************************************************************************
  POSIX basic type
*********************************************************************************************************/

typedef INT64        time_t;
typedef ULONG        timer_t;
typedef INT          clockid_t;
typedef ULONG        clock_t;

typedef UINT64       u_quad_t;
typedef INT64        quad_t;

typedef ULONG        dev_t;                                             /*  in sylixos save a pointer   */
typedef UINT32       gid_t;
typedef ULONG        ino_t;
typedef UINT64       ino64_t;
typedef INT          mode_t;
typedef UINT32       nlink_t;
typedef INT64        off_t;                                             /*  64 bit off_t                */
typedef INT64        off64_t;
typedef INT64        loff_t;                                            /*  same as off_t               */
typedef INT          pid_t;
typedef UINT32       uid_t;

typedef LONG         blksize_t;
typedef LONG         blkcnt_t;
typedef INT64        blkcnt64_t;

typedef UINT         useconds_t;
typedef LONG         suseconds_t;

typedef UINT         second_t;
typedef UINT         usecond_t;

/*********************************************************************************************************
  dma
*********************************************************************************************************/

typedef ULONG        dma_addr_t;                                        /*  32/64 bit                   */
typedef UINT64       dma64_addr_t;                                      /*  64 bit                      */

/*********************************************************************************************************
  phys addr (for ioremap second version)
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
#if (LW_CFG_CPU_WORD_LENGHT == 32) && (LW_CFG_CPU_PHYS_ADDR_64BIT > 0)
typedef UINT64       phys_addr_t;
#else
typedef ULONG        phys_addr_t;
#endif
#endif

/*********************************************************************************************************
  BSD SOCKET basic type
*********************************************************************************************************/

#if !defined(socklen_t) && !defined(SOCKLEN_T_DEFINED) && \
    !defined(__socklen_t_defined) && !defined(_SOCKLEN_T)
typedef UINT         socklen_t;
#define socklen_t    UINT
#define SOCKLEN_T_DEFINED 1
#define __socklen_t_defined 1
#define _SOCKLEN_T 1
#endif                                                                  /*  socklen_t                   */

typedef UCHAR        u_char;
typedef UINT16       u_short;
typedef UINT         u_int;
typedef ULONG        u_long;

typedef INT          SOCKET;

/*********************************************************************************************************
  兼容 linux, 目前无任何功能
*********************************************************************************************************/

struct module {
    char           *name;
    int             id;
};

#define THIS_MODULE  LW_NULL

/*********************************************************************************************************
  POSIX TIME ns
*********************************************************************************************************/

#define __TIMEVAL_NSEC_MAX     1000000000

struct timespec {                                                       /*  POSIX struct timespec       */
    time_t                     tv_sec;                                  /*  seconds                     */
    LONG                       tv_nsec;                                 /*  nanoseconds                 */
                                                                        /*  (0 - 1,000,000,000)         */
};

/*********************************************************************************************************
  POSIX TIME us
*********************************************************************************************************/

#define __TIMEVAL_USEC_MAX     1000000

struct timeval {
    time_t                     tv_sec;                                  /*  seconds                     */
    LONG                       tv_usec;                                 /*  microseconds                */
};

/*********************************************************************************************************
  POSIX itimer
*********************************************************************************************************/

struct itimerval {
    struct timeval             it_interval;
    struct timeval             it_value;
};

/*********************************************************************************************************
  POSIX timer
*********************************************************************************************************/

struct itimerspec {
    struct timespec            it_interval;                             /*  定时器重载值                */
    struct timespec            it_value;                                /*  到下一次到期为止剩余时间    */
};

/*********************************************************************************************************
  atomic_t 类型
*********************************************************************************************************/

typedef struct {
    volatile INT    counter;
} atomic_t;

/*********************************************************************************************************
  POSIX signal
*********************************************************************************************************/

struct siginfo;

typedef VOID    (*PSIGNAL_HANDLE)(INT);                                 /*  信号处理句柄函数类型        */
typedef VOID    (*PSIGNAL_HANDLE_ACT)(INT, struct siginfo *, PVOID);

typedef UINT64    sigset_t;                                             /*  信号集类型 (64bit)          */
typedef INT       sig_atomic_t;                                         /*  信号原子操作类型            */

struct sigaction {
    union {
        PSIGNAL_HANDLE      _sa_handler;
        PSIGNAL_HANDLE_ACT  _sa_sigaction;
    } _u;                                                               /*  信号服务函数句柄            */
    sigset_t             sa_mask;                                       /*  执行时的信号屏蔽码          */
    INT                  sa_flags;                                      /*  该句柄处理标志              */
    PSIGNAL_HANDLE       sa_restorer;                                   /*  恢复处理函数指针            */
};

#define sa_handler       _u._sa_handler
#define sa_sigaction     _u._sa_sigaction

typedef PSIGNAL_HANDLE   sighandler_t;

/*********************************************************************************************************
  UNIX BSD signal
*********************************************************************************************************/

struct sigvec {
    PSIGNAL_HANDLE       sv_handler;                                    /*  信号服务函数句柄            */
    sigset_t             sv_mask;                                       /*  执行时的信号屏蔽码          */
    INT                  sv_flags;                                      /*  该句柄处理标志              */
};

/*********************************************************************************************************
  POSIX 分散缓冲区
*********************************************************************************************************/

struct iovec {
    PVOID                      iov_base;                                /*  基地址                      */
    size_t                     iov_len;                                 /*  长度                        */
};

/*********************************************************************************************************
  signal rel(sigval siginfo sigevent)
*********************************************************************************************************/

typedef union sigval {
     INT                       sival_int;
     PVOID                     sival_ptr;
} sigval_t;

typedef struct siginfo {
    INT                        si_signo;
    INT                        si_errno;
    INT                        si_code;
    
    union {
        struct {
            INT                _si_pid;
            INT                _si_uid;
        } _kill;
        
        struct {
            INT                _si_tid;
            INT                _si_overrun;
        } _timer;
        
        struct {
            INT                _si_pid;
            INT                _si_uid;
        } _rt;
        
        struct {
            INT                _si_pid;
            INT                _si_uid;
            INT                _si_status;
            clock_t            _si_utime;
            clock_t            _si_stime;
        } _sigchld;
        
        struct {
            INT                _si_band;
            INT                _si_fd;
        } _sigpoll;
    } _sifields;
#define si_pid                 _sifields._kill._si_pid
#define si_uid                 _sifields._kill._si_uid
#define si_timerid             _sifields._timer._si_tid
#define si_overrun             _sifields._timer._si_overrun
#define si_status              _sifields._sigchld._si_status
#define si_utime               _sifields._sigchld._si_utime
#define si_stime               _sifields._sigchld._si_stime
#define si_band                _sifields._sigpoll._si_band
#define si_fd                  _sifields._sigpoll._si_fd
    
    union sigval               si_value;
#define si_addr                si_value.sival_ptr                       /*  Faulting insn/memory ref    */
#define si_int                 si_value.sival_int
#define si_ptr                 si_value.sival_ptr

    ULONG                      si_pad[4];
} siginfo_t;

#if LW_CFG_POSIX_EN > 0
#include "../SylixOS/posix/include/px_pthread_attr.h"
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */

typedef struct sigevent {
    INT                        sigev_signo;
    union sigval               sigev_value;
    INT                        sigev_notify;
    
    void                     (*sigev_notify_function)(union sigval);
#if LW_CFG_POSIX_EN > 0
    pthread_attr_t            *sigev_notify_attributes;
#else
    PVOID                      sigev_notify_attributes;
#endif                                                                  /*  LW_CFG_POSIX_EN > 0         */
    LW_OBJECT_HANDLE           sigev_notify_thread_id;                  /*  Linux-specific              */
                                                                        /*  equ pthread_t               */
    ULONG                      sigev_pad[8];
} sigevent_t;

/*********************************************************************************************************
  POSIX TIME ns
*********************************************************************************************************/

struct tm {
    INT             tm_sec;                                             /* seconds after minute [0, 59] */
    INT             tm_min;                                             /* minutes after hour   [0, 59] */
    INT             tm_hour;                                            /* hours after midnight [0, 23] */
    INT             tm_mday;                                            /* day of the month     [1, 31] */
    INT             tm_mon;                                             /* months since Jan     [0, 11] */
    INT             tm_year;                                            /* years since 1900             */
    INT             tm_wday;                                            /* days since Sunday    [0, 6]  */
    INT             tm_yday;                                            /* days since January 1 [0, 365]*/
#define tm_day      tm_yday
    INT             tm_isdst;                                           /* Daylight Saving Time flag    */
                                                                        /* must zero                    */
};

#endif                                                                  /*  __PTYPE                     */

/*********************************************************************************************************
  spinlock types.
*********************************************************************************************************/
#ifdef __SYLIXOS_KERNEL

typedef union {
    volatile UINT32 SLD_uiLock;
    
    struct {
#if LW_CFG_CPU_ENDIAN > 0
        UINT16      SLDQ_usTicket;
        UINT16      SLDQ_usSvcNow;
#else
        UINT16      SLDQ_usSvcNow;
        UINT16      SLDQ_usTicket;
#endif                                                                  /*  LW_CFG_CPU_ENDIAN           */
    } q;
#define SLD_usTicket    q.SLDQ_usTicket
#define SLD_usSvcNow    q.SLDQ_usSvcNow
} SPINLOCKTYPE;

#define LW_SPINLOCK_TICKET_SHIFT    16

#endif                                                                  /*  __SYLIXOS_KERNEL            */
/*********************************************************************************************************
  spinlock_t
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
struct __lw_cpu;
#endif                                                                  /*  __SYLIXOS_KERNEL            */

typedef struct {
#ifdef __SYLIXOS_KERNEL
    SPINLOCKTYPE                SL_sltData;                             /*  自旋锁                      */
#define SL_uiLock               SL_sltData.SLD_uiLock
#define SL_usTicket             SL_sltData.SLD_usTicket
#define SL_usSvcNow             SL_sltData.SLD_usSvcNow
    volatile struct __lw_cpu   *SL_pcpuOwner;                           /*  CPU 标识                    */

#else
    volatile UINT32             SL_uiLock;                              /*  自旋锁                      */
    volatile PVOID              SL_pcpuOwner;
#endif                                                                  /*  __SYLIXOS_KERNEL            */
    
    ULONG                       SL_ulCounter;                           /*  锁定计数器                  */
    PVOID                       SL_pvReserved;                          /*  保留                        */
} spinlock_t;

#define LW_SPINLOCK_DEFINE(sl)              spinlock_t  sl
#define LW_SPINLOCK_DECLARE(sl)             spinlock_t  sl

/*********************************************************************************************************
  spinlock_ca_t
*********************************************************************************************************/

#ifdef __SYLIXOS_KERNEL
#if LW_CFG_SMP_EN > 0 && LW_CFG_CPU_ARCH_CACHE_LINE > 0
typedef struct {
    union {
        spinlock_t              SLUCA_sl;
        UINT8                   SLUCA_ucPad[LW_CFG_CPU_ARCH_CACHE_LINE];
    } u;
#define SLCA_sl                 u.SLUCA_sl
} spinlock_ca_t;

#else
typedef struct {
    spinlock_t                  SLCA_sl;
} spinlock_ca_t;
#endif                                                                  /*  LW_CFG_CPU_ARCH_CACHE_LINE  */

#define LW_SPINLOCK_CA_DEFINE(slca)             spinlock_ca_t  slca
#define LW_SPINLOCK_CA_DEFINE_CACHE_ALIGN(slca) spinlock_ca_t  slca LW_CACHE_LINE_ALIGN
#define LW_SPINLOCK_CA_DECLARE(slca)            spinlock_ca_t  slca
#endif                                                                  /*  __SYLIXOS_KERNEL            */

#endif                                                                  /*  __K_PTYPE_H                 */
/*********************************************************************************************************
  END
*********************************************************************************************************/
