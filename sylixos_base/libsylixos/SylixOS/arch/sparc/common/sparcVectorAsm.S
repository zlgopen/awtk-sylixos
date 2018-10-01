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
;** 文   件   名: sparcVectorAsm.S
;**
;** 创   建   人: Xu.Guizhou (徐贵洲)
;**
;** 文件创建日期: 2017 年 05 月 15 日
;**
;** 描        述: SPARC 体系构架异常向量表.
;*********************************************************************************************************/

#ifndef ASSEMBLY
#define ASSEMBLY 1
#endif

#include <arch/assembler.h>
#include <config/kernel/kernel_cfg.h>
#include <config/mp/mp_cfg.h>

    FILE_BEGIN()

    IMPORT_LABEL(reset)
    IMPORT_LABEL(archDataStoreErrEntry)
    IMPORT_LABEL(archInstAccessMmuMissEntry)
    IMPORT_LABEL(archInstAccessErrEntry)
    IMPORT_LABEL(archRRegAccessErrEntry)
    IMPORT_LABEL(archInstAccessExcEntry)
    IMPORT_LABEL(archPrivInstEntry)
    IMPORT_LABEL(archIllegalInstEntry)
    IMPORT_LABEL(archFpDisableEntry)
    IMPORT_LABEL(archCpDisableEntry)
    IMPORT_LABEL(archUnimplFlushEntry)
    IMPORT_LABEL(archWatchPointDectectEntry)
    IMPORT_LABEL(archMemAddrNoAlignEntry)
    IMPORT_LABEL(archFpExcEntry)
    IMPORT_LABEL(archCpExcEntry)
    IMPORT_LABEL(archDataAccessErrEntry)
    IMPORT_LABEL(archDataAccessMmuMissEntry)
    IMPORT_LABEL(archDataAccessExcEntry)
    IMPORT_LABEL(archTagOverFlowEntry)
    IMPORT_LABEL(archDivZeroEntry)
    IMPORT_LABEL(archTrapInstEntry)
    IMPORT_LABEL(archIntEntry)
    IMPORT_LABEL(archWinOverFlowEntry)
    IMPORT_LABEL(archWinUnderFlowEntry)
    IMPORT_LABEL(archWinFlushTrapEntry)
    IMPORT_LABEL(archSysCallEntry)

    EXPORT_LABEL(vector)

;/*********************************************************************************************************
;  这里的异常向量表 vector 声明为弱符号, BSP 可以重新实现异常向量表
;*********************************************************************************************************/

    WEAK(vector)

;/*********************************************************************************************************
;  异常向量表
;*********************************************************************************************************/

    SECTION(.vector)

    .balign 4096, 0

