#/bin/bash
GIC=3
CPU=cortex-a76
RAM=512M
IMAGE=monitor.elf
SMP=2

qemu-system-aarch64 -M virt,secure=on,gic-version=$GIC \
  -cpu $CPU -smp $SMP -m $RAM -nographic -kernel $IMAGE \
  -serial mon:stdio -serial file:/dev/fd/2 \
  -drive id=drive0,file=data.bin,format=raw,if=none \
  -device virtio-blk-device,drive=drive0,bus=virtio-mmio-bus.0
