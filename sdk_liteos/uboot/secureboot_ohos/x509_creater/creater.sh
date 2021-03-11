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

if [ -f rsa_public_key_2048.der ]; then
  echo already have the cert!
  else
  openssl genrsa -out rsa_private_2048.key 2048
  openssl rsa -in rsa_private_2048.key -pubout -out rsa_public_2048.key
  openssl req -new -x509 -sha256 -key rsa_private_2048.key -out ca_2048.crt
  openssl x509 -outform der -inform pem -in ca_2048.crt -out rsa_public_key_2048.der
fi


if [ -f rsa_public_key_4096.der ]; then
  echo already have the cert!
  else
  openssl genrsa -out rsa_private_4096.key 4096
  openssl rsa -in rsa_private_4096.key -pubout -out rsa_public_4096.key
  openssl req -new -x509 -sha256 -key rsa_private_4096.key -out ca_4096.crt
  openssl x509 -outform der -inform pem -in ca_4096.crt -out rsa_public_key_4096.der
fi
