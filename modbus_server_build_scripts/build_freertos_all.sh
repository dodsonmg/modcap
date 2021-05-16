#!/bin/bash

###########################################################################
# Script to build FreeRTOS Modbus FETT demo targets with cheribuild and waf
#
# Starts by building the appropriate compiler-rt
#
# Builds several FreeRTOS Modbus demos and renames them accordingly:
# - main_modbus (basic)
# - main_modbus_microbenchmark
# - main_modbus_macrobenchmark
# - main_modbus_object_caps (purecap only)
# - main_modbus_object_caps_microbenchmark (purecap only)
# - main_modbus_object_caps_macrobenchmark (purecap only)
# - main_modbus_network_caps
# - main_modbus_network_caps_microbenchmark
# - main_modbus_network_caps_macrobenchmark
# - main_modbus_object_network_caps (purecap only)
# - main_modbus_object_network_caps_microbenchmark (purecap only)
# - main_modbus_object_network_caps_macrobenchmark (purecap only)
###########################################################################
set -e
set -u

CHERIBUILD_DIR=$HOME/cheribuild
CHERI_OUTPUT_DIR=$HOME/cheri/output
FREERTOS_TARGET=freertos-baremetal-riscv64
COMPILER_RT_TARGET=compiler-rt-builtins-baremetal-riscv64
CP_BIN_DIR=./bin
EXEC_PERIOD=(100)
NETWORK_DELAY=(10)
# EXEC_PERIOD=(20 100)
# NETWORK_DELAY=(0 10)

if [[ $# != 3 ]]; then
    echo "usage: $0.sh <nocheri | purecap> <qemu_virt | fett> <prog>"
    exit 1
fi

CHERI=$1
PLATFORM=$2
PROG=$3

# Identify the directory where cheribuild installs binaries
if [[ ${CHERI} == "nocheri" ]]; then
    BIN_DIR=${CHERI_OUTPUT_DIR}/sdk/baremetal/baremetal-riscv64/riscv64-unknown-elf/FreeRTOS/Demo/bin
elif [[ ${CHERI} == "purecap" ]]; then
    BIN_DIR=${CHERI_OUTPUT_DIR}/sdk/baremetal/baremetal-riscv64-purecap/riscv64-unknown-elf/FreeRTOS/Demo/bin
    COMPILER_RT_TARGET=${COMPILER_RT_TARGET}-purecap
    FREERTOS_TARGET=${FREERTOS_TARGET}-purecap
else
    echo "usage: $0.sh <nocheri | purecap> <qemu_virt | fett> <prog>"
    exit 1
fi

# build compiler_rt
${CHERIBUILD_DIR}/cheribuild.py ${COMPILER_RT_TARGET} --clean

# build the freertos modbus demo applications

# basic demo
${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
    --freertos/prog ${PROG} \
    --freertos/platform ${PLATFORM} \
    --skip-update --clean
# rename
cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}.elf

# basic network capability demo
${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
    --freertos/prog ${PROG} \
    --freertos/platform ${PLATFORM} \
    --freertos/modbus_network_caps \
    --skip-update --clean
# rename
cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_network_caps.elf

# create microbenchmarks with different execution periods
for exec_period in "${EXEC_PERIOD[@]}"; do
    # basic demo with microbenchmarking
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_microbenchmark \
        --freertos/modbus_exec_period ${exec_period} \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_microbenchmark_${exec_period}.elf

    # network capability demo with microbenchmarking
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_microbenchmark \
        --freertos/modbus_exec_period ${exec_period} \
        --freertos/modbus_network_caps \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_network_caps_microbenchmark_${exec_period}.elf
done

# create macrobenchmark with different simulated network delays
for network_delay in "${NETWORK_DELAY[@]}"; do
    # basic demo with macrobenchmarking
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_macrobenchmark \
        --freertos/modbus_network_delay ${network_delay} \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_macrobenchmark_${network_delay}.elf

    # network capability demo with macrobenchmarking
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_macrobenchmark \
        --freertos/modbus_network_delay ${network_delay} \
        --freertos/modbus_network_caps \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_network_caps_macrobenchmark_${network_delay}.elf
done

if [[ ${CHERI} == "purecap" ]]; then
    # object capability demo
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_object_caps \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_caps.elf

    # object and network capability demo
    ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
        --freertos/prog ${PROG} \
        --freertos/platform ${PLATFORM} \
        --freertos/modbus_object_caps \
        --freertos/modbus_network_caps \
        --skip-update --clean
    # rename
    cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_network_caps.elf

    # create microbenchmarks with different execution periods
    for exec_period in "${EXEC_PERIOD[@]}"; do
        # object capability demo with microbenchmarking
        ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
            --freertos/prog ${PROG} \
            --freertos/platform ${PLATFORM} \
            --freertos/modbus_microbenchmark \
            --freertos/modbus_exec_period ${exec_period} \
            --freertos/modbus_object_caps \
            --skip-update --clean
        # rename
        cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_caps_microbenchmark_${exec_period}.elf

        # object and network capability demo with microbenchmarking
        ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
            --freertos/prog ${PROG} \
            --freertos/platform ${PLATFORM} \
            --freertos/modbus_microbenchmark \
            --freertos/modbus_exec_period ${exec_period} \
            --freertos/modbus_object_caps \
            --freertos/modbus_network_caps \
            --skip-update --clean
        # rename
        cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_network_caps_microbenchmark_${exec_period}.elf
    done

    # create macrobenchmark with different simulated network delays
    for network_delay in "${NETWORK_DELAY[@]}"; do
        # object capability demo with macrobenchmarking
        ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
            --freertos/prog ${PROG} \
            --freertos/platform ${PLATFORM} \
            --freertos/modbus_macrobenchmark \
            --freertos/modbus_network_delay ${network_delay} \
            --freertos/modbus_object_caps \
            --skip-update --clean
        # rename
        cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_caps_macrobenchmark_${network_delay}.elf

        # object and network capability demo with macrobenchmarking
        ${CHERIBUILD_DIR}/cheribuild.py ${FREERTOS_TARGET} \
            --freertos/prog ${PROG} \
            --freertos/platform ${PLATFORM} \
            --freertos/modbus_macrobenchmark \
            --freertos/modbus_network_delay ${network_delay} \
            --freertos/modbus_object_caps \
            --freertos/modbus_network_caps \
            --skip-update --clean
        # rename
        cp ${BIN_DIR}/RISC-V-Generic_${PROG}.elf ${CP_BIN_DIR}/${PROG}_${CHERI}_object_network_caps_macrobenchmark_${network_delay}.elf
    done
fi
