wsl make
qemu-system-x86_64 -cdrom os.iso -hda fat.img -boot d -no-reboot -s -serial stdio -S