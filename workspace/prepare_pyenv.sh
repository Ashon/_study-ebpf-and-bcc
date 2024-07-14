#!/bin/bash

set -euo pipefail

sudo apt update \
  && sudo apt install --yes \
    git \
    build-essential \
    libssl-dev \
    zlib1g-dev \
    libbz2-dev \
    liblzma-dev \
    libreadline-dev \
    libsqlite3-dev \
    wget \
    curl \
    llvm \
    libncurses5-dev \
    libncursesw5-dev \
    xz-utils \
    tk-dev \
  && sudo apt autoremove --yes --allow-remove-essential \
  && sudo apt clean

PYENV_ROOT=/opt/pyenv

sudo git clone https://github.com/pyenv/pyenv.git $PYENV_ROOT
sudo chown -R $USER:$USER $PYENV_ROOT

echo "export PYENV_ROOT=$PYENV_ROOT"
echo "export PATH=\$PYENV_ROOT/shims:\$PYENV_ROOT/bin:\$PATH"
