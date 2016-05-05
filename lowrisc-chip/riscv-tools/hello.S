	.file	"hello.c"
	.text
	.globl	main
	.align	2
	.type	main,@function
main:
	addi	x2, x2, -32
	sd	x1, 24(x2)
	sd	x3, 16(x2)
	sd	x8, 8(x2)
	add	x8, x0, x2
	lui	x5, %hi(.L.str)
	addi	x10, x5, %lo(.L.str)
	jal	printf
	sw	x10, 4(x8)
	addiw	x10, x0, 0
	add	x2, x0, x8
	ld	x1, 24(x2)
	ld	x3, 16(x2)
	ld	x8, 8(x2)
	addi	x2, x2, 32
	ret
.Ltmp3:
	.size	main, .Ltmp3-main

	.type	.L.str,@object
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "Hello World!\n"
	.size	.L.str, 14


	.section	".note.GNU-stack","",@progbits
