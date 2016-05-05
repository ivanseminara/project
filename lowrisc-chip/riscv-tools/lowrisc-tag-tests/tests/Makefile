#=======================================================================
# Makefile for LowRISC tag tests
#-----------------------------------------------------------------------

default: all

#--------------------------------------------------------------------
# Build rules
#--------------------------------------------------------------------

RISCV_PREFIX=riscv64-unknown-elf-
RISCV_GCC = $(RISCV_PREFIX)gcc
RISCV_OBJDUMP = $(RISCV_PREFIX)objdump
RISCV_LINUX_PREFIX=riscv64-unknown-linux-gnu-
RISCV_LINUX_GCC = $(RISCV_LINUX_PREFIX)gcc
RISCV_GCC_OPTS = -I.. -O3
RISCV_LINUX_GCC_OPTS = -I.. -static -g -O3

#--------------------------------------------------------------------
# Build targets
#--------------------------------------------------------------------

tag_tests = \
	tag_ld_st \
	parity \
	memory_sweep \
	tag_protection1 \
	tag_protection2 \
	tag_check_test \
	tagged_register_test \
	tagged_register_amo_test \
	tagged_register_lrsc_test \
	subword_tag_check_test \
	unaligned_access \
	amo_tag_check_test

tag_tests_linux = $(addsuffix .linux, $(tag_tests))
tag_tests_disass = $(addsuffix .txt, $(tag_tests))

#--------------------------------------------------------------------
# Build
#--------------------------------------------------------------------

$(tag_tests): %:%.cc ../env/tag.h
	$(RISCV_GCC) $(RISCV_GCC_OPTS) $< -o $@

$(tag_tests_linux): %.linux:%.cc ../env/tag.h
	$(RISCV_LINUX_GCC) $(RISCV_LINUX_GCC_OPTS) $< -o $@

$(tag_tests_disass): %.txt:%
	$(RISCV_OBJDUMP) -d -S $< > $@

all: $(tag_tests)

linux: $(tag_tests_linux)

clean:
	rm -fr $(tag_tests) $(tag_tests_linux) $(tag_tests_disass)
