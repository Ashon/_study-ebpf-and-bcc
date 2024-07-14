FROM ubuntu:24.04 AS pyenv-base

RUN apt update \
  && apt install --yes \
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
  && apt autoremove --yes --allow-remove-essential \
  && apt clean

RUN git clone https://github.com/pyenv/pyenv.git /pyenv
ENV PYENV_ROOT=/pyenv
ENV PATH=$PYENV_ROOT/shims:$PYENV_ROOT/bin:$PATH

FROM pyenv-base AS python-311

RUN pyenv install 3.11.0
RUN pyenv global 3.11.0

FROM python-311

WORKDIR /app
RUN apt update \
  && apt install --yes \
    bpfcc-tools \
    linux-headers-generic \
  && apt autoremove --yes --allow-remove-essential \
  && apt clean

RUN pip install bcc pytest fastapi uvicorn
ENV PYTHONPATH="/usr/lib/python3/dist-packages"

COPY tcp_monitor.c build.py ./
CMD ["python", "build.py"]
