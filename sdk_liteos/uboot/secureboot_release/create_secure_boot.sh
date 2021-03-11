#! /bin/sh

#If want to decrypt ddr_init.bin,u-boot.bin and their signatures,
#you must to set the KEY and IV for aes, and IV can't be zero;
#otherwise, ddr_init.bin, u-boot.bin and their signatures would
#not be decrypted.

#The IV and KEY must be used at the same time.

#The length of IV should be 16 Byte.
IV=

#The length of KEY should be 32 Byte.
KEY=

#please set ddr_file uboot_file
ddr_init_file=ddr_init_reg_info.bin
uboot_file=u-boot-original.bin

if [ ! -e $uboot_file ] ||  [ ! -e $ddr_init_file ];then
echo ==================================================================================
echo "  please set ddr_init_file/uboot_file !!! "
echo ==================================================================================
exit 1 ;
fi
echo "";
echo "usage:";
echo "     ddr_init_file = $ddr_init_file";
echo ""
echo "     uboot_file    = $uboot_file";
echo "";
echo "     IV            = $IV";
echo "";
echo "     KEY           = $KEY";
echo "";


dec2hex(){
	printf "0x%08x" $1
}

function H_TO_NL {
        local tmp1=$[$1 & 0xff]
        local tmp2=$[$[$1 & 0xff00] >> 8]
        local tmp3=$[$[$1 & 0xff0000] >> 16]
        local tmp4=$[$[$1 & 0xff000000] >> 24]
        local val=$[$[$tmp1 << 24] | $[$tmp2 << 16] | $[$tmp3 << 8] | $tmp4]

        echo $val
}

function HASH_OTP_TABLE {
	Str=`cat $1`
	echo $Str > $2
	echo ================================================================================== >> $2
	echo "input_file: $1" >> $2
	echo ================================================================================== >> $2
	for((i=0; i<64; i=i+8))
	do
		word=${Str:i:8};
		word=$(printf "0x%s" $word)
		word=$(H_TO_NL $word)
		word=$(printf "0x%08x" $word)
		reg=$[0x100b000c + $(($i/2))]     # OTP reg
		reg=$(printf "%08x" $reg)
		echo rootkey_hash[$(($i/8))]=mw 0x$reg $word  >> $2
	done
}

function AES_OTP_TABLE {
	Str=$1
	echo "AES KEY FILE" > $2
	echo ================================================================================== >> $2
	echo "AES KEY:: $1" >> $2
	echo ================================================================================== >> $2
	for((i=0; i<64; i=i+8))
	do
		word=${Str:i:8};
		word=$(printf "0x%s" $word)
		word=$(H_TO_NL $word)
		word=$(printf "0x%08x" $word)
		reg=$[0x100b000c + $(($i/2))]     # OTP reg
		reg=$(printf "%08x" $reg)
		echo aes_key_val[$(($i/8))]=mw 0x$reg $word  >> $2
	done
}

##################2048############################
if [ $1 = "rsa2048pem_gen" ];then
if [ -f rsa2048pem/rsa_pub_2048.pem ]; then
echo "....................rsa_2048........................."
#4:RSA_pub N+E
openssl base64 -d -in rsa2048pem/rsa_pub_2048.pem -out private.bin
dd if=./private.bin of=./fb1 bs=1 skip=33 count=256
for((i=1;i<=253;i++))
do
	echo 0x00 | xxd -r >> fb2
done
dd if=./private.bin of=./fb3 bs=1 skip=291 count=3
cat fb1 fb2 fb3 > all.bin
cp all.bin rsa2048pem/rsa_pub_2048.bin
filesize=`wc -c < all.bin`
if [ $filesize == 512 ];then
echo ""
echo 0:RSA_PUB creat OK!
echo RSA_PUB file_size = $filesize
echo ""
else
echo 0:RSA_PUB creat error!
echo RSA_PUB file_size = $filesize
echo ""
fi
rm -f fb1 fb2 fb3 private.bin

#5:IV
if [ $IV ];then
echo 0x$IV | xxd -r >> all.bin
else
echo 0x00000000000000000000000000000000 | xxd -r >> all.bin
fi

