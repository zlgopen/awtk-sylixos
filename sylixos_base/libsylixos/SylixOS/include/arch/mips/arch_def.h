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
** 文   件   名: arch_def.h
**
** 创   建   人: Jiao.JinXing (焦进星)
**
** 文件创建日期: 2015 年 10 月 13 日
**
** 描        述: MIPS 相关定义.
*********************************************************************************************************/

#ifndef __MIPS_ARCH_DEF_H
#define __MIPS_ARCH_DEF_H

/*********************************************************************************************************
  __CONST64
*********************************************************************************************************/

#if defined(__ASSEMBLY__) || defined(ASSEMBLY)
#define __CONST64(x)                x
#else
#if LW_CFG_CPU_WORD_LENGHT == 32
#define __CONST64(x)                x ## UL
#else
#define __CONST64(x)                x ## ULL
#endif
#endif

/*********************************************************************************************************
  包含其它头文件
*********************************************************************************************************/

#include "./inc/sgidefs.h"

#if defined(__SYLIXOS_KERNEL) || defined(__ASSEMBLY__) || defined(ASSEMBLY)

#include "./inc/mipsregs.h"
#include "./inc/cacheops.h"
#include "./inc/cpu.h"

/*********************************************************************************************************
  KSEG0 KSEG1 地址转物理地址
*********************************************************************************************************/

#define MIPS_KSEG0_PA(va)           ((va) & 0x7fffffff)
#define MIPS_KSEG1_PA(va)           ((va) & 0x1fffffff)

/*********************************************************************************************************
  MIPS 指令
*********************************************************************************************************/

#if (!defined(__ASSEMBLY__)) && (!defined(ASSEMBLY))
typedef UINT32                      MIPS_INSTRUCTION;
#define MIPS_EXEC_INST(inst)        __asm__ __volatile__ (inst)
#endif                                                                  /*  !defined(__ASSEMBLY__)      */

/*********************************************************************************************************
  CP0 Status Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |C|C|C|C|R|F|R|M|P|B|T|S|M| | R |I|I|I|I|I|I|I|I|K|S|U|U|R|E|E|I|
  |U|U|U|U|P|R|E|X|X|E|S|R|M| | s |M|M|M|M|M|M|M|M|X|X|X|M|s|R|X|E| Status
  |3|2|1|0| | | | | |V| | |I| | v |7|6|5|4|3|2|1|0| | | | |v|L|L| |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_StatusCU      28                                              /*  Coprocessor enable (R/W)    */
#define M_StatusCU      (0xf << S_StatusCU)
#define S_StatusCU3     31
#define M_StatusCU3     (0x1 << S_StatusCU3)
#define S_StatusCU2     30
#define M_StatusCU2     (0x1 << S_StatusCU2)
#define S_StatusCU1     29
#define M_StatusCU1     (0x1 << S_StatusCU1)
#define S_StatusCU0     28
#define M_StatusCU0     (0x1 << S_StatusCU0)
#define S_StatusRP      27                                              /*  Enable reduced power        */
                                                                        /*  mode (R/W)                  */
#define M_StatusRP      (0x1 << S_StatusRP)
#define S_StatusFR      26                                              /*  Enable 64-bit FPRs          */
                                                                        /*  (MIPS64 only) (R/W)         */
#define M_StatusFR      (0x1 << S_StatusFR)
#define S_StatusRE      25                                              /*  Enable reverse endian (R/W) */
#define M_StatusRE      (0x1 << S_StatusRE)
#define S_StatusMX      24                                              /*  Enable access to MDMX       */
                                                                        /*  resources (MIPS64 only)(R/W)*/
#define M_StatusMX      (0x1 << S_StatusMX)
#define S_StatusPX      23                                              /*  Enable access to 64-bit     */
                                                                        /*  instructions/data           */
                                                                        /*  (MIPS64 only) (R/W)         */
#define M_StatusPX      (0x1 << S_StatusPX)
#define S_StatusBEV     22                                              /*  Enable Boot Exception       */
                                                                        /*  Vectors (R/W)               */
