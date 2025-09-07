#!/bin/bash

# Always create files in the same directory as this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Step 1: Generate CA Key and Certificate (Self-Signed)
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout "$SCRIPT_DIR/ca_key.pem" \
  -out "$SCRIPT_DIR/ca_cert.pem" \
  -sha256 -days 3650 \
  -subj "/C=US/ST=California/L=LosAngeles/O=MyCA/OU=Security/CN=MyRootCA"

# Step 2: Generate Server Key and CSR (Certificate Signing Request)
openssl req -newkey rsa:2048 -nodes \
  -keyout "$SCRIPT_DIR/server_key.pem" \
  -out "$SCRIPT_DIR/server_req.pem" \
  -subj "/C=US/ST=California/L=LosAngeles/O=MyServer/OU=Web/CN=localhost"

# Step 3: Sign Server CSR with CA to Create Server Certificate
openssl x509 -req \
  -in "$SCRIPT_DIR/server_req.pem" \
  -CA "$SCRIPT_DIR/ca_cert.pem" -CAkey "$SCRIPT_DIR/ca_key.pem" \
  -out "$SCRIPT_DIR/server_cert.pem" \
  -days 825 -sha256 \
  -set_serial 01 \
  -extfile <(printf "subjectAltName=DNS:localhost,IP:127.0.0.1")
