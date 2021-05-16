#!/bin/bash
########################################
# Simple script to run a freertos target
########################################

CHERIBUILD_DIR=$HOME/cheribuild
FREERTOS_TARGET=run-freertos-baremetal-riscv64

if [[ $# -lt 2 ]]; then
    echo "usage: $0.sh <nocheri | purecap> <prog_name>"
    exit 1
fi

CHERI=$1
PROG=$2

if [[ ${CHERI} == "purecap" ]]; then
    FREERTOS_TARGET=${FREERTOS_TARGET}-purecap
elif [[ ${CHERI} != "nocheri" ]]; then
    echo "usage: $0.sh <nocheri | purecap> <prog_name>"
    exit 1
fi

$CHERIBUILD_DIR/cheribuild.py ${FREERTOS_TARGET} --run-freertos/prog ${PROG}
