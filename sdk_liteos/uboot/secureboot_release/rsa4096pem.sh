#! /bin/sh


echo "======================================================================================"
echo "|                                     rsa4096pem                                      |"
echo "======================================================================================"
# generate rsa pub key and private key
if [ -f rsa4096pem/rsa_pub_4096_sha256.txt ]; then
	echo "use old rsa4096pem/rsa_pub.bin rsa4096pem/rsa_pub_4096_sha256.txt"
else
	echo "gen new rsa4096pem/rsa_pub.bin rsa4096pem/rsa_pub_4096_sha256.txt"
	openssl genrsa -out rsa4096pem/rsa_priv_4096.pem  4096
	openssl rsa -in rsa4096pem/rsa_priv_4096.pem -pubout -out rsa4096pem/rsa_pub_4096.pem
fi
