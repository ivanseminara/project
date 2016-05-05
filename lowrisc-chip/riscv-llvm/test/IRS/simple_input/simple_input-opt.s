	.file	"simple_input-opt.bc"
	.text
	.globl	function1
	.align	2
	.type	function1,@function
function1:                              # @function1
# BB#0:                                 # %entry
	addi	x2, x2, -48
	addiw	x5, x0, 1
	wrt	x1, x1, x5
	sdct	x1, 40(x2)              # 8-byte Folded Spill
	wrt	x3, x3, x5
	sdct	x3, 32(x2)              # 8-byte Folded Spill
	wrt	x9, x9, x5
	sdct	x9, 24(x2)              # 8-byte Folded Spill
	wrt	x8, x8, x5
	sdct	x8, 16(x2)              # 8-byte Folded Spill
	add	x8, x0, x2
	lui	x5, %hi(.L.str)
	addi	x10, x5, %lo(.L.str)
	jal	printf
	lui	x5, %hi(.L.str1)
	addi	x10, x5, %lo(.L.str1)
	jal	printf
	addi	x9, x8, 6
	addi	x10, x9, 0
	jal	gets
	lui	x5, %hi(.L.str2)
	addi	x10, x5, %lo(.L.str2)
	addi	x11, x9, 0
	jal	printf
	add	x2, x0, x8
	ld	x1, 40(x2)              # 8-byte Folded Reload
	stag	x0, 40(x2)              # 8-byte Folded Reload
	ld	x3, 32(x2)              # 8-byte Folded Reload
	stag	x0, 32(x2)              # 8-byte Folded Reload
	ld	x9, 24(x2)              # 8-byte Folded Reload
	stag	x0, 24(x2)              # 8-byte Folded Reload
	ld	x8, 16(x2)              # 8-byte Folded Reload
	stag	x0, 16(x2)              # 8-byte Folded Reload
	addi	x2, x2, 48
	ret
.Ltmp3:
	.size	function1, .Ltmp3-function1

	.globl	function2
	.align	2
	.type	function2,@function
function2:                              # @function2
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
	lui	x5, %hi(.L.str3)
	addi	x10, x5, %lo(.L.str3)
	jal	printf
	add	x2, x0, x8
	ld	x1, 24(x2)              # 8-byte Folded Reload
	stag	x0, 24(x2)              # 8-byte Folded Reload
	ld	x3, 16(x2)              # 8-byte Folded Reload
	stag	x0, 16(x2)              # 8-byte Folded Reload
	ld	x8, 8(x2)               # 8-byte Folded Reload
	stag	x0, 8(x2)               # 8-byte Folded Reload
	addi	x2, x2, 32
	ret
.Ltmp7:
	.size	function2, .Ltmp7-function2

	.globl	main
	.align	2
	.type	main,@function
main:                                   # @main
# BB#0:                                 # %entry
	addi	x2, x2, -48
	addiw	x5, x0, 1
	wrt	x1, x1, x5
	sdct	x1, 40(x2)              # 8-byte Folded Spill
	wrt	x3, x3, x5
	sdct	x3, 32(x2)              # 8-byte Folded Spill
	wrt	x8, x8, x5
	sdct	x8, 24(x2)              # 8-byte Folded Spill
	add	x8, x0, x2
	sw	x0, 20(x8)
	jal	function1
	lui	x5, %hi(function2)
	addi	x5, x5, %lo(function2)
	addi	x6, x8, 8
	li	x7, 4
	wrt	x5, x5, x7
	sdct	x5, 0(x6)
	ldct	x5, 0(x6)
	rdt	x6, x5
	bne	x6, x7, .LBB2_2
# BB#1:
	jalr	x5,0
	addiw	x10, x0, 0
	add	x2, x0, x8
	ld	x1, 40(x2)              # 8-byte Folded Reload
	stag	x0, 40(x2)              # 8-byte Folded Reload
	ld	x3, 32(x2)              # 8-byte Folded Reload
	stag	x0, 32(x2)              # 8-byte Folded Reload
	ld	x8, 24(x2)              # 8-byte Folded Reload
	stag	x0, 24(x2)              # 8-byte Folded Reload
	addi	x2, x2, 48
	ret
