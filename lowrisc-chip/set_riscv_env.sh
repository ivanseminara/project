#!/bin/bash
# source this file
echo "Setting up RISC-V environment..."
# Variables for RISC-V
if [ "$TOP" == "" ]; then
    echo "\$TOP is not available. So set it to the current directory $PWD."
    export TOP=$PWD
fi
export RISCV=$TOP/riscv
export PATH=/opt/riscv/bin:$RISCV/bin:$PATH
export CODE=$TOP/riscv-llvm/lib/Transforms/irs_tagged_memory
export TEST=$TOP/riscv-llvm/test/IRS
export BUILD=$TOP/riscv-llvm/build/

