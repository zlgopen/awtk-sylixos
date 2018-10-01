;/*********************************************************************************************************
;**
;**                                    中国软件开源组织
;**
;**                                   嵌入式实时操作系统
;**
;**                                       SylixOS(TM)
;**
;**                               Copyright  All Rights Reserved
;**
;**--------------文件信息--------------------------------------------------------------------------------
;**
;** 文   件   名: c6xContextAsm.inc
;**
;** 创   建   人: Jiao.JinXing (焦进星)
;**
;** 文件创建日期: 2017 年 03 月 17 日
;**
;** 描        述: c6x 体系构架上下文切换.
;*********************************************************************************************************/

;/*********************************************************************************************************
;
;  寄存器信息:
;
;  Non-scratch (被调用函数保护)
;       A10-A15, B10-B14 (B15)
;
;  Scratch (调用者函数保护)
;       A0-A9, A16-A31, B0-B9, B16-B31
;
;  Other (c64x)
;       AMR     Addressing mode                         任务
;         (A4-A7,B4-B7)
;       CSR     Control status                          任务      (*1)
;       GFPGFR  Galois field multiply control           任务
;       ICR     Interrupt clear                         全局
;       IER     Interrupt enable                        全局
;       IFR     Interrupt flag                          全局
;       IRP     Interrupt return pointer                任务
;       ISR     Interrupt set                           全局
;       ISTP    Interrupt service table pointer         全局
;       NRP     Nonmaskable interrupt return pointer    全局      (*2)
;       PCE1    Program counter, E1 phase               全局
;
;  Other (c64x+)
;       DIER    Debug interrupt enable                  全局
;       DNUM    DSP core number                         全局
;       ECR     Exception clear                         全局
;       EFR     Exception flag                          全局
;       GPLYA   GMPY A-side polynomial                  任务
;       GPLYB   GMPY B-side polynomial                  任务
;       IERR    Internal exception report               全局
;       ILC     Inner loop count                        任务
;       ITSR    Interrupt task state                    任务
;               (TSR is copied to ITSR on interrupt)
;       NTSR    NMI/Exception task state                全局      (*2)
;               (TSR is copied to NTSR on exception)
;       REP     Restricted entry point                  全局
;       RILC    Reload inner loop count                 任务
;       SSR     Saturation status                       任务      (*1)
;       TSCH    Time-stamp counter (high 32)            全局
;       TSCL    Time-stamp counter (low 32)             全局
;       TSR     Task state                              全局
;
;  Other (c66x)
;       FADCR   Floating point adder configuration      任务
;       FAUCR   Floating point auxiliary configuration  任务
;       FMCR    Floating point multiplier configuration 任务
;
;  (*1) 查看 sprugh7 中关于 CSR 的 SAT 位描述.
;  (*2) 不可屏蔽中断/异常需要处理.
;
;*********************************************************************************************************/

;/*********************************************************************************************************
;
;  EABI:
;
;  可用于条件的寄存器有 A0、A1、A2、B0、B1、B2
;
;  前 10 个入口参数使用寄存器 A4、B4、A6、B6、A8、B8、A10、B10、A12、B12
;
;  返回值使用寄存器 A4
;
;*********************************************************************************************************/

;/*********************************************************************************************************
;
;  汇编预定义宏:
;
;  .TMS320C6X          Always set to 1
;  .TMS320C6200        Set to 1 if target is C6200, otherwise 0
;  .TMS320C6400        Set to 1 if target is C6400, C6400+, C6740, or C6600; otherwise 0
;  .TMS320C6400_PLUS   Set to 1 if target is C6400+, C6740, or C6600; otherwise 0
;  .TMS320C6600        Set to 1 if target is C6600, otherwise 0
;  .TMS320C6700        Set to 1 if target is C6700, C6700+, C6740, or C6600; otherwise 0
;  .TMS320C6700_PLUS   Set to 1 if target is C6700+, C6740, or C6600; otherwise 0
;  .TMS320C6740        Set to 1 if target is C6740 or C6600, otherwise 0
;
;*********************************************************************************************************/

;/*********************************************************************************************************
; 寄存器上下文大小(不包含最后的 Reserved4)
;*********************************************************************************************************/

ARCH_REG_CTX_SIZE  .set    (328 - 4)

;/*********************************************************************************************************
;  保存一个小寄存器上下文(参数 A1: ARCH_REG_CTX 地址)
;  破坏 A0 B1 B7-B9 B16-B24(调用者函数保护)
;*********************************************************************************************************/

