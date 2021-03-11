.PHONY:all
#export MACH

BIT_WIDTH:=$(shell getconf LONG_BIT)

ifeq ($(BIT_WIDTH),64)
MACH:=m64
else
MACH:=m32
endif

all: ddrinit rsa2048pem_gen rsa4096pem_gen
ddrinit:
	pushd ddr_init;make;./mkddrinit.sh u-boot-original.bin;popd
	cp ./ddr_init/ddr_init_reg_info.bin ./

rsa2048pem_gen: ddrinit
	./rsa2048pem.sh
	sh create_secure_boot.sh rsa2048pem_gen

rsa4096pem_gen: ddrinit
	./rsa4096pem.sh
	sh create_secure_boot.sh rsa4096pem_gen

clean:
	pushd ddr_init;make clean;popd
	find . -name "*.bin" -exec rm -rf {} \;
distclean:clean
	rm rsa2048pem/*
	rm rsa4096pem/*
	rm -rf aes_otp_cfg.txt
