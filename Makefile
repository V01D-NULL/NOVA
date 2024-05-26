# Nuke built-in rules and variables.
override MAKEFLAGS += -rR

override IMAGE_NAME := hypervisor

# Convenience macro to reliably declare user overridable variables.
define DEFAULT_VAR =
    ifeq ($(origin $1),default)
        override $(1) := $(2)
    endif
    ifeq ($(origin $1),undefined)
        override $(1) := $(2)
    endif
endef

.PHONY: all
all: $(IMAGE_NAME).iso

.PHONY: run
run: $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -cdrom $(IMAGE_NAME).iso -boot d -serial stdio

.PHONY: run-uefi
run-uefi: ovmf $(IMAGE_NAME).iso
	qemu-system-x86_64 -M q35 -m 2G -bios ovmf/OVMF.fd -cdrom $(IMAGE_NAME).iso -boot d -enable-kvm -serial stdio

ovmf:
	mkdir -p ovmf
	cd ovmf && curl -Lo OVMF.fd https://retrage.github.io/edk2-nightly/bin/RELEASEX64_OVMF.fd

.PHONY: hypervisor
hypervisor:
	$(MAKE) -C nova

.PHONY: eclipse
eclipse:
	$(MAKE) -C eclipse

$(IMAGE_NAME).iso: hypervisor eclipse
	@rm -rf iso_root
	@mkdir -p iso_root/boot/grub
	@cp -v nova/build-x86_64/x86_64-nova iso_root/boot/
	
	@mkdir -p iso_root/boot/grub/EFI/BOOT
	@cp -v eclipse/eclipse.elf iso_root/boot
	@cp -v grub.cfg iso_root/boot/grub/grub.cfg

	grub-mkstandalone -O x86_64-efi -o iso_root/boot/grub/EFI/BOOT/BOOTX64.EFI \
		--modules="part_gpt part_msdos" \
		"boot/grub/grub.cfg=iso_root/boot/grub/grub.cfg"

# 	Create the ISO
	grub-mkrescue -o $(IMAGE_NAME).iso iso_root/ --modules="multiboot2"

#	Verify the ISO
	xorriso -indev $(IMAGE_NAME).iso -report_el_torito as_mkisofs

	@rm -rf iso_root


.PHONY: clean
clean:
	rm -rf iso_root $(IMAGE_NAME).iso 
	$(MAKE) -C nova clean RM='rm -rf'

.PHONY: distclean
distclean: clean
	rm -rf ovmf
	$(MAKE) -C nova distclean