SAVE_SMALL_REG_CTX  .macro
    MVK     0    , A0
    STW     A0   , *A1++(8)             ;/*  小寄存器上下文类型(跳过 Reserved1)                          */

    ADD     8    , A1 , B1              ;/*  B1 与 A1 错开 8 个字节                                      */

    STDW    A15:A14 , *A1++[2]          ;/*  保存 A15:A14                                                */
 || STDW    B15:B14 , *B1++[2]          ;/*  保存 B15:B14                                                */

    STDW    A13:A12 , *A1++[2]          ;/*  保存 A13:A12                                                */
 || STDW    B13:B12 , *B1++[2]          ;/*  保存 B13:B12                                                */

    STDW    A11:A10 , *A1++[2]          ;/*  保存 A11:A10                                                */
 || STDW    B11:B10 , *B1++[2]          ;/*  保存 B11:B10                                                */

    MVC     CSR  , B9
    STW     B9   , *B1++                ;/*  CSR                                                         */
 || MVC     AMR  , B8
    STW     B8   , *B1++                ;/*  AMR                                                         */
 || MVC     GFPGFR, B7
    STW     B7   , *B1++                ;/*  GFPGFR                                                      */

    STW     B3   , *B1++(8)             ;/*  B3 代替 IRP 保存(跳过 Reserved2)                            */

    .if (.TMS320C6740)
    MVC     FMCR , B24
    STW     B24  , *B1++                ;/*  FMCR                                                        */
 || MVC     FAUCR, B23
    STW     B23  , *B1++                ;/*  FAUCR                                                       */
 || MVC     FADCR, B22
    STW     B22  , *B1++                ;/*  FADCR                                                       */
    .else
    ADDK    +12  , B1
    .endif

    MVC     SSR  , B21
    STW     B21  , *B1++                ;/*  SSR                                                         */
 || MVC     RILC , B20
    STW     B20  , *B1++                ;/*  RILC                                                        */
 || MVC     ITSR , B19
    STW     B19  , *B1++                ;/*  ITSR                                                        */
 || MVC     GPLYB, B18
    STW     B18  , *B1++                ;/*  GPLYB                                                       */
 || MVC     GPLYA, B17
    STW     B17  , *B1++                ;/*  GPLYA                                                       */
 || MVC     ILC  , B16
    STW     B16  , *B1++                ;/*  ILC                                                         */
    .endm

;/*********************************************************************************************************
;  恢复一个小寄存器上下文(参数 A1: ARCH_REG_CTX 地址)
;  破坏 B1 B3 B7-B9 B16-B24(调用者函数保护)
;*********************************************************************************************************/

RESTORE_SMALL_REG_CTX  .macro
    ADDK    +8   , A1                   ;/*  跳过 ContextType & Reserved1                                */
    ADD     8    , A1 , B1              ;/*  B1 与 A1 错开 8 个字节                                      */

    ;/*
    ; * 避免并行操作, 因为 DMA/L1D 的硬件 BUG
    ; */
    LDDW    *A1++[2] , A15:A14          ;/*  恢复 A15:A14                                                */
    LDDW    *B1++[2] , B15:B14          ;/*  恢复 B15:B14                                                */

    LDDW    *A1++[2] , A13:A12          ;/*  恢复 A13:A12                                                */
    LDDW    *B1++[2] , B13:B12          ;/*  恢复 B13:B12                                                */

    LDDW    *A1++[2] , A11:A10          ;/*  恢复 A11:A10                                                */
    LDDW    *B1++[2] , B11:B10          ;/*  恢复 B11:B10                                                */

    LDDW    *A1++[2] , A5:A4            ;/*  恢复 A5:A4                                                  */

    LDW     *B1++ , B9                  ;/*  CSR                                                         */
    LDW     *B1++ , B8                  ;/*  AMR                                                         */
    LDW     *B1++ , B7                  ;/*  GFPGFR                                                      */
    LDW     *B1++ , B3                  ;/*  B3                                                          */
    ADDK    +4 , B1                     ;/*  Reserved2                                                   */
    CLR     B9  , 9  , 9  , B9          ;/*  清除 SAT 位                                                 */
    MVC     B8 , AMR
    MVC     B7 , GFPGFR

    .if (.TMS320C6740)
    LDW     *B1++ , B24                 ;/*  FMCR                                                        */
    LDW     *B1++ , B23                 ;/*  FAUCR                                                       */
    LDW     *B1++ , B22                 ;/*  FADCR                                                       */
    NOP     2
    MVC     B24 , FMCR
    MVC     B23 , FAUCR
    MVC     B22 , FADCR
    .else
    ADDK    +12 , B1
    .endif

    LDW     *B1++ , B21                 ;/*  SSR                                                         */
    LDW     *B1++ , B20                 ;/*  RILC                                                        */
    LDW     *B1++ , B19                 ;/*  ITSR                                                        */
    LDW     *B1++ , B18                 ;/*  GPLYB                                                       */
    LDW     *B1++ , B17                 ;/*  GPLYA                                                       */
    LDW     *B1++ , B16                 ;/*  ILC                                                         */
 || MVC     B21 , SSR
    MVC     B20 , RILC
    B       B3                          ;/*  返回                                                        */
    MVC     B19 , ITSR
    MVC     B18 , GPLYB
    MVC     B17 , GPLYA
    MVC     B16 , ILC
    MVC     B9  , CSR                   ;/*  最后同时恢复 CSR                                            */
    .endm

