#!/bin/bash

set -euo pipefail

mkdir -p ./tmp

cat ebpf-dev.tpl.yaml \
| sed -e "s|<!--CHANGE_THIS_TO_ABSOLUTE_PWD-->|$(pwd)|g" \
  > ./tmp/ebpf-dev.yaml

echo '\n' \
| limactl start ./tmp/ebpf-dev.yaml
