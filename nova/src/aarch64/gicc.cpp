/*
 * Generic Interrupt Controller: Physical CPU Interface (GICC/ICC)
 *
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

#include "acpi.hpp"
#include "assert.hpp"
#include "gicc.hpp"
#include "space_hst.hpp"
#include "stdio.hpp"

Gicc::Mode Gicc::mode { Mode::MMIO };

void Gicc::init()
{
    init_mode();

    if (!Acpi::resume && Cpu::bsp && mode == Mode::MMIO)
        mmap_mmio();

    switch (mode) {
        case Mode::MMIO: return init_mmio();
        case Mode::REGS: return init_regs();
    }
}

void Gicc::mmap_mmio()
{
    if (EXPECT_FALSE (!phys))
        panic ("%s failure", __PRETTY_FUNCTION__);

    // FIXME
    constexpr auto size { Board::gic[2].size };
    constexpr auto offs { Board::gic[2].size == 0x20000 ? 0xf000 : 0 };

    // Map MMIO region
    for (unsigned i { 0 }; i < 2; i++)
        Hptp::master_map (MMAP_GLB_GICC + i * PAGE_SIZE (0), phys + offs + i * PAGE_SIZE (0), 0, Paging::Permissions (Paging::G | Paging::W | Paging::R), Memattr::dev());

    // Reserve MMIO region
    Space_hst::access_ctrl (phys, size, Paging::NONE);
}

void Gicc::init_mmio()
{
    // Disable interrupt signaling
    write (Reg32::CTLR, 0);

    write (Reg32::BPR, BIT_RANGE (2, 0));
    write (Reg32::PMR, BIT_RANGE (7, 0));

    // Enable interrupt signaling
    write (Reg32::CTLR, BIT (9) | BIT (6) | BIT (5) | BIT (0));

    auto const iidr { read (Reg32::IIDR) };
    auto const arch { iidr >> 16 & BIT_RANGE (3, 0) };

    trace (TRACE_INTR, "GICC: %#010lx v%u r%up%u Impl:%#x Prod:%#x",
           phys, arch, arch, iidr >> 12 & BIT_RANGE (3, 0), iidr & BIT_RANGE (11, 0), iidr >> 20);
}

void Gicc::init_regs()
{
    set_el1_bpr1    (BIT_RANGE (2, 0));
    set_el1_pmr     (BIT_RANGE (7, 0));
    set_el1_igrpen1 (BIT (0));
    set_el1_ctlr    (BIT (1));

    // Ensure system register writes executed
    Barrier::isb();

    trace (TRACE_INTR, "GICC: REGS");
}

void Gicc::init_mode()
{
    if (!Cpu::feature (Cpu::Cpu_feature::GIC))
        return;

    // Disable IRQ/FIQ bypass and enable system registers
    set_el2_sre (get_el2_sre() | BIT_RANGE (2, 0));

    // Ensure system register write executed
    Barrier::isb();

    if (get_el2_sre() & BIT (0))
        mode = Mode::REGS;
}

void Gicc::send_cpu (unsigned sgi, cpu_t cpu)
{
    assert (sgi < NUM_SGI && mode == Mode::REGS);

    auto const mpidr { Cpu::remote_mpidr (cpu) };

    send_sgi (sgi << 24 | (mpidr & BIT64_RANGE (39, 32)) << 16 |    // Aff3
                          (mpidr & BIT64_RANGE (23, 16)) << 16 |    // Aff2
                          (mpidr & BIT64_RANGE (15,  8)) <<  8 |    // Aff1
                      BIT (mpidr & BIT64_RANGE ( 3,  0)));          // Aff0
}

void Gicc::send_exc (unsigned sgi)
{
    assert (sgi < NUM_SGI && mode == Mode::REGS);

    send_sgi (BIT64 (40) | sgi << 24);
}
