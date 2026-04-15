CC = x86_64-elf-gcc
CFLAGS = -ffreestanding -nostdlib -mno-red-zone -Wall -Wextra -I kernel/include -Wl,-z,max-page-size=0x1000

KERNEL_SRC = kernel/src/kernel.c
FONT_SRC = kernel/src/font.psf

FONT_OBJ = font.o
KERNEL_OBJ = kernel.o

KERNEL_ELF = kernel.elf
ISO = pragma.iso

LIMINE_DIR = limine
ISO_ROOT = iso_root

.PHONY: all clean run

all: $(ISO)

$(KERNEL_OBJ): $(KERNEL_SRC)
	$(CC) $(CFLAGS) -c -o $@ $<

$(FONT_OBJ): $(FONT_SRC)
	objcopy -O elf64-x86-64 -B i386 -I binary $< $@

$(KERNEL_ELF): $(KERNEL_OBJ) $(FONT_OBJ)
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
	rm -rf $(KERNEL_ELF) $(ISO) $(ISO_ROOT)