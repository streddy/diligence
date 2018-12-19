#!/bin/bash

PWD="$(pwd)"
# NOTE: change this path depending on the location of your OpenSSL source
OPENSSL_LOC="/usr/local/src/openssl-1.1.1a/apps/ct_log_list.cnf"

# clear existing symlink
sudo rm $OPENSSL_LOC

# signature checks are currently skipped to avoid python library issues in the parsing script
python print_log_list.py --log_list=log-list/log_list.json --log_list_schema=log-list/log_list_schema.json --openssl_output=True --skip_signature_check
mv True ct_log_list.cnf

sudo ln -s $PWD/ct_log_list.cnf $OPENSSL_LOC
