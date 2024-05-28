### Eclipse System Bridge

The Eclipse System Bridge (ESB) acts as the intermediary layer between the NOVA hypervisor and the Moon microkernel. It ensures seamless communication, allowing the Moon microkernel to run securely within a Trusted Execution Environment (TEE). 

#### Folder structure:
nova/ - [NOVA micro hypervisor](https://hypervisor.org) source code and documentation

eclipse/ - Root execution context running directly ontop of NOVA. It is the binding between NOVA and the Moon kernel

### Requirements:
- git
- google test (on ubuntu: `apt install libgtest-dev`)
- gcc
- ld
- nasm
- qemu-system-x86
- make
- grub-mkstandalone
- grub-mkrescue
- xorriso