;/*********************************************************************************************************
;  保存一个大寄存器上下文(参数 B1: ARCH_REG_CTX ARCH_REG_PAIR(B3, B2) 低地址)
;*********************************************************************************************************/

SAVE_BIG_REG_CTX  .macro    RP_REG
    STDW    B3:B2 , *B1--[2]
 || STDW    A3:A2 , *A1--[2]

    STDW    B5:B4 , *B1--[2]
 || STDW    A7:A6 , *A1--[2]

    STDW    B7:B6 , *B1--[2]
 || STDW    A9:A8 , *A1--[2]

    STDW    B9:B8 , *B1--[2]
 || STDW    A17:A16 , *A1--[2]

    STDW    B17:B16 , *B1--[2]
 || STDW    A19:A18 , *A1--[2]

    STDW    B19:B18 , *B1--[2]
 || STDW    A21:A20 , *A1--[2]

    STDW    B21:B20 , *B1--[2]
 || STDW    A23:A22 , *A1--[2]

    STDW    B23:B22 , *B1--[2]
 || STDW    A25:A24 , *A1--[2]

    STDW    B25:B24 , *B1--[2]
 || STDW    A27:A26 , *A1--[2]

    STDW    B27:B26 , *B1--[2]
 || STDW    A29:A28 , *A1--[2]

    STDW    B29:B28 , *B1--[2]
 || STDW    A31:A30 , *A1--[1]          ;/*  A1 最后指向 ARCH_REG_PAIR(B31, B30) 低地址                  */

    STDW    B31:B30 , *B1--[2]

    ADDK    -4 , A1                     ;/*  A1 指向 ILC 地址                                            */

    MVC     ILC , B16
    STW     B16 , *A1--                 ;/*  ILC                                                         */
 || MVC     GPLYA , B17
    STW     B17 , *A1--                 ;/*  GPLYA                                                       */
 || MVC     GPLYB , B18
    STW     B18 , *A1--                 ;/*  GPLYB                                                       */
 || MVC     ITSR , B19
    STW     B19 , *A1--                 ;/*  ITSR                                                        */
 || MVC     RILC , B20
    STW     B20 , *A1--                 ;/*  RILC                                                        */
 || MVC     SSR , B21
    STW     B21 , *A1--                 ;/*  SSR                                                         */

    .if (.TMS320C6740)
    MVC     FADCR , B22
    STW     B22 , *A1--                 ;/*  FADCR                                                       */
 || MVC     FAUCR , B23
    STW     B23 , *A1--                 ;/*  FAUCR                                                       */
 || MVC     FMCR , B24
    STW     B24 , *A1--(8)              ;/*  FMCR(跳过 Reserved2)                                        */
    .else
    ADDK    -16 , A1
    .endif

    MVC     RP_REG , B25
    STW     B25 , *A1--                 ;/*  IRP                                                         */
 || MVC     GFPGFR , B26
    STW     B26 , *A1--                 ;/*  GFPGFR                                                      */
 || MVC     AMR , B27
    STW     B27 , *A1--                 ;/*  AMR                                                         */
 || MVC     CSR , B28
    STW     B28 , *A1--(8)              ;/*  CSR, A1 最后指向 ARCH_REG_PAIR(A5, A4) 低地址               */

    ADD     -8 , A1 , B1                ;/*  B1 与 A1 错开 8 个字节                                      */

    STDW    A5:A4 , *A1--[2]

    STDW    B11:B10 , *B1--[2]
 || STDW    A11:A10 , *A1--[2]

    STDW    B13:B12 , *B1--[2]
 || STDW    A13:A12 , *A1--[2]

    STDW    B15:B14 , *B1--[2]
 || STDW    A15:A14 , *A1--[1]

    MVK     1 , B1
    STW     B1 , *+A1(0)                ;/*  大寄存器上下文类型                                          */
    .endm