#6:DDR_len
#1)The ddr image must be filled with 16 bytes.
filesize=`wc -c < $ddr_init_file`
echo "1:The ddr image must be 16-byte aligned!"
echo $ddr_init_file dec_size = $filesize
a=$(($filesize % 16))
if [ $a == 0 ];then
b=0
else
b=$((16-$a))
fi
cp $ddr_init_file ddr_16byte_alig.bin
for((i=1;i<=$b;i++))
do
	echo 0x00 | xxd -r >> ddr_16byte_alig.bin
done
filesize=`wc -c < ddr_16byte_alig.bin`
echo ddr_16byte_alig.bin dec_size = $filesize
echo ""
#2)fill iamge len
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r > ddr_len.txt
#big_lit ddr_len.txt
cat ddr_len.txt >> all.bin

#7:DDR.BIN
cat ddr_16byte_alig.bin >> all.bin

#8:ddr_sig
openssl dgst -sha256 -sign rsa2048pem/rsa_priv_2048.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out ddr_sig.bin ddr_16byte_alig.bin
if [ -f ddr_sig.bin ]; then
echo 2:creat ddr_sig.bin ok!
echo ""
fi

cat ddr_sig.bin >> all.bin

#9:u-boot_len
#1)The boot image must be filled with 16 bytes.
filesize=`wc -c < $uboot_file`
echo "3:The boot image must be 16-byte aligned!"
echo $uboot_file dec_size = $filesize
a=$(($filesize % 16))
if [ $a == 0 ];then
b=0
else
b=$((16-$a))
fi
cp $uboot_file uboot_16byte_alig.bin
for((i=1;i<=$b;i++))
do
	echo 0x00 | xxd -r >> uboot_16byte_alig.bin
done
#2)fill iamge len
filesize=`wc -c < uboot_16byte_alig.bin`
filesize=$[filesize]
echo uboot_16byte_alig.bin dec_size = $filesize
echo ""
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r > uboot_len.txt
#big_lit uboot_len.txt
cat uboot_len.txt >> all.bin

#10:u-boot.bin + uboot_sing.bin
if [ $KEY ]; then
#IV and KEY have set,
#1) Obtain a new KEY by decrypting the ECB mode.
echo 0x50db86c592c52f0c436cca6f2ffecaf5 | xxd -r > seed_1.bin
echo 0x4a96ae013fc60e205e9da4c9d5ad9b99 | xxd -r > seed_2.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$KEY"  -in seed_1.bin -out out_1.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$KEY"  -in seed_2.bin -out out_2.bin
cat out_2.bin >> out_1.bin
KEY_ecb=$(xxd -ps out_1.bin | sed 'N;s/\n//g')
rm out_*.bin seed_*.bin
echo 4:Obtain a new KEY by decrypting the ECB mode!
echo new_KEY = $KEY_ecb
echo ""
#boot_sig+boot ---->> openssl_cbc ------>> cbc mode boot
#2)boot_sig+boot
openssl dgst -sha256 -sign rsa2048pem/rsa_priv_2048.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out uboot_sig.bin uboot_16byte_alig.bin
if [ -f uboot_sig.bin ];then
echo 5:AES:creat uboot_sig.bin OK!
echo ""
else
echo 5:AES:creat uboot_sig.bin error!
echo ""
fi
cp uboot_16byte_alig.bin u-cbc.bin
cat uboot_sig.bin >> u-cbc.bin
#3) Use the new KEY and IV to encrypt the image in CBC mode.
if [ $IV ];then
openssl enc -aes-256-cbc -nopad -K "$KEY_ecb"  -iv "$IV" -in u-cbc.bin -out cbc_boot.bin
else
echo error: please set IV!
fi
cat cbc_boot.bin >> all.bin
rm u-cbc.bin cbc_boot.bin

else
#If the IV and KEY are not set, use the default image.
#1)boot_bin
cat uboot_16byte_alig.bin >> all.bin
#2):boot_sig
openssl dgst -sha256 -sign rsa2048pem/rsa_priv_2048.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out uboot_sig.bin uboot_16byte_alig.bin
if [ -f uboot_sig.bin ];then
echo 4:RSA:creat uboot_sig.bin OK!
echo ""
else
echo 4:RSA:creat uboot_sig.bin error!
fi
cat uboot_sig.bin >> all.bin
fi

#1:MAGIC
touch head.txt
val=0x4253424d
val=$(H_TO_NL $val)
val=$(dec2hex $val)
echo $val | xxd -r > head.txt

