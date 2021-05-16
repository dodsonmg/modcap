#!/bin/bash
##################################################################
# Simple script to build a freertos target with cheribuild and waf
##################################################################

CHERIBUILD_DIR=$HOME/cheribuild
FREERTOS_TARGET=freertos-baremetal-riscv64
COMPILER_RT_TARGET=compiler-rt-builtins-baremetal-riscv64

if [[ $# -lt 2 ]]; then
    echo "usage: $0.sh <nocheri | purecap> <prog_name>"
    exit 1
fi

# select the desired target
if [[ $1 == "purecap" ]]; then
    FREERTOS_TARGET=$FREERTOS_TARGET-purecap
    COMPILER_RT_TARGET=$COMPILER_RT_TARGET-purecap
elif [[ $1 != "nocheri" ]]; then
    echo "usage: $0.sh <nocheri | purecap> <prog_name>"
    exit 1
fi

# build compiler-rt first
# this is a bit inefficient, but only takes a few seconds
$CHERIBUILD_DIR/cheribuild.py $COMPILER_RT_TARGET --clean

# build the target
$CHERIBUILD_DIR/cheribuild.py $FREERTOS_TARGET \
    --freertos/prog $2 \
    --skip-update --clean
