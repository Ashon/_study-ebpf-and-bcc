#!/bin/bash

set -euo pipefail

limactl stop -f ebpf-dev
limactl delete ebpf-dev
