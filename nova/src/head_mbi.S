/*
 * Multiboot Headers
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2019-2024 Udo Steinberg, BedRock Systems, Inc.
 *
 * This file is part of the NOVA microhypervisor.
 *
 * NOVA is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * NOVA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License version 2 for more details.
 */

#include "macros.hpp"
#include "multiboot.hpp"

.section .head
.balign 8
.Lmb2:  .long   MULTIBOOT_V2_HEADER
        .long   0                  # Architecture (i386)
        .long   .Lmb1 - .Lmb2
        .long -(.Lmb1 - .Lmb2) - MULTIBOOT_V2_HEADER

        .quad   0x0000000800000006 # Alignment tag
        .quad   0x0000000800000007 # EFI boot services tag
        .quad   0x000000180000000a # Relocatable header tag
        .quad   0xffffffff00200000 # Relocatable header tag
        .quad   0x0000000200200000 # Relocatable header tag
        .balign 8
        framebuffer_tag_start:
            .short 5                                         #     ; type
            .short 1                                          #    ; flags
            .word framebuffer_tag_end - framebuffer_tag_start  #  ; size
            .word 800                                          #  ; width
            .word 600                                          #  ; height
            .word 32                                           #  ; depth
        framebuffer_tag_end:
        .balign 8
        .quad   0x0000000800000000 # Denotes end of header tags
.Lmb1:
