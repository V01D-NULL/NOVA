/*
 * Advanced Configuration and Power Interface (ACPI)
 *
 * Copyright (C) 2009-2011 Udo Steinberg <udo@hypervisor.org>
 * Economic rights: Technische Universitaet Dresden (Germany)
 *
 * Copyright (C) 2012-2013 Udo Steinberg, Intel Corporation.
 * Copyright (C) 2014 Udo Steinberg, FireEye, Inc.
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

#include "acpi_table_dmar.hpp"
#include "cmdline.hpp"
#include "hip.hpp"
#include "hpet.hpp"
#include "ioapic.hpp"
#include "pci.hpp"
#include "smmu.hpp"
#include "space_dma.hpp"
#include "stdio.hpp"

void Acpi_table_dmar::Remapping_drhd::parse() const
{
    auto const smmu { new Smmu { phys } };
    if (EXPECT_FALSE (!smmu))
        panic ("SMMU allocation failed");

    if (flags & BIT (0))
        Pci::Device::claim_all (smmu);

    for (auto ptr { reinterpret_cast<uintptr_t>(this + 1) }; ptr < reinterpret_cast<uintptr_t>(this) + length; ) {

        auto const s { reinterpret_cast<Scope const *>(ptr) };
        auto const d { Pci::pci (segment, s->b, s->d, s->f) };

        trace (TRACE_FIRM | TRACE_PARSE, "SMMU: %#lx Scope Type %u Device %04x:%02x:%02x.%x", uint64_t { phys }, std::to_underlying (s->type()), Pci::seg (d), Pci::bus (d), Pci::dev (d), Pci::fun (d));

        switch (s->type()) {
            case Scope::Type::PCI_EP:
            case Scope::Type::PCI_SH: Pci::Device::claim_dev (smmu, d); break;
            case Scope::Type::IOAPIC: Ioapic::claim_dev (d, s->id); break;
            case Scope::Type::HPET: Hpet::claim_dev (d, s->id); break;
            default: break;
        }

        ptr += s->length;
    }
}

void Acpi_table_dmar::Remapping_rmrr::parse() const
{
    uint64_t const b { align_dn (base,  PAGE_SIZE (0)) };
    uint64_t const l { align_up (limit, PAGE_SIZE (0)) };

    // Grant DMA read/write access to RMRR
    Space_dma::access_ctrl (b, l - b, Paging::Permissions (Paging::W | Paging::R));

    for (auto ptr { reinterpret_cast<uintptr_t>(this + 1) }; ptr < reinterpret_cast<uintptr_t>(this) + length; ) {

        auto const s { reinterpret_cast<Scope const *>(ptr) };
        auto const d { Pci::pci (segment, s->b, s->d, s->f) };

        trace (TRACE_FIRM | TRACE_PARSE, "RMRR: %#010lx-%#010lx Scope Type %u Device %04x:%02x:%02x.%x", b, l, std::to_underlying (s->type()), Pci::seg (d), Pci::bus (d), Pci::dev (d), Pci::fun (d));

        Smmu *smmu { nullptr };

        switch (s->type()) {
            case Scope::Type::PCI_EP: smmu = Pci::Device::find_smmu (d); break;
            default: break;
        }

        if (smmu && !smmu->configured (d))
            smmu->configure (&Space_dma::nova, d, false);

        ptr += s->length;
    }
}

void Acpi_table_dmar::parse() const
{
    if ((Smmu::ir = flags & BIT (0)))
        Lapic::x2apic &= !(flags & BIT (1));

    if (EXPECT_FALSE (Cmdline::nosmmu))
        return;

    for (auto ptr { reinterpret_cast<uintptr_t>(this + 1) }; ptr < reinterpret_cast<uintptr_t>(this) + table.header.length; ) {

        auto const r { reinterpret_cast<Remapping const *>(ptr) };

        switch (r->type()) {
            case Remapping::Type::DRHD: static_cast<Remapping_drhd const *>(r)->parse(); break;
            case Remapping::Type::RMRR: static_cast<Remapping_rmrr const *>(r)->parse(); break;
            default: break;
        }

        ptr += r->length;
    }

    Hip::set_feature (Hip_arch::Feature::SMMU);
}