#2:TOTAL_LEN
filesize=`wc -c < all.bin`
filesize=$[$filesize+16]
echo all.bin dec_size = $filesize
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r >> head.txt

#3:RSA2048
a=0x00000100
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r >> head.txt
echo $a | xxd -r >> head.txt

#######big->lit##########
#touch head_e.txt
#Fill in the first 16 bytes of the image.
cat all.bin  >> head.txt
cat head.txt > u-boot-rsa2048.bin

#Gets the hash value of the public KEY
echo Gets the hash value of the public KEY!
dd if=./u-boot-rsa2048.bin of=rsa2048pem/rsa_pub_2048.bin bs=1 skip=16 count=512
openssl dgst -sha256 -r  -hex rsa2048pem/rsa_pub_2048.bin >rsa2048pem/rsa_pub_2048_sha256.txt


HASH_OTP_TABLE rsa2048pem/rsa_pub_2048_sha256.txt rsa2048pem/rsa2048_pem_hash_val.txt
cat rsa2048pem/rsa2048_pem_hash_val.txt

#clean
rm ddr_16byte_alig.bin uboot_16byte_alig.bin all.bin
rm *.txt *_sig.bin
echo "....................................................."
echo
echo
fi
fi

##################################################
####################4096##########################
#################################################
if [ $1 = "rsa4096pem_gen" ];then
if [ -f rsa4096pem/rsa_pub_4096.pem ]; then
echo "....................rsa_4096........................."
#4:RSA_pub
openssl base64 -d -in rsa4096pem/rsa_pub_4096.pem -out private_4096.bin
dd if=./private_4096.bin of=./fb1 bs=1 skip=33 count=512
for((i=1;i<=509;i++))
do
	echo 0x00 | xxd -r >> fb2
done
dd if=./private_4096.bin of=./fb3 bs=1 skip=547 count=3
cat fb1 fb2 fb3 > all.bin

filesize=`wc -c < all.bin`
if [ $filesize == 1024 ];then
	echo ""
	echo 0:RSA_PUB creat OK!
	echo RSA_PUB file_size = $filesize
	echo ""
else
	echo 0:RSA_PUB creat error!
	echo RSA_PUB file_size = $filesize
	echo ""
fi
rm -f fb1 fb2 fb3 private.bin

#5:IV
if [ $IV ];then
echo 0x$IV | xxd -r >> all.bin
else
echo 0x00000000000000000000000000000000 | xxd -r >> all.bin
fi

#6:DDR_len
#1)The ddr image must be filled with 16 bytes.
filesize=`wc -c < $ddr_init_file`
echo "1:The ddr image must be 16-byte aligned!"
echo $ddr_init_file dec_size = $filesize
a=$(($filesize % 16))
if [ $a == 0 ];then
b=0
else
b=$((16-$a))
fi
cp $ddr_init_file ddr_16byte_alig.bin
for((i=1;i<=$b;i++))
do
	echo 0x00 | xxd -r >> ddr_16byte_alig.bin
done
filesize=`wc -c < ddr_16byte_alig.bin`
echo ddr_16byte_alig.bin dec_size = $filesize
echo ""
#2)fill iamge len
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r > ddr_len.txt
#big_lit ddr_len.txt
cat ddr_len.txt >> all.bin

#7:DDR.BIN
openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out ddr_sig.bin ddr_16byte_alig.bin
cat ddr_16byte_alig.bin >> all.bin

#8:ddr_sig
cat ddr_sig.bin >> all.bin
if [ -f ddr_sig.bin ]; then
echo 2:creat ddr_sig.bin ok!
echo ""
fi

#9:u-boot_len
#1)The boot image must be filled with 16 bytes.
filesize=`wc -c < $uboot_file`
echo "3:The boot image must be 16-byte aligned!"
echo $uboot_file dec_size = $filesize
a=$(($filesize % 16))
if [ $a == 0 ];then
b=0
else
b=$((16-$a))
fi
cp $uboot_file uboot_16byte_alig.bin
for((i=1;i<=$b;i++))
do
	echo 0x00 | xxd -r >> uboot_16byte_alig.bin
