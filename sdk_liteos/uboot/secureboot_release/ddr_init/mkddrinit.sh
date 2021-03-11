#! /bin/sh

[ -n "$1" ] || { 
	echo ""; 
	echo "usage:"; 
	echo "     mkddrinit.sh <uboot bin>"; 
	echo "     e.g." 
		echo "     mkddrinit.sh u-boot.bin"; 
	echo ""; 
	exit 1; 
}

if [ -f ../$1 ]; then
    echo "get reginfo.bin from $1..."
else
    echo "no $1!!!"
    exit
fi
dd if=../$1 of=./fb4 bs=1 count=8192 skip=64
dd if=./ddr_init.bin of=./fb1 bs=1 count=64
dd if=./fb4 of=./fb2 bs=8192 conv=sync
dd if=./ddr_init.bin of=./fb3 bs=1 skip=8256
cat fb1 fb2 fb3 > ddr_init_reg_info.bin
rm -f fb1 fb2 fb3 fb4