;/*********************************************************************************************************
;  IRQ 保存一个大寄存器上下文(参数 B1: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

IRQ_SAVE_BIG_REG_CTX  .macro    RP_REG
    LDW     *+B15(8) , B0               ;/*  从栈中取出 B1 到 B0                                         */
    ADDK    +(ARCH_REG_CTX_SIZE - 4) , B1   ;/*  B1 指向 REG_uiB1 地址                                   */
    NOP     3
    STW     B0 , *B1--[1]               ;/*  存储 B1 到 ARCH_REG_CTX.REG_uiB1                            */

    LDW     *+B15(4) , B0               ;/*  从栈中取出 B0 到 B0                                         */
    ADDK    +8 , B15                    ;/*  将 B15 调整回去                                             */
    NOP     3
    STW     B0 , *B1--(12)              ;/*  存储 B0 到 ARCH_REG_CTX.REG_uiB0                            */
                                        ;/*  B1 最后指向 ARCH_REG_PAIR(A1, A0) 低地址                    */
    STDW    A1:A0 , *B1--[1]            ;/*  存储 A1:A0,B1 最后指向 ARCH_REG_PAIR(B3, B2) 低地址         */

    ADD     -8 , B1 , A1                ;/*  B1 与 A1 错开 8 个字节                                      */

    SAVE_BIG_REG_CTX    RP_REG
    .endm

;/*********************************************************************************************************
;  IRQ 嵌套保存一个大寄存器上下文
;*********************************************************************************************************/

IRQ_NEST_SAVE_BIG_REG_CTX  .macro    RP_REG
    ADDK    -8 , B15                    ;/*  B15 最后指向 ARCH_REG_PAIR(A1, A0) 低地址                   */
    STDW    A1:A0 , *B15--[1]           ;/*  存储 A1:A0,B1 最后指向 ARCH_REG_PAIR(B3, B2) 低地址         */

    MV      B15 , B1
    ADD     -8 , B1 , A1                ;/*  B1 与 A1 错开 8 个字节                                      */

    ADDK    +24 , B15                   ;/*  将 B15 调整回中断前                                         */

    SAVE_BIG_REG_CTX    RP_REG

    ADD     -8 , A1 , B15               ;/*  B15 使用 8 字节对齐的空栈                                   */
    .endm

;/*********************************************************************************************************
;  恢复一个大寄存器上下文(参数 A1: ARCH_REG_CTX 地址)
;*********************************************************************************************************/

