set pagination off
set confirm off
set architecture i386:x86-64
file /mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_kernel_followthrough/boot_attempt8/gcc-O2-omit-info/kernel.elf
target remote | qemu-system-x86_64 -machine pc-i440fx-10.2 -accel tcg -cpu qemu64 -smp 1 -m 128M -display none -serial file:/mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_kernel_followthrough/boot_attempt8/gcc-O2-omit-info/gdb-serial.txt -monitor none -no-reboot -net none -boot d -cdrom /mnt/f/Learning/Curricula/c_mastery/reviews/v5/expansion/legacy_kernel_followthrough/boot_attempt8/gcc-O2-omit-info/boot.iso -device isa-debug-exit,iobase=0xf4,iosize=0x04 -S -gdb stdio
hbreak *_start
continue
printf "KF_GDB_START eax=%lx ebx=%lx\n", $eax, $ebx
hbreak *kf_mode_boundary
continue
printf "KF_GDB_BEFORE cs=%lx cr0=%lx cr3=%lx\n",$cs,$cr0,$cr3
stepi
printf "KF_GDB_AFTER cs=%lx rip=%lx\n",$cs,$rip
monitor info registers
kill
quit
