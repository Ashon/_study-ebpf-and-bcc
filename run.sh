#!/bin/bash

docker run -it --rm \
  --privileged -it \
  -p 8000:8000 \
  -v /sys/kernel/debug:/sys/kernel/debug:rw \
  -v /lib/modules:/lib/modules:ro \
  -v /usr/src:/usr/src:ro \
  -v /etc/localtime:/etc/localtime:ro \
  --pid=host \
  tcp-monitor python tcp_monitor.py
