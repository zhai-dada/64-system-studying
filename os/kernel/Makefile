CFLAGS := -mcmodel=large -fno-builtin -fno-stack-protector -m64 -fno-PIC -c 
OBJFLAGS := -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary
LDFLAGS := -b elf64-x86-64 -z muldefs
PIC := APIC
MACHINE := BOCH
head.o:head.S
	gcc -E head.S -o head.s -D$(MACHINE)
	as --64 head.s -o head.o
apu_boot.o:apu_boot.S
	gcc -E apu_boot.S -o apu_boot.s
	as --64 apu_boot.s -o apu_boot.o
entry.o:entry.S
	gcc -E entry.S -o entry.s
	as --64 entry.s -o entry.o
printk.o:printk.c
	gcc $(CFLAGS) printk.c -D$(MACHINE)
cpu.o:cpu.c
	gcc $(CFLAGS) cpu.c
trap.o:trap.c
	gcc $(CFLAGS) trap.c
memory.o:memory.c
	gcc $(CFLAGS) memory.c
interrupt.o:interrupt.c
	gcc $(CFLAGS) interrupt.c
mouse.o:mouse.c
	gcc $(CFLAGS) mouse.c
task.o:task.c
	gcc $(CFLAGS) task.c
keyboard.o:keyboard.c
	gcc $(CFLAGS) keyboard.c
smp.o:smp.c
	gcc $(CFLAGS) smp.c -D$(MACHINE)
time.o:time.c
	gcc $(CFLAGS) time.c
HPET.o:HPET.c
	gcc $(CFLAGS) HPET.c
softirq.o:softirq.c
	gcc $(CFLAGS) softirq.c
schedule.o:schedule.c
	gcc $(CFLAGS) schedule.c
spinlock.o:spinlock.c
	gcc $(CFLAGS) spinlock.c
semaphore.o:semaphore.c
	gcc $(CFLAGS) semaphore.c
atomic.o:atomic.c
	gcc $(CFLAGS) atomic.c
disk.o:disk.c 
	gcc $(CFLAGS) disk.c 
fat32.o:fat32.c
	gcc $(CFLAGS) fat32.c 
syscall.o:syscall.c 
	gcc $(CFLAGS) syscall.c 
sys.o:sys.c 
	gcc $(CFLAGS) sys.c 
vfs.o:vfs.c 
	gcc $(CFLAGS) vfs.c
waitqueue.o:waitqueue.c 
	gcc $(CFLAGS) waitqueue.c 
ifeq ($(PIC), APIC)
PIC.o:APIC.c
	gcc $(CFLAGS) APIC.c -o PIC.o -D$(MACHINE)
else
PIC.o:8259A.c
	gcc $(CFLAGS) 8259A.c -o PIC.o
endif

main.o:main.c
	gcc $(CFLAGS) main.c -D$(PIC) -D$(MACHINE)
test.o:test.c
	gcc $(CFLAGS) test.c -D$(PIC)

system:head.o main.o printk.o trap.o cpu.o entry.o memory.o interrupt.o PIC.o keyboard.o smp.o apu_boot.o task.o time.o HPET.o fat32.o vfs.o syscall.o sys.o spinlock.o softirq.o schedule.o semaphore.o atomic.o disk.o waitqueue.o 
	ld $(LDFLAGS) -o system head.o entry.o main.o printk.o cpu.o trap.o sys.o syscall.o memory.o PIC.o interrupt.o task.o keyboard.o disk.o fat32.o vfs.o spinlock.o apu_boot.o smp.o softirq.o HPET.o time.o schedule.o semaphore.o atomic.o waitqueue.o -T Kernel.lds
	
all:system
	objcopy $(OBJFLAGS) system kernel.bin
	objdump -D system > test
.PHONY:
clean:
	rm -rf *.s *.o *.bin system kernel.bin

