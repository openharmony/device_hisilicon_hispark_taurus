#!/bin/bash
#
# Copyright (c) 2020 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -e

function usage() {
    echo "";
    echo "usage:";
    echo "     sec_os.sh <kernel bin> <kernel addr> <rootfs img> <rootfs addr> <other bin> <other addr>";
    echo "     e.g."
    echo "     sec_os.sh kernel.bin 0x100000 rootfs.img 0xA00000";
    echo "";
    exit 1;
}

function dec2hex(){
    printf "0x%08x" $1
}

function h2nl() {
    local tmp1=$[$1 & 0xff]
    local tmp2=$[$[$1 & 0xff00] >> 8]
    local tmp3=$[$[$1 & 0xff0000] >> 16]
    local tmp4=$[$[$1 & 0xff000000] >> 24]
    local val=$[$[$tmp1 << 24] | $[$tmp2 << 16] | $[$tmp3 << 8] | $tmp4]

    echo $val
}

if [ $# == 0 ] || [ $(($#%2)) != 0 ]; then
    usage
fi

sig_num=$(expr $# / 2)

# for 2048
if [ -f rsa_priv_2048.pem ]; then

if [ -f x509_2048.bin ]; then
  rm x509_2048.bin
fi

#add x509 len
cp  x509_creater/rsa_public_key_2048.der rsa_public_key_2048.der
filesize=`wc -c < rsa_public_key_2048.der`

a=$(($filesize % 16))
if [ "$a" == 0 ];then
b=0
else
b=$((16-$a))
fi
cp  rsa_public_key_2048.der rsa_public_key_16byte_alig.bin
for((i=1;i<=$b;i++))
do
    echo 0x00 | xxd -r >> rsa_public_key_16byte_alig.bin
done
rm rsa_public_key_2048.der;
filesize=`wc -c < rsa_public_key_16byte_alig.bin`

# add total length
all_len=0;
all_len=$[filesize+all_len];
all_len=$[256+all_len];
all_len=$[12+all_len];
all_len=$[264*sig_num+all_len];

all_len=$(dec2hex $all_len)
all_len=$(h2nl $all_len)
all_len=$(dec2hex $all_len)

echo $all_len | xxd -r > all_len.txt
cat all_len.txt >>x509_2048.bin
rm all_len.txt

sig_num_2048=$(dec2hex $sig_num)
sig_num_2048=$(h2nl $sig_num_2048)
sig_num_2048=$(dec2hex $sig_num_2048)
echo $sig_num_2048 | xxd -r > sig_num.txt
cat sig_num.txt >>x509_2048.bin
rm sig_num.txt

x509_len=$filesize
x509_len=$(dec2hex $x509_len)
x509_len=$(h2nl $x509_len)
x509_len=$(dec2hex $x509_len)
echo $x509_len | xxd -r > x509_len.txt
cat x509_len.txt >>x509_2048.bin
rm x509_len.txt

cat rsa_public_key_16byte_alig.bin >>x509_2048.bin
rm rsa_public_key_16byte_alig.bin

#generate the signature
for((i=1;i<=sig_num;i++))
do
    bin_name_num=$(($i * 2 - 1))
    bin_addr_num=$(($i * 2))
    bin_name=${!bin_name_num}
    bin_addr=${!bin_addr_num}
    bin_addr_head=${bin_addr:0:2}

    if [ "$bin_addr_head" != "0x" ] && [ "$bin_addr_head" != "0X" ]; then
        echo "addr err!"
        usage
    fi

    if [ -f "$bin_name" ]; then
        #add signature
        openssl dgst -sha256 -sign x509_creater/rsa_private_2048.key -sigopt rsa_padding_mode:pss\
        -sigopt rsa_pss_saltlen:-1 -out {$bin_name}.txt $bin_name
        cat {$bin_name}.txt>> x509_2048.bin
        rm {$bin_name}.txt
        #add length
        filesize=`wc -c < $bin_name`
        filesize=$(dec2hex $filesize)
        filesize=$(h2nl $filesize)
        filesize=$(dec2hex $filesize)
        echo $filesize | xxd -r > filesize.txt
        cat filesize.txt >>x509_2048.bin
        rm filesize.txt
        #add address
        addr=$(dec2hex $bin_addr)
        addr=$(h2nl $addr)
        addr=$(dec2hex $addr)
        echo $addr | xxd -r > addr.txt
        cat addr.txt >>x509_2048.bin
        rm addr.txt
    else
        echo "no $bin_name!"
    fi
done

openssl dgst -sha256 -sign rsa_priv_2048.pem -sigopt rsa_padding_mode:pss\
 -sigopt rsa_pss_saltlen:-1 -out cert_sig.bin x509_2048.bin
cat cert_sig.bin >> x509_2048.bin
rm cert_sig.bin

else
    echo "no rsa_priv_2048.pem!"
fi


# for 4096
if [ -f rsa_priv_4096.pem ]; then

if [ -f x509_4096.bin ]; then
  rm x509_4096.bin
fi
cp  x509_creater/rsa_public_key_4096.der rsa_public_key_4096.der
#add x509 len
filesize=`wc -c < rsa_public_key_4096.der`
a=$(($filesize % 16))
if [ "$a" == 0 ];then
b=0
else
b=$((16-$a))
fi
cp rsa_public_key_4096.der rsa_public_key_16byte_alig.bin
for((i=1;i<=$b;i++))
do
    echo 0x00 | xxd -r >> rsa_public_key_16byte_alig.bin
done
rm rsa_public_key_4096.der
filesize=`wc -c < rsa_public_key_16byte_alig.bin`
# add total length
all_len=0;
all_len=$[filesize+all_len];
all_len=$[512+all_len];
all_len=$[12+all_len];
all_len=$[520*sig_num+all_len];

all_len=$(dec2hex $all_len)
all_len=$(h2nl $all_len)
all_len=$(dec2hex $all_len)

echo $all_len | xxd -r > all_len.txt
cat all_len.txt >>x509_4096.bin
rm all_len.txt

sig_num_4096=$(dec2hex $sig_num)
sig_num_4096=$(h2nl $sig_num_4096)
sig_num_4096=$(dec2hex $sig_num_4096)
echo $sig_num_4096 | xxd -r > sig_num.txt
cat sig_num.txt >>x509_4096.bin
rm sig_num.txt

x509_len=$filesize
x509_len=$(dec2hex $x509_len)
x509_len=$(h2nl $x509_len)
x509_len=$(dec2hex $x509_len)
echo $x509_len | xxd -r > x509_len.txt
cat x509_len.txt >>x509_4096.bin
rm x509_len.txt

cat rsa_public_key_16byte_alig.bin >>x509_4096.bin
rm rsa_public_key_16byte_alig.bin

#generate the signature
for((i=1;i<=sig_num;i++))
do
    bin_name_num=$(($i * 2 - 1))
    bin_addr_num=$(($i * 2))
    bin_name=${!bin_name_num}
    bin_addr=${!bin_addr_num}
    bin_addr_head=${bin_addr:0:2}

    if [ "$bin_addr_head" != "0x" ] && [ "$bin_addr_head" != "0X" ]; then
        echo "addr err!"
        usage
    fi

    if [ -f "$bin_name" ]; then
        #add signature
        openssl dgst -sha256 -sign x509_creater/rsa_private_4096.key -sigopt rsa_padding_mode:pss\
        -sigopt rsa_pss_saltlen:-1 -out {$bin_name}.txt $bin_name
        cat {$bin_name}.txt>> x509_4096.bin
        rm {$bin_name}.txt
        #add length
        filesize=`wc -c < $bin_name`
        filesize=$(dec2hex $filesize)
        filesize=$(h2nl $filesize)
        filesize=$(dec2hex $filesize)
        echo $filesize | xxd -r > filesize.txt
        cat filesize.txt >>x509_4096.bin
        rm filesize.txt
        #add address
        addr=$(dec2hex $bin_addr)
        addr=$(h2nl $addr)
        addr=$(dec2hex $addr)
        echo $addr | xxd -r > addr.txt
        cat addr.txt >>x509_4096.bin
        rm addr.txt
    else
        echo "no $bin_name!"
    fi
done

openssl dgst -sha256 -sign rsa_priv_4096.pem -sigopt rsa_padding_mode:pss\
 -sigopt rsa_pss_saltlen:-1 -out cert_sig.bin x509_4096.bin
cat cert_sig.bin >> x509_4096.bin
rm cert_sig.bin

else
    echo "no rsa_priv_4096.pem!"
fi