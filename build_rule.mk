include common.mk

OBJS = boot.o
DEPS = $(filter %.d, $(subst .o,.d, $(OBJS)))
KERNEL = kernel.elf
IMAGEFILE = disk.img
IMAGE = /tmp/$(IMAGEFILE)

MAKE := $(MAKE) -f $(RULE_FILE)

MOUNT_DIR=/mnt/baremetal
MOUNT_DIR_EFI=/mnt/efi
OVMF_DIR = /home/vagrant/edk2-UDK2017/

TELNET_ADDR = 127.0.0.1
TELNET_PORT = 1235

FLAGS = -ggdb3 -O3 -Wall -Werror=return-type -Werror=unused-result -nostdinc -nostdlib -fno-builtin -mcmodel=large -MMD -MP
ASFLAGS += $(FLAGS)
CFLAGS += $(FLAGS) -std=c99

# QEMUでUEFI環境をエミュレートするための設定
# この行をコメントアウトするとBIOS環境をエミュレートする
QEMU_UEFI_OPTION = -drive if=pflash,readonly,file=$(OVMF_DIR)OVMF_CODE.fd,format=raw -drive if=pflash,file=$(OVMF_DIR)OVMF_VARS.fd,format=raw

# ヘッダファイルの依存関係解決用
-include $(DEPS)

default: KERNEL

$(KERNEL): $(OBJS) kernel.ld
	$(LD) -nostdinc -nostdlib -Tkernel.ld $(OBJS) -o $@

# ディスクイメージの新規作成
$(IMAGE):
	$(MAKE) umount
# 空のディスクイメージの作成
	dd if=/dev/zero of=$(IMAGE) bs=1M count=200
# パーティションの作成
	sgdisk -a 1 -n 1::2047 -t 1:EF02 -a 2048 -n 2::100M -t 2:EF00 -n 3:: -t 3:8300 $(IMAGE)
# マウントとgrubのインストール
	set -- $$(sudo kpartx -av $(IMAGE)); \
	sudo mkfs.vfat -F32 /dev/mapper/$${12}; \
	sudo mount -t vfat /dev/mapper/$${12} $(MOUNT_DIR_EFI); \
	sudo mkfs.ext2 /dev/mapper/$${21}; \
	sudo mount -t ext2 /dev/mapper/$${21} $(MOUNT_DIR); \
	sudo grub-install --target=i386-pc --no-floppy $${8} --root-directory $(MOUNT_DIR); \
	sudo grub-install --target=x86_64-efi --no-nvram --efi-directory=$(MOUNT_DIR_EFI) --boot-directory=$(MOUNT_DIR)/boot
	sudo mv $(MOUNT_DIR_EFI)/EFI/ubuntu $(MOUNT_DIR_EFI)/EFI/boot
	sudo mv $(MOUNT_DIR_EFI)/EFI/boot/grubx64.efi $(MOUNT_DIR_EFI)/EFI/boot/bootx64.efi
	$(MAKE) umount

$(MOUNT_DIR)/boot/grub/grub.cfg: grub.cfg
	sudo cp grub.cfg $(MOUNT_DIR)/boot/grub/grub.cfg

$(MOUNT_DIR)/core/:
	sudo mkdir -p $(MOUNT_DIR)/core/

$(MOUNT_DIR)/core/$(KERNEL): $(MOUNT_DIR)/core/ $(KERNEL)
	sudo cp $(KERNEL) $(MOUNT_DIR)/core/$(KERNEL)

.PHONY: runqemu killqemu

runqemu: image
	sudo qemu-system-x86_64 $(QEMU_UEFI_OPTION) -cpu qemu64 -smp 8 -machine q35 -monitor telnet:$(TELNET_ADDR):$(TELNET_PORT),server,nowait -vnc 0.0.0.0:0,password -drive format=raw,file=$(IMAGE) &
	sleep 0.2s
	echo "set_password vnc a" | netcat $(TELNET_ADDR) $(TELNET_PORT)

killqemu:
	-sudo pkill -KILL qemu

.PHONY: run hd image mount umount clean

run:
	$(MAKE) killqemu
	$(MAKE) runqemu
	-telnet $(TELNET_ADDR) $(TELNET_PORT)
	$(MAKE) killqemu

hd: image
	$(if $(shell if [ ! -e /dev/sdb ]; then echo "no usb"; fi), \
	  @echo "error: insert usb memory!"; exit 1, \
	)
	sudo dd if=$(IMAGE) of=/dev/sdb

image: $(IMAGE) mount $(MOUNT_DIR)/boot/grub/grub.cfg $(MOUNT_DIR)/core/$(KERNEL) umount

mount: $(IMAGE)
	$(MAKE) umount
	set -- $$(sudo kpartx -av $(IMAGE)); \
	sudo mount -t vfat /dev/mapper/$${12} $(MOUNT_DIR_EFI); \
	sudo mount -t ext2 /dev/mapper/$${21} $(MOUNT_DIR);

umount:
	$(if $(shell mountpoint -q $(MOUNT_DIR); if [ $$? = 0 ]; then echo "mounted"; fi), sudo umount $(MOUNT_DIR))
	$(if $(shell mountpoint -q $(MOUNT_DIR_EFI); if [ $$? = 0 ]; then echo "mounted"; fi), sudo umount $(MOUNT_DIR_EFI))
	sudo kpartx -d $(IMAGE) || return 0
	sudo losetup -d /dev/loop[0-9] > /dev/null 2>&1 || return 0

clean:
	$(MAKE) umount
	-rm -rf $(IMAGE) $(KERNEL) $(DEPS) $(OBJS)