#define M_StatusBEV     (0x1 << S_StatusBEV)
#define S_StatusTS      21                                              /*  Denote TLB shutdown (R/W)   */
#define M_StatusTS      (0x1 << S_StatusTS)
#define S_StatusSR      20                                              /*  Denote soft reset (R/W)     */
#define M_StatusSR      (0x1 << S_StatusSR)
#define S_StatusNMI     19
#define M_StatusNMI     (0x1 << S_StatusNMI)                            /*  Denote NMI (R/W)            */
#define S_StatusIM      8                                               /*  Interrupt mask (R/W)        */
#define M_StatusIM      (0xff << S_StatusIM)
#define S_StatusIM7     15
#define M_StatusIM7     (0x1 << S_StatusIM7)
#define S_StatusIM6     14
#define M_StatusIM6     (0x1 << S_StatusIM6)
#define S_StatusIM5     13
#define M_StatusIM5     (0x1 << S_StatusIM5)
#define S_StatusIM4     12
#define M_StatusIM4     (0x1 << S_StatusIM4)
#define S_StatusIM3     11
#define M_StatusIM3     (0x1 << S_StatusIM3)
#define S_StatusIM2     10
#define M_StatusIM2     (0x1 << S_StatusIM2)
#define S_StatusIM1     9
#define M_StatusIM1     (0x1 << S_StatusIM1)
#define S_StatusIM0     8
#define M_StatusIM0     (0x1 << S_StatusIM0)
#define S_StatusKX      7                                               /*  Enable access to extended   */
                                                                        /*  kernel addresses            */
                                                                        /*  (MIPS64 only) (R/W)         */
#define M_StatusKX      (0x1 << S_StatusKX)
#define S_StatusSX      6                                               /*  Enable access to extended   */
                                                                        /*  supervisor addresses        */
                                                                        /*  (MIPS64 only) (R/W)         */
#define M_StatusSX      (0x1 << S_StatusSX)
#define S_StatusUX      5                                               /*  Enable access to extended   */
                                                                        /*  user addresses (MIPS64 only)*/
#define M_StatusUX      (0x1 << S_StatusUX)
#define S_StatusKSU     3                                               /*  Two-bit current mode (R/W)  */
#define M_StatusKSU     (0x3 << S_StatusKSU)
#define S_StatusUM      4                                               /*  User mode if supervisor mode*/
                                                                        /*  not implemented (R/W)       */
#define M_StatusUM      (0x1 << S_StatusUM)
#define S_StatusSM      3                                               /*  Supervisor mode (R/W)       */
#define M_StatusSM      (0x1 << S_StatusSM)
#define S_StatusERL     2                                               /*  Denotes error level (R/W)   */
#define M_StatusERL     (0x1 << S_StatusERL)
#define S_StatusEXL     1                                               /*  Denotes exception level(R/W)*/
#define M_StatusEXL     (0x1 << S_StatusEXL)
#define S_StatusIE      0                                               /*  Enables interrupts (R/W)    */
#define M_StatusIE      (0x1 << S_StatusIE)

/*********************************************************************************************************
  CP0 Cause Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |B| | C |       |I|W|           |I|I|I|I|I|I|I|I| |         | R |
  |D| | E | Rsvd  |V|P|    Rsvd   |P|P|P|P|P|P|P|P| | ExcCode | s | Cause
  | | |   |       | | |           |7|6|5|4|3|2|1|0| |         | v |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_CauseBD       31
#define M_CauseBD       (0x1 << S_CauseBD)
#define S_CauseCE       28
#define M_CauseCE       (0x3<< S_CauseCE)
#define S_CauseIV       23
#define M_CauseIV       (0x1 << S_CauseIV)
#define S_CauseWP       22
#define M_CauseWP       (0x1 << S_CauseWP)
#define S_CauseIP       8
#define M_CauseIP       (0xff << S_CauseIP)
#define S_CauseIPEXT    10
#define M_CauseIPEXT    (0x3f << S_CauseIPEXT)
#define S_CauseIP7      15
#define M_CauseIP7      (0x1 << S_CauseIP7)
#define S_CauseIP6      14
#define M_CauseIP6      (0x1 << S_CauseIP6)
#define S_CauseIP5      13
#define M_CauseIP5      (0x1 << S_CauseIP5)
#define S_CauseIP4      12
#define M_CauseIP4      (0x1 << S_CauseIP4)
#define S_CauseIP3      11
#define M_CauseIP3      (0x1 << S_CauseIP3)
#define S_CauseIP2      10
#define M_CauseIP2      (0x1 << S_CauseIP2)
#define S_CauseIP1      9
#define M_CauseIP1      (0x1 << S_CauseIP1)
#define S_CauseIP0      8
#define M_CauseIP0      (0x1 << S_CauseIP0)
#define S_CauseExcCode  2
#define M_CauseExcCode  (0x1f << S_CauseExcCode)

/*********************************************************************************************************
  CP0 Config Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |M|                             |B| A |  A  |               | K | Config
  | | Reserved for Implementations|E| T |  R  |    Reserved   | 0 |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_ConfigMore    31                                              /*  Additional config registers */
                                                                        /*  present (R)                 */
