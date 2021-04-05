# `modcap` - distributed capabilities for the Modbus protocol

The `modcap' library is a proof of concept for composing hardware-enforced
architectural capabilities, software-definded object capabilities, and
cryptographic network capabilities to provide a true, least-privilege access
control mechanism for an entire system-of-systems.

A *capability* demonstrates that a *subject* has *authority* to *invoke* a
*resource* [0,1] .  That is, when a subject presents a capability, the
capability proves the subject has the specified access rights for the named
resource [0].

For this PoC, all Modbus objects (coils, discrete inputs, and registers) are
converted to capabilities, and a device on the network, or a software library
on a given device, is only granted a capability with the specific permissions
necessary to execute a required task.

This PoC makes use of Capability Hardware Enhanced RISC Instruction (CHERI)
architectural capabilities [2] and Macaroon cryptographic capabilities [3].
The initial concept of composing these capabilities to provide access control
for Cyber-Physical Systems (CPS) was proposed by Dodson *et al.* [4].

## Project layout

The `modcap` library is meant to be used as a drop-in replacement for the
`libmodbus` C library [5] and requires very limited modification to existing
applications; however, it requires the hardware (and OS, where applicable) to
be CHERI-aware.

The PoC assumes a Modbus server is running on a CPS device and a Modbus client
is running either on another device or on a Linux host.  `libmodbus` is
designed to handle all Modbus protocol communication between a server and
client, including TCP interactions.  The project incorporates two shim layers
to minimise changes to existing code.  One shim layer sits between the
application and `libmodbus` to convert between low-level CHERI capabilities and
software-defined object capabilities, and the other layer sits between the
application and `libmacaroons` to convert between software-defined object
capabilities and network capabilities.

The `libmodbus` submodule is a modified version of the `libmodbus` C library
[5].  The `modcap` branch is a port to FreeRTOS [6] with added support for
sending Macaroon network tokens as part of the protocol.  This isn't strictly
necessary but simplifies the PoC by avoiding the need to incorporate a separate
channel for transmitting Macaroons.

The `libmacaroons` submodule is a modified version of the `libmacaroons` C
library [7].  The `modcap` branch is a port to FreeRTOS [6].

`libmodbus_object_caps` is a shim layer to translate Modbus object into
capabilities and minimise object permissions held by `libmodbus`.

`libmodbus_network_caps` is a shim layer to translate Modbus object into
capabilities and minimise object permissions held by `libmacaroons`.

`modbus_benchmarks` is a helper library to perform micro and macrobenchmarks.

`modbus_client` is a PoC client application meant to run on a Linux host and
communicate with a `libmodbus` server.

`benchmark_scripts` contains example shell scripts for benchmarking different
configurations of Modbus server and client in the setup described below.

## Installation

The LLVM-based CHERI toolchain, including a CHERI-aware QEMU and soft,
CHERI-aware FPGA cores, is available through the Clean Slate Trustworthy Secure
Research and Development (CTSRD - pronounced "custard") project [8].

This PoC makes use of a CHERI-aware version of FreeRTOS [9].  The Modbus server
demo is available in the `modbus_server` branch.

Building Modbus server demo on FreeRTOS is most easily accomplished using the
`modbus_server` branch of `cheribuild` [10].

[0] [Saltzer and Schroeder, ‘The Protection of Information in Computer Systems’.](https://ieeexplore.ieee.org/stamp/stamp.jsp?arnumber=1451869)

[1] [Miller, Yee, and Shapiro, ‘Capability Myths Demolished’.](https://srl.cs.jhu.edu/pubs/SRL2003-02.pdf)

[2] [Watson et al., ‘An Introduction to CHERI’.](https://www.cl.cam.ac.uk/techreports/UCAM-CL-TR-941.pdf)

[3] [Birgisson et al., ‘Macaroons’.](https://www.ndss-symposium.org/ndss2014/programme/macaroons-cookies-contextual-caveats-decentralized-authorization-cloud/)

[4] [Dodson et al., ‘CHERI Macaroons: Efficient, Host-Based Access Control for Cyber-Physical Systems’.](https://doi.org/10.17863/CAM.54214)

[5] [https://github.com/stephane/libmodbus](https://github.com/stephane/libmodbus)

[6] [https://www.freertos.org/](https://www.freertos.org/)

[7] [https://github.com/rescrv/libmacaroons](https://github.com/rescrv/libmacaroons)

[8] [https://github.com/CTSRD-CHERI/cheribuild](https://github.com/CTSRD-CHERI/cheribuild)

[9] [https://github.com/CTSRD-CHERI/FreeRTOS-Demos-CHERI-RISC-V/tree/modbus_server](https://github.com/CTSRD-CHERI/FreeRTOS-Demos-CHERI-RISC-V/tree/modbus_server)

[10] [https://github.com/CTSRD-CHERI/cheribuild/tree/modbus_server](https://github.com/CTSRD-CHERI/cheribuild/tree/modbus_server)
