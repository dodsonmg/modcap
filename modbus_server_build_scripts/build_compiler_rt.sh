#!/bin/bash

##################################################################
# Simple script to build compiler-rt for either nocheri or purecap
##################################################################

CHERIBUILD_DIR=$HOME/cheribuild
TARGET=compiler-rt-builtins-baremetal-riscv64

if [[ $# -lt 1 ]]; then
    echo "usage: build_compiler_rt.sh <nocheri | purecap>"
    exit 1
fi

# build the desired target
if [[ $1 == "purecap" ]]; then
    TARGET=$TARGET-purecap
elif [[ $1 != "nocheri" ]]; then
    echo "usage: build_compiler_rt.sh <nocheri | purecap>"
    exit 1
fi

$CHERIBUILD_DIR/cheribuild.py $TARGET --clean
