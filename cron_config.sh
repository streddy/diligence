#!/bin/bash

# log list file location
LOC1="$(pwd)/list-gen/log-list"
LOC2="$(pwd)/list-gen"

# get current crontab
crontab -l > current

# download latest CT log lists at 01:00 AM daily
echo "0 1 * * * wget -O $LOC1/log_list.json https://www.gstatic.com/ct/log_list/log_list.json" >> current
echo "0 1 * * * wget -O $LOC1/log_list.sig https://www.gstatic.com/ct/log_list/log_list.sig" >> current
echo "0 1 * * * wget -O $LOC1/log_list_pubkey.pem https://www.gstatic.com/ct/log_list/log_list_pubkey.pem" >> current
echo "0 1 * * * wget -O $LOC1/log_list_schema.json https://www.gstatic.com/ct/log_list/log_list_schema.json" >> current
# configure local CT log list at 01:01 AM daily
echo "1 1 * * * cd $LOC2 && ./list-gen.sh" >> current

crontab current
rm current
