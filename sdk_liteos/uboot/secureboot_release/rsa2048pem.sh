#! /bin/sh


echo "======================================================================================"
echo "|                                     rsa2048pem                                      |"
echo "======================================================================================"
# generate rsa pub key and private key
if [ -f rsa2048pem/rsa_pub_2048_sha256.txt ]; then
	echo "use old rsa2048pem/rsa_pub.bin rsa2048pem/rsa_pub_2048_sha256.txt"
else
	echo "gen new rsa2048pem/rsa_pub.bin rsa2048pem/rsa_pub_2048_sha256.txt"
	openssl genrsa -out rsa2048pem/rsa_priv_2048.pem  2048
	openssl rsa -in rsa2048pem/rsa_priv_2048.pem -pubout -out rsa2048pem/rsa_pub_2048.pem
fi
