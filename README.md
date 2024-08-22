
# Development Environment

## [1/2] Install lima

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

## [2/2] Define & Start Development VM Instance

``` sh
# Define & Start VM instance from yaml definition.
$ ./launch.sh
```

# ebpf Development Environment (python, bcc)

## Connect to VM Instance

``` sh
$ limactl shell ebpf-dev
```

## Prepare development environment

``` sh
# go to project directory
$ cd /workspace/tcp_monitor

# prepare python 3.11.0 virtualenv
$ pyenv install 3.11.0
$ pyenv virtualenv 3.11.0 ebpf-dev
$ pyenv local ebpf-dev

# install bcc
$ sudo apt-get install bpfcc-tools linux-headers-$(uname -r)
$ pip install bcc
```

## build & run container

``` sh
$ cd /workspace/tcp_monitor
$ docker compose build
$ docker compose up
```

## Hacks

``` sh
# fixes. ImportError: cannot import name 'BPF' from 'bcc'
$ export PYTHONPATH=$(dirname `find /usr/lib -name bcc`):$PYTHONPATH
```

# Tracepoints

``` sh
# if kernel debug point does not exists, mount debugfs manually
sudo mount -t debugfs none /sys/kernel/debug

# Example: Available event list
/sys/kernel/debug/tracing/available_events
/sys/kernel/debug/tracing/available_filter_functions
/sys/kernel/debug/tracing/available_filter_functions_addrs
/sys/kernel/debug/tracing/available_tracers
...
```
