	.file	"stack_protection.ll"
	.text
	.globl	main
	.align	2
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	addi	x2, x2, -32
	addiw	x5, x0, 1
	wrt	x1, x1, x5
	sdct	x1, 24(x2)              # 8-byte Folded Spill
	wrt	x3, x3, x5
	sdct	x3, 16(x2)              # 8-byte Folded Spill
	wrt	x8, x8, x5
	sdct	x8, 8(x2)               # 8-byte Folded Spill
	add	x8, x0, x2
	sw	x0, 4(x8)
	jal	test
	lui	x5, %hi(.L.str)
	addi	x10, x5, %lo(.L.str)
	jal	printf
	li	x10, 1
	add	x2, x0, x8
	ld	x1, 24(x2)              # 8-byte Folded Reload
	stag	x0, 24(x2)              # 8-byte Folded Reload
	ld	x3, 16(x2)              # 8-byte Folded Reload
	stag	x0, 16(x2)              # 8-byte Folded Reload
	ld	x8, 8(x2)               # 8-byte Folded Reload
	stag	x0, 8(x2)               # 8-byte Folded Reload
	addi	x2, x2, 32
	ret
.Ltmp3:
	.size	main, .Ltmp3-main

	.globl	test
	.align	2
	.type	test,@function
test:                                   # @test
# BB#0:                                 # %entry
	addi	x2, x2, -96
	addiw	x5, x0, 1
	wrt	x1, x1, x5
	sdct	x1, 88(x2)              # 8-byte Folded Spill
	wrt	x3, x3, x5
	sdct	x3, 80(x2)              # 8-byte Folded Spill
	wrt	x8, x8, x5
	sdct	x8, 72(x2)              # 8-byte Folded Spill
	add	x8, x0, x2
	lui	x5, %hi(.L.str1)
	addi	x10, x5, %lo(.L.str1)
	jal	printf
	sw	x0, 4(x8)
	sw	x0, 4(x8)
	j	.LBB1_2
.LBB1_1:                                # %for.body
                                        #   in Loop: Header=BB1_2 Depth=1
	lw	x11, 4(x8)
	lui	x5, %hi(.L.str2)
	addi	x10, x5, %lo(.L.str2)
	li	x12, 16
	jal	printf
	lw	x5, 4(x8)
	slli	x6, x5, 2
	addi	x7, x8, 8
	add	x6, x7, x6
	sw	x5, 0(x6)
	lw	x5, 4(x8)
	addiw	x5, x5, 1
	sw	x5, 4(x8)
.LBB1_2:                                # %for.cond
                                        # =>This Inner Loop Header: Depth=1
	li	x5, 63
	lw	x6, 4(x8)
	bge	x5, x6, .LBB1_1
# BB#3:                                 # %for.end
	lui	x5, %hi(.L.str3)
	addi	x10, x5, %lo(.L.str3)
	jal	printf
	add	x2, x0, x8
	ld	x1, 88(x2)              # 8-byte Folded Reload
	stag	x0, 88(x2)              # 8-byte Folded Reload
	ld	x3, 80(x2)              # 8-byte Folded Reload
	stag	x0, 80(x2)              # 8-byte Folded Reload
	ld	x8, 72(x2)              # 8-byte Folded Reload
	stag	x0, 72(x2)              # 8-byte Folded Reload
	addi	x2, x2, 96
	ret
.Ltmp7:
	.size	test, .Ltmp7-test

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "Successfully terminated\n"
	.size	.L.str, 25

	.type	.L.str1,@object         # @.str1
.L.str1:
	.asciz	 "Should fail to write on writing element 16...\n"
	.size	.L.str1, 47

	.type	.L.str2,@object         # @.str2
.L.str2:
	.asciz	 "Filling array element %d of %d\n"
	.size	.L.str2, 32

	.type	.L.str3,@object         # @.str3
.L.str3:
	.asciz	 "Should have terminated by now! Trying to return to bogus address...\n"
	.size	.L.str3, 69


	.section	".note.GNU-stack","",@progbits
