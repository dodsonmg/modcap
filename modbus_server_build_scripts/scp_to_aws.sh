#!/bin/bash

###########################################################################
# Script to scp all FreeRTOS Modbus FETT demos to AWS F1
###########################################################################

set -e
set -u

BIN_DIR=./bin
USERNAME=ubuntu
ELF_SUFFIX=.elf

if [[ $# != 2 ]]; then
    echo "usage: $0.sh <prog> <server ip>"
    exit 1
fi

PROG=$1
SERVER_IP=$2

scp ${BIN_DIR}/${PROG}*${ELF_SUFFIX} ${USERNAME}@${SERVER_IP}:~
