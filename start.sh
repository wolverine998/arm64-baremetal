#/bin/bash
GIC=3
CPU=cortex-a55
RAM=512M
IMAGE=monitor.elf
SMP=4

qemu-system-aarch64 -M virt,secure=on,gic-version=$GIC \
  -cpu $CPU -smp $SMP -m $RAM -nographic -kernel $IMAGE \
  -serial mon:stdio -serial file:/dev/fd/2
