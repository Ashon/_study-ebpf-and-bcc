FROM ubuntu:24.04

WORKDIR /app

RUN apt update \
  && apt install -qy \
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
    bpfcc-tools \
    linux-headers-$(uname -r) \
  && apt clean

RUN git clone https://github.com/pyenv/pyenv.git .pyenv

ENV PYENV_ROOT=/app/.pyenv
ENV PATH=$PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH

RUN pyenv install 3.11.0
RUN pyenv global 3.11.0

RUN pip install bcc pytest fastapi uvicorn
ENV PYTHONPATH="/usr/lib/python3/dist-packages"

COPY tcp_monitor.c tcp_monitor.py ./