RESTORE_BIG_REG_CTX  .macro
    ADDK    +8   , A1                   ;/*  跳过 ContextType & Reserved1                                */
    ADD     8    , A1 , B1              ;/*  B1 与 A1 错开 8 个字节                                      */

    LDDW    *A1++[2] , A15:A14          ;/*  恢复 A15:A14                                                */
    LDDW    *B1++[2] , B15:B14          ;/*  恢复 B15:B14                                                */

    LDDW    *A1++[2] , A13:A12          ;/*  恢复 A13:A12                                                */
    LDDW    *B1++[2] , B13:B12          ;/*  恢复 B13:B12                                                */

    LDDW    *A1++[2] , A11:A10          ;/*  恢复 A11:A10                                                */
    LDDW    *B1++[2] , B11:B10          ;/*  恢复 B11:B10                                                */

    LDDW    *A1++[2] , A5:A4            ;/*  恢复 A5:A4                                                  */

    LDW     *B1++ , B9                  ;/*  CSR                                                         */
    LDW     *B1++ , B8                  ;/*  AMR                                                         */
    LDW     *B1++ , B7                  ;/*  GFPGFR                                                      */
    LDW     *B1++ , B6                  ;/*  IRP                                                         */

    ADDK    +4 , B1                     ;/*  Reserved2                                                   */

    CLR     B9  , 9  , 9  , B9          ;/*  清除 SAT 位                                                 */
    MVC     B9 , CSR                    ;/*  并不会开中断                                                */
    MVC     B8 , AMR
    MVC     B7 , GFPGFR
    MVC     B6 , IRP

    .if (.TMS320C6740)
    LDW     *B1++ , B24                 ;/*  FMCR                                                        */
    LDW     *B1++ , B23                 ;/*  FAUCR                                                       */
    LDW     *B1++ , B22                 ;/*  FADCR                                                       */
    NOP     2
    MVC     B24 , FMCR
    MVC     B23 , FAUCR
    MVC     B22 , FADCR
    .else
    ADDK    +12 , B1
    .endif

    LDW     *B1++ , B21                 ;/*  SSR                                                         */
    LDW     *B1++ , B20                 ;/*  RILC                                                        */
    LDW     *B1++ , B19                 ;/*  ITSR                                                        */
    LDW     *B1++ , B18                 ;/*  GPLYB                                                       */
    LDW     *B1++ , B17                 ;/*  GPLYA                                                       */
    LDW     *B1++ , B16                 ;/*  ILC                                                         */
 || MVC     B21 , SSR
    MVC     B20 , RILC
    MVC     B19 , ITSR
    MVC     B18 , GPLYB
    MVC     B17 , GPLYA
    MVC     B16 , ILC

    ADD     8    , B1 , A1              ;/*  A1 与 B1 错开 8 个字节                                      */

    LDDW    *B1++[2] , B31:B30          ;/*  恢复 B31:B30                                                */
    LDDW    *A1++[2] , A31:A30          ;/*  恢复 A31:A30                                                */

    LDDW    *B1++[2] , B29:B28          ;/*  恢复 B29:B28                                                */
    LDDW    *A1++[2] , A29:A28          ;/*  恢复 A29:A28                                                */

    LDDW    *B1++[2] , B27:B26          ;/*  恢复 B27:B26                                                */
    LDDW    *A1++[2] , A27:A26          ;/*  恢复 A27:A26                                                */

    LDDW    *B1++[2] , B25:B24          ;/*  恢复 B25:B24                                                */
    LDDW    *A1++[2] , A25:A24          ;/*  恢复 A25:A24                                                */

    LDDW    *B1++[2] , B23:B22          ;/*  恢复 B23:B22                                                */
    LDDW    *A1++[2] , A23:A22          ;/*  恢复 A23:A22                                                */

    LDDW    *B1++[2] , B21:B20          ;/*  恢复 B21:B20                                                */
    LDDW    *A1++[2] , A21:A20          ;/*  恢复 A21:A20                                                */

    LDDW    *B1++[2] , B19:B18          ;/*  恢复 B19:B18                                                */
    LDDW    *A1++[2] , A19:A18          ;/*  恢复 A19:A18                                                */

    LDDW    *B1++[2] , B17:B16          ;/*  恢复 B17:B16                                                */
    LDDW    *A1++[2] , A17:A16          ;/*  恢复 A17:A16                                                */

    LDDW    *B1++[2] , B9:B8            ;/*  恢复 B9:B8                                                  */
    LDDW    *A1++[2] , A9:A8            ;/*  恢复 A9:A8                                                  */

    LDDW    *B1++[2] , B7:B6            ;/*  恢复 B7:B6                                                  */
    LDDW    *A1++[2] , A7:A6            ;/*  恢复 A7:A6                                                  */

    LDDW    *B1++[2] , B5:B4            ;/*  恢复 B5:B4                                                  */
    LDDW    *A1++[2] , A3:A2            ;/*  恢复 A3:A2                                                  */

    LDDW    *B1++[2] , B3:B2            ;/*  恢复 B3:B2                                                  */
    LDDW    *A1      , A1:A0            ;/*  恢复 A1:A0                                                  */

    B       IRP
 || LDW     *+B1(4)  , B0               ;/*  恢复 B1:B0                                                  */
    LDW     *+B1(8)  , B1
    NOP     4
    .endm

;/*********************************************************************************************************
;  IRQ 嵌套恢复一个大寄存器上下文
;*********************************************************************************************************/

IRQ_NEST_RESTORE_BIG_REG_CTX  .macro
    ADD     +8 , B15 , A1               ;/*  参数 A1: ARCH_REG_CTX 地址(B15 使用 8 字节对齐的空栈, 见上) */
    RESTORE_BIG_REG_CTX
    .endm

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
