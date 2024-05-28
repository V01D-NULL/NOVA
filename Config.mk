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

ARCH = x86_64
BOARD = acpi

# Control Flow Protection (hypervisor)
CFP = full