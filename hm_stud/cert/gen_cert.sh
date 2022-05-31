#!/bin/sh
openssl genrsa -out test.com.key 1024
openssl req -new -key test.com.key -out test.com.csr
openssl x509 -req -days 365 -in test.com.csr -signkey test.com.key -out test.com.crt
cat test.com.key test.com.crt > openssl genrsa -out test.com.key 1024
openssl req -new -key test.com.key -out test.com.csr
openssl x509 -req -days 365 -in test.com.csr -signkey test.com.key -out test.com.crt
cat test.com.key test.com.crt > test.com.pem