done
#2)fill iamge len
filesize=`wc -c < uboot_16byte_alig.bin`
filesize=$[filesize]
echo uboot_16byte_alig.bin dec_size = $filesize
echo ""
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r > uboot_len.txt
#big_lit uboot_len.txt
cat uboot_len.txt >> all.bin

#10:u-boot.bin + uboot_sing.bin
if [ $KEY ]; then
#IV and KEY have set,
#1) Obtain a new KEY by decrypting the ECB mode.
echo 0x50db86c592c52f0c436cca6f2ffecaf5 | xxd -r > seed_1.bin
echo 0x4a96ae013fc60e205e9da4c9d5ad9b99 | xxd -r > seed_2.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$KEY"  -in seed_1.bin -out out_1.bin
openssl enc -nopad -d -nosalt -aes-256-ecb -K "$KEY"  -in seed_2.bin -out out_2.bin
cat out_2.bin >> out_1.bin
KEY_ecb=$(xxd -ps out_1.bin | sed 'N;s/\n//g')
rm out_*.bin seed_*.bin
echo 4:Obtain a new KEY by decrypting the ECB mode!
echo new_KEY = $KEY_ecb
echo ""
#boot_sig+boot ---->> openssl_cbc ------>> cbc mode boot
#2)boot_sig+boot
openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out uboot_sig.bin uboot_16byte_alig.bin
if [ -f uboot_sig.bin ];then
echo 5:AES:creat uboot_sig.bin OK!
echo ""
else
echo 5:AES:creat uboot_sig.bin error!
echo ""
fi

cp uboot_16byte_alig.bin u-cbc.bin
cat uboot_sig.bin >> u-cbc.bin
#3) Use the new KEY and IV to encrypt the image in CBC mode.
if [ $IV ];then
openssl enc -aes-256-cbc -nopad -K "$KEY_ecb"  -iv "$IV" -in u-cbc.bin -out cbc_boot.bin
fi
cat cbc_boot.bin >> all.bin
rm u-cbc.bin cbc_boot.bin

else
#If the IV and KEY are not set, use the default image.
#1)boot_bin
cat uboot_16byte_alig.bin >> all.bin
#2):boot_sig
openssl dgst -sha256 -sign rsa4096pem/rsa_priv_4096.pem -sigopt rsa_padding_mode:pss -sigopt rsa_pss_saltlen:-1 -out uboot_sig.bin uboot_16byte_alig.bin
if [ -f uboot_sig.bin ];then
echo 4:RSA:creat uboot_sig.bin OK!
echo ""
else
echo 4:RSA:creat uboot_sig.bin error!
echo ""
fi
cat uboot_sig.bin >> all.bin
fi

#1:MAGIC
touch head.txt
val=0x4253424D
val=$(H_TO_NL $val)
val=$(dec2hex $val)
echo $val | xxd -r > head.txt

#2:TOTAL_LEN
filesize=`wc -c < all.bin`
filesize=$[$filesize+16]
echo all.bin dec_size = $filesize
a=$(dec2hex $filesize)
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r >> head.txt

#3:RSA4096
a=0x00000200
a=$(H_TO_NL $a)
a=$(dec2hex $a)
echo $a | xxd -r >> head.txt
echo $a | xxd -r >> head.txt

#######big->lit##########
#touch head_e.txt
#Fill in the first 16 bytes of the image.
cat all.bin  >> head.txt
cat head.txt > u-boot-rsa4096.bin

#Gets the hash value of the public KEY
echo Gets the hash value of the public KEY!
dd if=./u-boot-rsa4096.bin of=rsa4096pem/rsa_pub_4096.bin bs=1 skip=16 count=1024
openssl dgst -sha256 -r  -hex rsa4096pem/rsa_pub_4096.bin >rsa4096pem/rsa_pub_4096_sha256.txt

HASH_OTP_TABLE rsa4096pem/rsa_pub_4096_sha256.txt  rsa4096pem/rsa4096_pem_hash_val.txt
cat rsa4096pem/rsa4096_pem_hash_val.txt

#clean
rm ddr_16byte_alig.bin uboot_16byte_alig.bin all.bin
rm *.txt *_sig.bin
echo "....................................................."
fi
fi

if [ $KEY ]; then
AES_OTP_TABLE $KEY aes_otp_cfg.txt
echo  create aes_otp_cfg.txt over!
fi