FUNC_DEF(vector)
    RTRAP(0x00, reset );                            ;/*  00 reset trap                                   */
    TRAP( 0x01, archInstAccessExcEntry );           ;/*  01 instruction access exception                 */
    TRAP( 0x02, archIllegalInstEntry );             ;/*  02 illegal instruction                          */
    TRAP( 0x03, archPrivInstEntry );                ;/*  03 privileged instruction                       */
    TRAP( 0x04, archFpDisableEntry );               ;/*  04 fp disabled                                  */
    TRAP( 0x05, archWinOverFlowEntry );             ;/*  05 window overflow                              */
    TRAP( 0x06, archWinUnderFlowEntry );            ;/*  06 window underflow                             */
    TRAP( 0x07, archMemAddrNoAlignEntry );          ;/*  07 memory address not aligned                   */
    TRAP( 0x08, archFpExcEntry );                   ;/*  08 fp exception                                 */
    TRAP( 0x09, archDataAccessExcEntry );           ;/*  09 data access exception                        */
    TRAP( 0x0a, archTagOverFlowEntry );             ;/*  0A tag overflow                                 */
    TRAP( 0x0b, archWatchPointDectectEntry );       ;/*  0B watchpoint_detected                          */
    BAD_TRAP;                                       ;/*  0C undefined                                    */
    BAD_TRAP;                                       ;/*  0D undefined                                    */
    BAD_TRAP;                                       ;/*  0E undefined                                    */
    BAD_TRAP;                                       ;/*  0F undefined                                    */
    BAD_TRAP;                                       ;/*  10 undefined                                    */

    ;/*
    ; *  interrupt_level_n traps
    ; */
    TRAP( 0x11, archIntEntry );
    TRAP( 0x12, archIntEntry );
    TRAP( 0x13, archIntEntry );
    TRAP( 0x14, archIntEntry );
    TRAP( 0x15, archIntEntry );
    TRAP( 0x16, archIntEntry );
    TRAP( 0x17, archIntEntry );
    TRAP( 0x18, archIntEntry );
    TRAP( 0x19, archIntEntry );
    TRAP( 0x1a, archIntEntry );
    TRAP( 0x1b, archIntEntry );
    TRAP( 0x1c, archIntEntry );
    TRAP( 0x1d, archIntEntry );
    TRAP( 0x1e, archIntEntry );
    TRAP( 0x1f, archIntEntry );

    TRAP( 0x20, archRRegAccessErrEntry );           ;/*  20 r_register_access_error                      */
                                                    ;/*  register file EDAC error (LEON-FT only!)        */
    TRAP( 0x21, archInstAccessErrEntry );           ;/*  21 instruction_access_error                     */
    BAD_TRAP;                                       ;/*  22 undefined                                    */
    BAD_TRAP;                                       ;/*  23 undefined                                    */
    TRAP( 0x24, archCpDisableEntry );               ;/*  24 cp_disabled                                  */
    TRAP( 0x25, archUnimplFlushEntry );             ;/*  25 unimplemented_FLUSH                          */
    BAD_TRAP;                                       ;/*  26 undefined                                    */
    BAD_TRAP;                                       ;/*  27 undefined                                    */
    TRAP( 0x28, archCpExcEntry );                   ;/*  28 cp_exception                                 */
    TRAP( 0x29, archDataAccessErrEntry );           ;/*  29 data_access_error                            */
    TRAP( 0x2a, archDivZeroEntry );                 ;/*  2A division_by_zero                             */
    TRAP( 0x2b, archDataStoreErrEntry );            ;/*  2B data_store_error                             */
    TRAP( 0x2c, archDataAccessMmuMissEntry );       ;/*  2C data_access_MMU_miss                         */
    BAD_TRAP;                                       ;/*  2D undefined                                    */
    BAD_TRAP;                                       ;/*  2E undefined                                    */
    BAD_TRAP;                                       ;/*  2F undefined                                    */

    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  30 - 33 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  34 - 37 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  38 - 3B undefined                               */
    TRAP( 0x3c, archInstAccessMmuMissEntry );       ;/*  3C instruction_access_MMU_miss                  */
              BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  3D - 3F undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  40 - 43 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  44 - 47 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  48 - 4B undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  4C - 4F undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  50 - 53 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  54 - 57 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  58 - 5B undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  5C - 5F undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  60 - 63 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  64 - 67 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  68 - 6B undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  6C - 6F undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  70 - 73 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  74 - 77 undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  78 - 7B undefined                               */
    BAD_TRAP; BAD_TRAP; BAD_TRAP; BAD_TRAP;         ;/*  7C - 7F undefined                               */

    ;/*
    ; *  Software traps
    ; *
    ; *  NOTE: At the risk of being redundant... this is not a full
    ; *        table.  The setjmp on the SPARC requires a window flush trap
    ; *        handler and SylixOS will preserve the entries that were
    ; *        installed before.
    ; */
    TRAP( 0x80, archTrapInstEntry );                ;/*  80 syscall SW trap                              */
    TRAP( 0x81, archTrapInstEntry );                ;/*  81 breakpoint SW trap                           */
    TRAP( 0x82, archTrapInstEntry );                ;/*  81 abortpoint SW trap                           */
    TRAP( 0x83, archWinFlushTrapEntry );            ;/*  83 flush windows SW trap                        */
    TRAP( 0x84, archSysCallEntry );                 ;/*  84 syscall SW trap                              */
    TRAP( 0x85, archSysCallEntry );                 ;/*  85 syscall SW trap                              */
    TRAP( 0x86, archSysCallEntry );                 ;/*  86 syscall SW trap                              */
    TRAP( 0x87, archSysCallEntry );                 ;/*  87 syscall SW trap                              */
    TRAP( 0x88, archSysCallEntry );                 ;/*  88 syscall SW trap                              */
    TRAP( 0x89, archSysCallEntry );                 ;/*  89 syscall SW trap                              */
    TRAP( 0x8a, archSysCallEntry );                 ;/*  8A syscall SW trap                              */
    TRAP( 0x8b, archSysCallEntry );                 ;/*  8B syscall SW trap                              */
    TRAP( 0x8c, archSysCallEntry );                 ;/*  8C syscall SW trap                              */
    TRAP( 0x8d, archSysCallEntry );                 ;/*  8D syscall SW trap                              */
    TRAP( 0x8e, archSysCallEntry );                 ;/*  8E syscall SW trap                              */
    TRAP( 0x8f, archSysCallEntry );                 ;/*  8F syscall SW trap                              */
    TRAP( 0x90, archSysCallEntry );                 ;/*  90 syscall SW trap                              */
    TRAP( 0x91, archSysCallEntry );                 ;/*  91 syscall SW trap                              */
    TRAP( 0x92, archSysCallEntry );                 ;/*  92 syscall SW trap                              */
    TRAP( 0x93, archSysCallEntry );                 ;/*  93 syscall SW trap                              */
    TRAP( 0x94, archSysCallEntry );                 ;/*  94 syscall SW trap                              */
    TRAP( 0x95, archSysCallEntry );                 ;/*  95 syscall SW trap                              */
    TRAP( 0x96, archSysCallEntry );                 ;/*  96 syscall SW trap                              */
    TRAP( 0x97, archSysCallEntry );                 ;/*  97 syscall SW trap                              */
    TRAP( 0x98, archSysCallEntry );                 ;/*  98 syscall SW trap                              */
    TRAP( 0x99, archSysCallEntry );                 ;/*  99 syscall SW trap                              */
    TRAP( 0x9a, archSysCallEntry );                 ;/*  9A syscall SW trap                              */
    TRAP( 0x9b, archSysCallEntry );                 ;/*  9B syscall SW trap                              */
    TRAP( 0x9c, archSysCallEntry );                 ;/*  9C syscall SW trap                              */
    TRAP( 0x9d, archSysCallEntry );                 ;/*  9D syscall SW trap                              */
    TRAP( 0x9e, archSysCallEntry );                 ;/*  9E syscall SW trap                              */
    TRAP( 0x9f, archSysCallEntry );                 ;/*  9F syscall SW trap                              */
    TRAP( 0xa0, archSysCallEntry );                 ;/*  A0 syscall SW trap                              */
    TRAP( 0xa1, archSysCallEntry );                 ;/*  A1 syscall SW trap                              */
    TRAP( 0xa2, archSysCallEntry );                 ;/*  A2 syscall SW trap                              */
    TRAP( 0xa3, archSysCallEntry );                 ;/*  A3 syscall SW trap                              */
    TRAP( 0xa4, archSysCallEntry );                 ;/*  A4 syscall SW trap                              */
    TRAP( 0xa5, archSysCallEntry );                 ;/*  A5 syscall SW trap                              */
    TRAP( 0xa6, archSysCallEntry );                 ;/*  A6 syscall SW trap                              */
    TRAP( 0xa7, archSysCallEntry );                 ;/*  A7 syscall SW trap                              */
    TRAP( 0xa8, archSysCallEntry );                 ;/*  A8 syscall SW trap                              */
    TRAP( 0xa9, archSysCallEntry );                 ;/*  A9 syscall SW trap                              */
    TRAP( 0xaa, archSysCallEntry );                 ;/*  AA syscall SW trap                              */
    TRAP( 0xab, archSysCallEntry );                 ;/*  AB syscall SW trap                              */
    TRAP( 0xac, archSysCallEntry );                 ;/*  AC syscall SW trap                              */
    TRAP( 0xad, archSysCallEntry );                 ;/*  AD syscall SW trap                              */
    TRAP( 0xae, archSysCallEntry );                 ;/*  AE syscall SW trap                              */
    TRAP( 0xaf, archSysCallEntry );                 ;/*  AF syscall SW trap                              */
    TRAP( 0xb0, archSysCallEntry );                 ;/*  B0 syscall SW trap                              */
    TRAP( 0xb1, archSysCallEntry );                 ;/*  B1 syscall SW trap                              */
    TRAP( 0xb2, archSysCallEntry );                 ;/*  B2 syscall SW trap                              */
    TRAP( 0xb3, archSysCallEntry );                 ;/*  B3 syscall SW trap                              */
    TRAP( 0xb4, archSysCallEntry );                 ;/*  B4 syscall SW trap                              */
    TRAP( 0xb5, archSysCallEntry );                 ;/*  B5 syscall SW trap                              */
    TRAP( 0xb6, archSysCallEntry );                 ;/*  B6 syscall SW trap                              */
    TRAP( 0xb7, archSysCallEntry );                 ;/*  B7 syscall SW trap                              */
    TRAP( 0xb8, archSysCallEntry );                 ;/*  B8 syscall SW trap                              */
    TRAP( 0xb9, archSysCallEntry );                 ;/*  B9 syscall SW trap                              */
    TRAP( 0xba, archSysCallEntry );                 ;/*  BA syscall SW trap                              */
    TRAP( 0xbb, archSysCallEntry );                 ;/*  BB syscall SW trap                              */
    TRAP( 0xbc, archSysCallEntry );                 ;/*  BC syscall SW trap                              */
    TRAP( 0xbd, archSysCallEntry );                 ;/*  BD syscall SW trap                              */
    TRAP( 0xbe, archSysCallEntry );                 ;/*  BE syscall SW trap                              */
    TRAP( 0xbf, archSysCallEntry );                 ;/*  BF syscall SW trap                              */
    TRAP( 0xc0, archSysCallEntry );                 ;/*  C0 syscall SW trap                              */
    TRAP( 0xc1, archSysCallEntry );                 ;/*  C1 syscall SW trap                              */
    TRAP( 0xc2, archSysCallEntry );                 ;/*  C2 syscall SW trap                              */
    TRAP( 0xc3, archSysCallEntry );                 ;/*  C3 syscall SW trap                              */
    TRAP( 0xc4, archSysCallEntry );                 ;/*  C4 syscall SW trap                              */
    TRAP( 0xc5, archSysCallEntry );                 ;/*  C5 syscall SW trap                              */
    TRAP( 0xc6, archSysCallEntry );                 ;/*  C6 syscall SW trap                              */
    TRAP( 0xc7, archSysCallEntry );                 ;/*  C7 syscall SW trap                              */
    TRAP( 0xc8, archSysCallEntry );                 ;/*  C8 syscall SW trap                              */
    TRAP( 0xc9, archSysCallEntry );                 ;/*  C9 syscall SW trap                              */
    TRAP( 0xca, archSysCallEntry );                 ;/*  CA syscall SW trap                              */
    TRAP( 0xcb, archSysCallEntry );                 ;/*  CB syscall SW trap                              */
    TRAP( 0xcc, archSysCallEntry );                 ;/*  CC syscall SW trap                              */
    TRAP( 0xcd, archSysCallEntry );                 ;/*  CD syscall SW trap                              */
    TRAP( 0xce, archSysCallEntry );                 ;/*  CE syscall SW trap                              */
    TRAP( 0xcf, archSysCallEntry );                 ;/*  CF syscall SW trap                              */
    TRAP( 0xd0, archSysCallEntry );                 ;/*  D0 syscall SW trap                              */
    TRAP( 0xd1, archSysCallEntry );                 ;/*  D1 syscall SW trap                              */
    TRAP( 0xd2, archSysCallEntry );                 ;/*  D2 syscall SW trap                              */
    TRAP( 0xd3, archSysCallEntry );                 ;/*  D3 syscall SW trap                              */
    TRAP( 0xd4, archSysCallEntry );                 ;/*  D4 syscall SW trap                              */
    TRAP( 0xd5, archSysCallEntry );                 ;/*  D5 syscall SW trap                              */
    TRAP( 0xd6, archSysCallEntry );                 ;/*  D6 syscall SW trap                              */
    TRAP( 0xd7, archSysCallEntry );                 ;/*  D7 syscall SW trap                              */
    TRAP( 0xd8, archSysCallEntry );                 ;/*  D8 syscall SW trap                              */
    TRAP( 0xd9, archSysCallEntry );                 ;/*  D9 syscall SW trap                              */
    TRAP( 0xda, archSysCallEntry );                 ;/*  DA syscall SW trap                              */
    TRAP( 0xdb, archSysCallEntry );                 ;/*  DB syscall SW trap                              */
    TRAP( 0xdc, archSysCallEntry );                 ;/*  DC syscall SW trap                              */
    TRAP( 0xdd, archSysCallEntry );                 ;/*  DD syscall SW trap                              */
    TRAP( 0xde, archSysCallEntry );                 ;/*  DE syscall SW trap                              */
    TRAP( 0xdf, archSysCallEntry );                 ;/*  DF syscall SW trap                              */
    TRAP( 0xe0, archSysCallEntry );                 ;/*  E0 syscall SW trap                              */
    TRAP( 0xe1, archSysCallEntry );                 ;/*  E1 syscall SW trap                              */
    TRAP( 0xe2, archSysCallEntry );                 ;/*  E2 syscall SW trap                              */
    TRAP( 0xe3, archSysCallEntry );                 ;/*  E3 syscall SW trap                              */
    TRAP( 0xe4, archSysCallEntry );                 ;/*  E4 syscall SW trap                              */
    TRAP( 0xe5, archSysCallEntry );                 ;/*  E5 syscall SW trap                              */
    TRAP( 0xe6, archSysCallEntry );                 ;/*  E6 syscall SW trap                              */
    TRAP( 0xe7, archSysCallEntry );                 ;/*  E7 syscall SW trap                              */
    TRAP( 0xe8, archSysCallEntry );                 ;/*  E8 syscall SW trap                              */
    TRAP( 0xe9, archSysCallEntry );                 ;/*  E9 syscall SW trap                              */
    TRAP( 0xea, archSysCallEntry );                 ;/*  EA syscall SW trap                              */
    TRAP( 0xeb, archSysCallEntry );                 ;/*  EB syscall SW trap                              */
    TRAP( 0xec, archSysCallEntry );                 ;/*  EC syscall SW trap                              */
    TRAP( 0xed, archSysCallEntry );                 ;/*  ED syscall SW trap                              */
    TRAP( 0xee, archSysCallEntry );                 ;/*  EE syscall SW trap                              */
    TRAP( 0xef, archSysCallEntry );                 ;/*  EF syscall SW trap                              */
    TRAP( 0xf0, archSysCallEntry );                 ;/*  F0 syscall SW trap                              */
    TRAP( 0xf1, archSysCallEntry );                 ;/*  F1 syscall SW trap                              */
    TRAP( 0xf2, archSysCallEntry );                 ;/*  F2 syscall SW trap                              */
    TRAP( 0xf3, archSysCallEntry );                 ;/*  F3 syscall SW trap                              */
    TRAP( 0xf4, archSysCallEntry );                 ;/*  F4 syscall SW trap                              */
    TRAP( 0xf5, archSysCallEntry );                 ;/*  F5 syscall SW trap                              */
    TRAP( 0xf6, archSysCallEntry );                 ;/*  F6 syscall SW trap                              */
    TRAP( 0xf7, archSysCallEntry );                 ;/*  F7 syscall SW trap                              */
    TRAP( 0xf8, archSysCallEntry );                 ;/*  F8 syscall SW trap                              */
    TRAP( 0xf9, archSysCallEntry );                 ;/*  F9 syscall SW trap                              */
    TRAP( 0xfa, archSysCallEntry );                 ;/*  FA syscall SW trap                              */
    TRAP( 0xfb, archSysCallEntry );                 ;/*  FB syscall SW trap                              */
    TRAP( 0xfc, archSysCallEntry );                 ;/*  FC syscall SW trap                              */
    TRAP( 0xfd, archSysCallEntry );                 ;/*  FD syscall SW trap                              */
    TRAP( 0xfe, archSysCallEntry );                 ;/*  FE syscall SW trap                              */
    TRAP( 0xff, archSysCallEntry );                 ;/*  FF syscall SW trap                              */

    FUNC_END(vector)

    FILE_END()

;/*********************************************************************************************************
;  END
;*********************************************************************************************************/
