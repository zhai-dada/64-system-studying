CFLAGS := -mcmodel=large -fno-builtin -fno-stack-protector -m64 -fno-PIC -c 

init.o: init.c
	gcc  $(CFLAGS) init.c
lib.o: lib.c
	gcc  $(CFLAGS) lib.c
errno.o: errno.c
	gcc  $(CFLAGS) errno.c
malloc.o:malloc.c
	gcc $(CFLAGS) malloc.c 
keyboard.o:keyboard.c 
	gcc $(CFLAGS) keyboard.c 
string.o:string.c 
	gcc $(CFLAGS) string.c 
printf.o:printf.c 
	gcc $(CFLAGS) printf.c 
system_api_lib: init.o lib.o errno.o malloc.o keyboard.o string.o printf.o
	ld -b elf64-x86-64 -z muldefs -o system_api_lib init.o errno.o malloc.o lib.o keyboard.o string.o printf.o -T user.lds
all: system_api_lib
	objcopy -I elf64-x86-64 -S -R ".eh_frame" -R ".comment" -O binary system_api_lib init.bin

clean:
	rm -rf *.o *.s *.bin 