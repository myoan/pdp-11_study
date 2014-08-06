/ write(1, hello, 6);
mov $1, r0
sys write
hello
6


/ r1 = hello;
/ *(uint16_t *)r1 = 0x4548;
mov $hello, r1
mov $42510, (r1)

/ write(1, hello, 6);
mov $1, r0
sys write
hello
6


/ exit(0);
mov $0, r0
sys exit


.data
hello: <hello\n>
