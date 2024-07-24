#!/bin/bash

REMARKABLE_HOST="remarkable"
APP_PATH=${1}
APP=$(basename "${APP_PATH}")
BASE_DIR="/home/root/${APP}"
RM_USER="root"

ssh ${RM_USER}@${REMARKABLE_HOST} "systemctl stop remux; systemctl stop xochitl; killall gdbserver; killall ${APP}"
#this is probably brittle, I'm sure it's fine
scp ${APP_PATH} ${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/${APP}
echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
ssh ${RM_USER}@${REMARKABLE_HOST} "cd ${BASE_DIR}; gdbserver :1243 /opt/bin/rm2fb-client ./${APP}" &
