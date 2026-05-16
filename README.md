# Kernalite

**Kernalite** is a minimalist, educational operating system kernel built from scratch. The project aims to recreate core components of the Linux architecture to explore the fundamentals of operating system design and hardware-software interaction.

## 🎯 Project Goals
* **Multi-Arch Support:** Initially targeting **aarch64** (ARM64), with planned ports for **x86_64** and legacy 32-bit architectures.
* **Linux-Inspired:** Implementing core Unix-like subsystems including memory management, scheduling, and interrupt handling.
* **Educational Clarity:** Maintaining a clean, documented codebase that prioritizes readability over extreme optimization.

## 📂 Project Structure

* **`docs/`**: Technical specifications, research notes, and architectural definitions.
* **`scripts/`**: Build automation scripts and linker files (`linker.ld`) to define the kernel's memory layout.
* **`src/boot/`**: The kernel entry point.
    * *Note:* While x86 uses Multiboot/GRUB, for **aarch64**, this handles Exception Level (EL) setup and initial CPU handoff from the bootloader.
* **`src/kernel/`**: The architecture-independent logic (the "Brain").
    * **`cpu/`**: Logic for handling interrupts, exceptions, and the system tick.
    * **`mm/`**: Memory Management—responsible for physical page allocation (bitmaps) and virtual memory (paging).
    * **`sched/`**: The task scheduler and process management.
* **`src/arch/`**: Hardware-specific implementations (Drivers, MMU setup, and CPU-specific registers).
* **`src/include/`**: Global header files, mirroring the `src` directory structure for easy dependency management.

## 🛠 Building & Running

This project requires a cross-compiler (e.g., `aarch64-none-elf-gcc`) to ensure the kernel is built correctly for the target architecture regardless of your host machine.

### Build System
The project is designed to be built using:
* **Make:** For low-level control over the build process.
* **CMake:** (Optional) For modular project management as the codebase grows.

### Git Submodules
This repository depends on external sources under `external/dtc`, which is tracked as a git submodule.
When cloning this repository, initialize submodules before building:
```bash
git clone --recurse-submodules <repo-url>
cd kernalite
make all
```
If you already cloned without submodules, run:
```bash
git submodule update --init --recursive
```

### Emulation
To test Kernalite on aarch64 using QEMU:
```bash
qemu-system-aarch64 -M virt -cpu cortex-a53 -kernel build/kernalite.elf -nographic
