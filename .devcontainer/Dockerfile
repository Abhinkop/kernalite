FROM mcr.microsoft.com/devcontainers/base:noble

RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    lld \
    llvm \
    llvm-dev \
    llvm-runtime \
    clangd \
    gcc-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    gdb-multiarch \
    qemu-system-arm \
    qemu-efi-aarch64 \
    qemu-utils \
    make \
    git \
    git-absorb \
    device-tree-compiler \
    bc \
    bison \
    flex \
    libssl-dev \
    pkg-config \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*