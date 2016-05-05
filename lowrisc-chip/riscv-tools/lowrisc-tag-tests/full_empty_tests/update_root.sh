#!/bin/bash

# This script includes a binary (first argument)  into the linux image
# $TOP/riscv-tools/busybox-1.21.1/root.bin
if [ "$#" -le 0 ] 
	then
		echo "Please provide a program to load into the root image"
		exit 0
fi
if [ -z "$TOP" ] 
	then
		echo "Please set the environment variable TOP"
		exit 0
fi
echo "update the root image: add $1"
sudo mount -o loop $TOP/riscv-tools/busybox-1.21.1/root.bin $TOP/riscv-tools/busybox-1.21.1/mnt
sudo cp "$1" $TOP/riscv-tools/busybox-1.21.1/mnt/"$1"
sleep 1
sudo umount $TOP/riscv-tools/busybox-1.21.1/mnt

echo "done!"
