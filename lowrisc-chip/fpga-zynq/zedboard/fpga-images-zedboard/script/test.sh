#!/bin/bash
cur_path=$PWD
if [[ $cur_path == *"tests/isa"* ]] || [[ $cur_path == *"tests/mt"* ]] || [[ $cur_path == *"tests/benchmark"* ]]
then
	echo "running pre-loaded test cases...";
	find * -exec echo "*** testing " {} " ***" \; -exec /home/root/fesvr-zynq {} \; -o -exec echo "!!! test case " {} " failed!!!" \; ;
elif [[ $cur_path == *"tests/tag"* ]]
then 
	echo "running pre-loaded test cases...";
	find * -exec echo "*** testing " {} " ***" \; -exec /home/root/fesvr-zynq /home/root/pk {} \; -o -exec echo "!!! test case " {} " failed!!!" \; ;
else
	echo "wrong directory";
fi

