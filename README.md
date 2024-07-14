
# Development Environment

## [1/3] Prepare Lima Virtual machine environment

``` sh
# lima: linux virtual machine API
# socket_vmnet: lima shared network driver
$ brew install lima socket_vmnet
```

fix default path of lima network config

``` sh
$ vi ~/.lima/_config/networks.yaml
```

fix `socketVMNet` path correctly

``` sh
$ readlink -f $(which socket_vmnet)
"{{ this is socket_vmnet path }}"
```

``` yaml
# file: ~/.lima/_config/networks.yaml
paths:
  socketVMNet: "{{ socket_vmnet path }}"
  ...
```

## [2/3] Define & Start Development VM Instance

Define Machine

``` sh
$ limactl create \
  --name=ebpf-dev \
  --cpus=2 \
  --memory=2 \
  --network lima:shared \
  --mount $(pwd):/works:w
```

Start Machine

``` sh
$ limactl start ebpf-dev
```

## [3/3] Bootstrapping VM Instance for ebpf dev

``` sh
$ limactl shell ebpf-dev
```

# ebpf Development Environment (python, bcc)

## Pyenv

``` sh
$ sudo apt install -qy \
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
  tk-dev
```

## Python 3.11

``` sh
$ pyenv install 3.11.0
$ pyenv virtualenv 3.11.0 ebpf-dev
$ pyenv local ebpf-dev
```

## bcc

``` sh
$ sudo apt-get install bpfcc-tools linux-headers-$(uname -r)
$ pip install bcc
```

## Hacks

``` sh
# fixes. ImportError: cannot import name 'BPF' from 'bcc'
$ export PYTHONPATH=$(dirname `find /usr/lib -name bcc`):$PYTHONPATH
```

# Tracepoints

``` sh
/sys/kernel/debug/tracing/events
```
