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
RISCV_LINUX_GCC_OPTS = -I.. -O3 -static -g -pthread

#--------------------------------------------------------------------
# Build targets
#--------------------------------------------------------------------

tag_tests = \
	simple_full_empty

tag_tests_linux = $(addsuffix .linux, $(tag_tests))
tag_tests_disass = $(addsuffix .txt, $(tag_tests))
tag_tests_disass_linux = $(addsuffix .linuxtxt, $(tag_tests))

#--------------------------------------------------------------------
# Build
#--------------------------------------------------------------------

$(tag_tests): %:%.cc ../env/tag.h
	$(RISCV_GCC) $(RISCV_GCC_OPTS) $< -o $@

$(tag_tests_linux): %.linux:%.cc ../env/tag.h ../env/full_empty.h
	$(RISCV_LINUX_GCC) $(RISCV_LINUX_GCC_OPTS) $< -o $@

$(tag_tests_disass): %.txt:%
	$(RISCV_OBJDUMP) -d  $< > $@

full_empty_spin.linux: full_empty.cc
	$(RISCV_LINUX_GCC) $(RISCV_LINUX_GCC_OPTS) $< -o $@

full_empty_sched.linux: full_empty.cc
	$(RISCV_LINUX_GCC) $(RISCV_LINUX_GCC_OPTS) -DSCHEDUL $< -o $@

all: $(tag_tests)

linux: $(tag_tests_linux)

clean:
	rm -fr $(tag_tests) $(tag_tests_linux) $(tag_tests_disass)
