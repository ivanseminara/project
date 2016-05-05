#! /bin/sh

INCLUDES="-I. -I $RISCV/riscv64-unknown-elf/include"

main=$( basename $1 )
main=stack_protection.c
name=${main%.c}

echo Compiling: ${main}
echo -e "\n##############################################################################\n"
echo Creating IR representation:
clang -O0 -target riscv -mcpu=LowRISC -mriscv=LowRISC $INCLUDES -S $main -emit-llvm -o $name.ll
echo -e "\toutput to ${name}.ll"

echo -e "\n##############################################################################\n"
echo Running irs_tagged_memory pass:
opt -load $TOP/riscv-llvm/build/Debug+Asserts/lib/irs_tagged_memory.so -irs_tagged_memory < $name.ll > ${name}-opt.bc

echo -e "\n##############################################################################\n"
echo Creating Disassembly of the optimized bytecode to check for successful rewrite:
llvm-dis ${name}.opt.bc -o=${name}-opt.ll
echo -e "\toutput to ${name}-opt.ll"

echo -e "\n##############################################################################\n"
echo Compiling Optimized version:
echo -e "\tAssembly output to ${name}-opt.s"
llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${name}-opt.bc -o ${name}-opt.s
riscv64-unknown-elf-gcc -o test-${name}-opt.riscv ${name}-opt.s 
echo -e "\tLinking output to test-${name}.riscv"

echo -e "\n##############################################################################\n"
echo Compiling Unoptimized version:
echo -e "\tAssembly output to ${name}.s"
llc -use-init-array -filetype=asm -march=riscv -mcpu=LowRISC ${name}.ll -o ${name}.s
riscv64-unknown-elf-gcc -o test-${name}.riscv ${name}.s 
echo -e "\tLinking output to test-${name}.riscv"

echo -e "\n##############################################################################\n"
echo Running $1
spike pk test-${name}-opt.riscv
