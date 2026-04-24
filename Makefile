CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -nostdlib -mcmodel=kernel -mno-red-zone -Wall -Wextra -I kernel/include -Wl,-z,max-page-size=0x1000
ASM = nasm
ASMFLAGS = -f elf64


KERNEL_SRC = kernel/src/kernel.c
TERMINAL_SRC = kernel/src/terminal.c
PMM_SRC = kernel/src/pmm.c
VMM_SRC = kernel/src/vmm.c
GDT_SRC = kernel/src/gdt.c
IDT_SRC = kernel/src/idt.c
ISR_SRC = kernel/src/isr.asm


FONT8_SRC = kernel/src/font8.psf
FONT16_SRC = kernel/src/font16.psf
FONT32_SRC = kernel/src/font32.psf


FONT8_OBJ = font8.o
FONT16_OBJ = font16.o
FONT32_OBJ = font32.o


KERNEL_OBJ = kernel.o
TERMINAL_OBJ = terminal.o
PMM_OBJ = pmm.o
VMM_OBJ = vmm.o
GDT_OBJ = gdt.o
IDT_OBJ = idt.o
ISR_OBJ = isr.o


KERNEL_ELF = kernel.elf
ISO = pragma.iso
LIMINE_DIR = limine
ISO_ROOT = iso_root
.PHONY: all clean run
all: $(ISO)

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(TERMINAL_OBJ): $(TERMINAL_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(PMM_OBJ): $(PMM_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(VMM_OBJ): $(VMM_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(GDT_OBJ): $(GDT_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(IDT_OBJ): $(IDT_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<
$(ISR_OBJ): $(ISR_SRC)
	$(ASM) $(ASMFLAGS) -o $@ $<


$(FONT8_OBJ): $(FONT8_SRC)
	objcopy -O elf64-x86-64 -I binary $< $@
$(FONT16_OBJ): $(FONT16_SRC)
	objcopy -O elf64-x86-64 -I binary $< $@
$(FONT32_OBJ): $(FONT32_SRC)
	objcopy -O elf64-x86-64 -I binary $< $@

$(KERNEL_ELF): $(KERNEL_OBJ) $(TERMINAL_OBJ) $(PMM_OBJ) $(VMM_OBJ) $(GDT_OBJ) $(IDT_OBJ) $(ISR_OBJ) $(FONT8_OBJ) $(FONT16_OBJ) $(FONT32_OBJ)
	$(CC) $(CFLAGS) -T linker.ld -o $@ $^
$(ISO): $(KERNEL_ELF)
	rm -rf $(ISO_ROOT)
	mkdir -p $(ISO_ROOT)/boot/limine
	mkdir -p $(ISO_ROOT)/EFI/BOOT
	cp $(KERNEL_ELF) $(ISO_ROOT)/boot/
	cp limine.conf $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-bios.sys $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_ROOT)/boot/limine/
	cp $(LIMINE_DIR)/BOOTX64.EFI $(ISO_ROOT)/EFI/BOOT/
	cp $(LIMINE_DIR)/BOOTIA32.EFI $(ISO_ROOT)/EFI/BOOT/
	xorriso -as mkisofs -R -r -J \
	    -b boot/limine/limine-bios-cd.bin \
	    -no-emul-boot -boot-load-size 4 -boot-info-table -hfsplus \
	    -apm-block-size 2048 \
	    --efi-boot boot/limine/limine-uefi-cd.bin \
	    -efi-boot-part --efi-boot-image --protective-msdos-label \
	    $(ISO_ROOT) -o $(ISO)
	$(LIMINE_DIR)/limine bios-install $(ISO)
run: $(ISO)
	qemu-system-x86_64 \
	    -bios /usr/share/ovmf/OVMF.fd \
	    -cdrom $(ISO) \
	    -m 256M
clean:
	rm -rf $(KERNEL_ELF) $(ISO) $(ISO_ROOT) *.o