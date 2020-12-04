#-
# SPDX-License-Identifier: BSD-2-Clause
#
# Copyright (c) 2020 Michael Dodson
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.
#

def configure(ctx):
    print("Configuring modcap @", ctx.path.abspath())

    ctx.env.append_value('INCLUDES', [
        ctx.path.abspath(),
        ctx.path.abspath() + '/include',
        ctx.path.abspath() + '/libmodbus/src/',
        ctx.path.abspath() + '/libmodbus/include/',
        ctx.path.abspath() + '/libmacaroons/include/',
        ctx.path.abspath() + '/libmodbus_object_caps/include/',
        ctx.path.abspath() + '/libmodbus_network_caps/include/'
    ])

    ctx.env.append_value('LIB_DEPS', ['freertos_tcpip', 'virtio'])


def build(bld):
    print("Building modcap")

    LIBMACAROONS_DIR = 'libmacaroons/'
    LIBMODBUS_DIR = 'libmodbus/'
    LIBMODBUS_OBJECT_CAPS_DIR = 'libmodbus_object_caps/'
    LIBMODBUS_NETWORK_CAPS_DIR = 'libmodbus_network_caps/'

    bld.stlib(features=['c'],
              source=[
                  LIBMODBUS_DIR + 'src/modbus.c',
                  LIBMODBUS_DIR + 'src/modbus-data.c',
                  LIBMODBUS_DIR + 'src/modbus-tcp.c',
                  LIBMODBUS_DIR + 'src/modbus-helpers.c'
              ],
              use=[
                  "freertos_core",
                  "freertos_bsp",
                  "freertos_tcpip"
              ],
              target="modbus")

    bld.stlib(
        features=['c'],
        source=[
            LIBMACAROONS_DIR + 'src/base64.c',
            LIBMACAROONS_DIR + 'src/explicit_bzero.c',
            LIBMACAROONS_DIR + 'src/macaroons.c',
            LIBMACAROONS_DIR + 'src/packet.c', LIBMACAROONS_DIR + 'src/port.c',
            LIBMACAROONS_DIR + 'src/sha256.c', LIBMACAROONS_DIR + 'src/shim.c',
            LIBMACAROONS_DIR + 'src/slice.c',
            LIBMACAROONS_DIR + 'src/timingsafe_bcmp.c',
            LIBMACAROONS_DIR + 'src/tweetnacl.c',
            LIBMACAROONS_DIR + 'src/v1.c', LIBMACAROONS_DIR + 'src/v2.c',
            LIBMACAROONS_DIR + 'src/varint.c'
        ],
        use=[
            "freertos_core",
            "freertos_bsp"
        ],
        target="macaroons")

    if bld.env.PURECAP:
        bld.stlib(features=['c'],
                  source=[LIBMODBUS_OBJECT_CAPS_DIR + 'src/modbus_object_caps.c'],
                  use=[
                      "freertos_core",
                      "freertos_bsp",
                      "freertos_tcpip"
                  ],
                  defines=bld.env.DEFINES + ['MODBUS_OBJECT_CAPS=1'],
                  target="modbus_object_caps")

    bld.stlib(features=['c'],
              source=[LIBMODBUS_NETWORK_CAPS_DIR + 'src/modbus_network_caps.c'],
              use=[
                  "freertos_core",
                  "freertos_bsp",
                  "freertos_tcpip",
                  "macaroons"
              ],
              target="modbus_network_caps")
