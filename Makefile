.PHONY: all clean run

CC      = gcc
CFLAGS  = -ffreestanding -fno-stack-protector -fno-pic \
          -mno-red-zone -m64 -std=c11 -Wall -Ikernel/include \
          -ffunction-sections -fcf-protection=none

LD      = ld
LDFLAGS = -nostdlib -T kernel/linker.ld -z max-page-size=0x1000

SRCS    = $(wildcard kernel/src/*.c)
OBJS    = $(SRCS:.c=.o) kernel/src/isr.o

all: boot/island.img

boot/stage1.bin: boot/stage1.asm
	nasm -f bin boot/stage1.asm -o boot/stage1.bin

boot/stage2.bin: boot/stage2.asm
	nasm -f bin boot/stage2.asm -o boot/stage2.bin

kernel/src/%.o: kernel/src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel/src/isr.o: kernel/src/isr.asm
	nasm -f elf64 kernel/src/isr.asm -o kernel/src/isr.o

kernel/kernel.bin: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel/kernel.elf $^
	objcopy -O binary kernel/kernel.elf $@

boot/island.img: boot/stage1.bin boot/stage2.bin kernel/kernel.bin
	dd if=/dev/zero of=boot/island.img bs=512 count=2880
	dd if=boot/stage1.bin of=boot/island.img bs=512 seek=0 conv=notrunc
	dd if=boot/stage2.bin of=boot/island.img bs=512 seek=1 conv=notrunc
	dd if=kernel/kernel.bin of=boot/island.img bs=512 seek=3 conv=notrunc

run: boot/island.img
	qemu-system-x86_64 \
		-drive id=disk0,format=raw,file=boot/island.img,if=none \
		-device ide-hd,drive=disk0,bus=ide.0 \
		-no-reboot

clean:
	rm -f boot/stage1.bin boot/stage2.bin boot/island.img \
	      kernel/kernel.bin $(OBJS)