#!/bin/bash
# Step 1: Generate CA Key and Certificate (Self-Signed)
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout ca_key.pem \
  -out ca_cert.pem \
  -sha256 -days 3650 \
  -subj "/C=US/ST=California/L=LosAngeles/O=MyCA/OU=Security/CN=MyRootCA"

# Step 2: Generate Server Key and CSR (Certificate Signing Request)
openssl req -newkey rsa:2048 -nodes \
  -keyout server_key.pem \
  -out server_req.pem \
  -subj "/C=US/ST=California/L=LosAngeles/O=MyServer/OU=Web/CN=localhost"

# Step 3: Sign Server CSR with CA to Create Server Certificate
openssl x509 -req \
  -in server_req.pem \
  -CA ca_cert.pem -CAkey ca_key.pem \
  -out server_cert.pem \
  -days 825 -sha256 \
  -set_serial 01 \
  -extfile <(printf "subjectAltName=DNS:localhost,IP:127.0.0.1")
