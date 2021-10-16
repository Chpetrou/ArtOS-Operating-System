# Automatically generate lists of sources using wildcards. 
C_SOURCES = $(wildcard *.c cpu/*.c devices/*.c fs/*.c libkern/*.c mm/*.c sys/*.c video/*.c libc/stdlib/*.c libc/string/*.c libc/stdio/*.c)
HEADERS = $(wildcard asm/*.h include/*.h libc/include/arch/x86/*.h)

# Convert the *.c filenames to *.o to give a list of object files to build
OBJ = ${C_SOURCES:.c=.o}

# Defaul build target
all: kernel.bin

# Run bochs to simulate booting of our code.
run: myos.iso 
	qemu-system-i386 -smp 2 -cdrom $<

# This builds the binary of our kernel from two object files:
# - the kernel_entry, which jumps to main() in our kernel
# - the compiled C kernel
kernel.bin: boot.o ${OBJ}
	i686-elf-gcc -m32 -T link32.ld -z max-page-size=4096 -o $@ -ffreestanding -O2 -nostdlib $^ -lgcc

# Generic rule for compiling C code to an object file
# For simplicity, we C files depend on all header files.
%.o : %.c ${HEADERS}
	i686-elf-gcc -m32 -c $< -o $@ -I include/ -ffreestanding -O2 -Wall -Wextra

# Assemble the kernel_entry.
boot.o:
	nasm -f elf -o boot.o boot.s
    
iso : kernel.bin
	mkdir -p ../isodir/boot/grub
	cp kernel.bin ../isodir/boot/kernel.bin
	cd tools && $(MAKE)
	cp tools/initrd.img ../isodir/boot/initrd.img
	cp ../grub.cfg ../isodir/boot/grub/grub.cfg
	grub-mkrescue -o myos.iso ../isodir
    
clean:
	rm -fr *.bin *.dis *.o os-image
	rm -fr cpu/*.o devices/*.o fs/*.o libkern/*.o mm/*.o sys/*.o tools/*.o video/*.o libc/stdlib/*.o libc/string/*.o libc/stdio/*.o kernel/*.o kernel/arch/x86/*.o boot/*.bin libc/*.o

good:
	make clean
	clear
	clear
	make all