#define M_ConfigMore    (0x1 << S_ConfigMore)
#define S_ConfigImpl    16                                              /*  Implementation-specific     */
                                                                        /*  fields                      */
#define M_ConfigImpl    (0x7fff << S_ConfigImpl)
#define S_ConfigBE      15                                              /*  Denotes big-endian          */
                                                                        /*  operation (R)               */
#define M_ConfigBE      (0x1 << S_ConfigBE)
#define S_ConfigAT      13                                              /*  Architecture type (R)       */
#define M_ConfigAT      (0x3 << S_ConfigAT)
#define S_ConfigAR      10                                              /*  Architecture revision (R)   */
#define M_ConfigAR      (0x7 << S_ConfigAR)
#define S_ConfigMT      7                                               /*  MMU Type (R)                */
#define M_ConfigMT      (0x7 << S_ConfigMT)
#define S_ConfigK0      0                                               /*  Kseg0 coherency             */
                                                                        /*  algorithm (R/W)             */
#define M_ConfigK0      (0x7 << S_ConfigK0)

/*********************************************************************************************************
  Bits in the CP0 config register
  Generic bits
*********************************************************************************************************/

#define MIPS_CACHABLE_NO_WA          0
#define MIPS_CACHABLE_WA             1
#define MIPS_UNCACHED                2
#define MIPS_CACHABLE_NONCOHERENT    3
#define MIPS_CACHABLE_CE             4
#define MIPS_CACHABLE_COW            5
#define MIPS_CACHABLE_CUW            6
#define MIPS_CACHABLE_ACCELERATED    7

/*********************************************************************************************************
  CP0 Config1 Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |M|  MMU Size |  IS |  IL |  IA |  DS |  DL |  DA |C|M|P|W|C|E|F| Config1
  | |           |     |     |     |     |     |     |2|D|C|R|A|P|P|
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_Config1FP         0                                           /*  Denotes floating point      */
                                                                        /*  present (R)                 */
#define M_Config1FP         (0x1 << S_Config1FP)

/*********************************************************************************************************
  CP0 IntCtl Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |IPTI |IPPCI|                               |    VS   |         |IntCtl
  |     |     |            0                  |         |    0    |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_IntCtlIPPI    29
#define M_IntCtlIPPI    (0x7 << S_IntCtlIPPI)
#define S_IntCtlIPPCI   26
#define M_IntCtlIPPCI   (0x7 << S_IntCtlIPPCI)
#define S_IntCtlVS      5
#define M_IntCtlVS      (0x1f << S_IntCtlVS)

/*********************************************************************************************************
  CP0 CacheErr Register

   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |T|L|E|E|E|E|E|E|E|E|                                           |CacheErr
  |P|E|D|T|S|E|B|I|1|0|                                           |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*********************************************************************************************************/

#define S_CacheType     31                                              /*  reference type              */
#define M_CacheType     (0x1 << S_CacheType)                            /*  0=Instr, 1=Data             */
#define S_CacheLevel    30                                              /*  reference level             */
#define M_CacheLevel    (0x1 << S_CacheLevel)                           /*  0=Primary, 1=Secondary      */
#define S_CacheData     29                                              /*  data field                  */
#define M_CacheData     (0x1 << S_CacheData)                            /*  0=No error, 1=Error         */
#define S_CacheTag      28                                              /*  Tag field                   */
#define M_CacheTag      (0x1 << S_CacheTag)                             /*  0=No error, 1=Error         */
#define S_CacheBus      27                                              /*  error on bus                */
#define M_CacheBus      (0x1 << S_CacheBus)                             /*  0=No, 1=Yes                 */
#define S_CacheECC      26                                              /*  ECC error                   */
#define M_CacheECC      (0x1 << S_CacheECC)                             /*  0=No, 1=Yes                 */
#define S_CacheBoth     25                                              /*  Data & Instruction error    */
#define M_CacheBoth     (0x1 << S_CacheBoth)                            /*  0=No, 1=Yes                 */
#define S_CacheEI       24
#define M_CacheEI       (0x1 << S_CacheEI)
#define S_CacheE1       23
#define M_CacheE1       (0x1 << S_CacheE1)
#define S_CacheE0       22
#define M_CacheE0       (0x1 << S_CacheE0)

#endif                                                                  /*  defined(__SYLIXOS_KERNEL)   */
#endif                                                                  /*  __ARCH_DEF_H                */
/*********************************************************************************************************
  END
*********************************************************************************************************/