.LBB2_2:
	jal	__llvm_riscv_check_tagged_failure
.Ltmp11:
	.size	main, .Ltmp11-main

	.section	.text.__llvm_riscv_init_tagged_memory_csrs,"axG",@progbits,__llvm_riscv_init_tagged_memory_csrs,comdat
	.weak	__llvm_riscv_init_tagged_memory_csrs
	.align	2
	.type	__llvm_riscv_init_tagged_memory_csrs,@function
__llvm_riscv_init_tagged_memory_csrs:   # @__llvm_riscv_init_tagged_memory_csrs
	.cfi_startproc
# BB#0:                                 # %entry
	addi	x2, x2, -8
.Ltmp15:
	.cfi_def_cfa_offset 8
	addiw	x5, x0, 1
	wrt	x8, x8, x5
	sdct	x8, 0(x2)               # 8-byte Folded Spill
.Ltmp16:
	.cfi_offset x8, -8
	add	x8, x0, x2
.Ltmp17:
	.cfi_def_cfa_register x8
	li	x5, 12
	csrw	ld_tag, x5
	li	x5, 10
	csrw	sd_tag, x5
	add	x2, x0, x8
	ld	x8, 0(x2)               # 8-byte Folded Reload
	stag	x0, 0(x2)               # 8-byte Folded Reload
	addi	x2, x2, 8
	ret
.Ltmp18:
	.size	__llvm_riscv_init_tagged_memory_csrs, .Ltmp18-__llvm_riscv_init_tagged_memory_csrs
	.cfi_endproc

	.section	.text.__llvm_riscv_check_tagged_failure,"axG",@progbits,__llvm_riscv_check_tagged_failure,comdat
	.weak	__llvm_riscv_check_tagged_failure
	.align	2
	.type	__llvm_riscv_check_tagged_failure,@function
__llvm_riscv_check_tagged_failure:      # @__llvm_riscv_check_tagged_failure
	.cfi_startproc
# BB#0:                                 # %entry
	addi	x2, x2, -32
.Ltmp22:
	.cfi_def_cfa_offset 32
	addiw	x5, x0, 1
	wrt	x1, x1, x5
	sdct	x1, 24(x2)              # 8-byte Folded Spill
	wrt	x3, x3, x5
	sdct	x3, 16(x2)              # 8-byte Folded Spill
	wrt	x8, x8, x5
	sdct	x8, 8(x2)               # 8-byte Folded Spill
.Ltmp23:
	.cfi_offset x1, -8
.Ltmp24:
	.cfi_offset x3, -16
.Ltmp25:
	.cfi_offset x8, -24
	add	x8, x0, x2
.Ltmp26:
	.cfi_def_cfa_register x8
	jal	abort
	add	x2, x0, x8
	ld	x1, 24(x2)              # 8-byte Folded Reload
	stag	x0, 24(x2)              # 8-byte Folded Reload
	ld	x3, 16(x2)              # 8-byte Folded Reload
	stag	x0, 16(x2)              # 8-byte Folded Reload
	ld	x8, 8(x2)               # 8-byte Folded Reload
	stag	x0, 8(x2)               # 8-byte Folded Reload
	addi	x2, x2, 32
	ret
.Ltmp27:
	.size	__llvm_riscv_check_tagged_failure, .Ltmp27-__llvm_riscv_check_tagged_failure
	.cfi_endproc

	.type	.L.str,@object          # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	 "Function 2\n"
	.size	.L.str, 12

	.type	.L.str1,@object         # @.str1
.L.str1:
	.asciz	 "Insert value: "
	.size	.L.str1, 15

	.type	.L.str2,@object         # @.str2
.L.str2:
	.asciz	 "value: %s\n"
	.size	.L.str2, 11

	.type	.L.str3,@object         # @.str3
.L.str3:
	.asciz	 "Function 3\n"
	.size	.L.str3, 12

	.section	.init_array.0,"aw",@init_array
	.align	3
	.quad	__llvm_riscv_init_tagged_memory_csrs

	.section	".note.GNU-stack","",@progbits
