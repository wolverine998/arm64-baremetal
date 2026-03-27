# ARM64 Baremetal monolithic binary

Implementation of ARM64 Secure Monitor(**EL3**), \
Secure Execution Environment(**S-EL1**) and \
Application OS(**EL1**).

## Current progress

### Secure Monitor
- [x] Context switch
- [x] Generic Interrupt Controler v3
- [x] SGI, SPI, PPI, LPI fully supported
- [x] PSCI version **1.0**

### Secure Execution Environment
- [x] Secure service handler
- [x] Buffer based command protocol
- [x] Crpyto Engine version **1.0**

### Application OS
- [x] 32-bit Virtual Memory addressing
- [x] Page based allocator
- [x] Simple heap allocator
- [x] Queue scheduler
- [x] GIC Interrupt Translation Service
- [x] Minimal standard library
- [x] Virtio-blk-device driver
- [x] Device driver subsystem



***More features comming soon!*** 
