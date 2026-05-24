FROM mcr.microsoft.com/devcontainers/base:noble

ARG ARM_TOOLCHAIN_VERSION=13.3.rel1
ARG TOOLCHAIN_DIR=/opt/arm-toolchain
ARG USERNAME=vscode

# Set up command history to be shared across sessions
RUN SNIPPET="export PROMPT_COMMAND='history -a' && export HISTFILE=/commandhistory/.bash_history" \
    && mkdir /commandhistory \
    && touch /commandhistory/.bash_history \
    && chown -R $USERNAME /commandhistory \
    && echo "$SNIPPET" >> "/home/$USERNAME/.bashrc"

# Install required packages
RUN apt-get update && apt-get install -y \
    bash-completion \
    build-essential \
    clang \
    clang-format \
    clang-tidy \
    doxygen \
    lld \
    llvm \
    llvm-dev \
    llvm-runtime \
    clangd \
    gcc-aarch64-linux-gnu \
    graphviz \
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
    && rm -rf /var/lib/apt/lists/* \
    && cp /usr/lib/git-core/git-absorb $(git --exec-path)/git-absorb

# Install ARM GNU Toolchain
RUN mkdir -p ${TOOLCHAIN_DIR} && \
    wget -q https://developer.arm.com/-/media/Files/downloads/gnu/${ARM_TOOLCHAIN_VERSION}/binrel/arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-aarch64-none-elf.tar.xz && \
    tar -xf arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-aarch64-none-elf.tar.xz -C ${TOOLCHAIN_DIR} --strip-components=1 && \
    rm arm-gnu-toolchain-${ARM_TOOLCHAIN_VERSION}-x86_64-aarch64-none-elf.tar.xz
ENV PATH="${TOOLCHAIN_DIR}/bin:${PATH}"

# Set up Git aliases
RUN echo '\n\
# Git aliases\n\
alias gp="git push -f"\n\
alias gir="git rebase -i --autosquash"\n\
alias girm="git rebase -i --autosquash main"\n\
alias gmr="git rebase main"\n\
alias gfix="git commit --fixup"\n\
alias gan="git commit --amend --no-edit"\n\
alias ga="git commit --amend"\n\
\n\
' >> /home/vscode/.bashrc
