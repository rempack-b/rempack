#!/bin/bash

REMARKABLE_HOST="remarkable"
APP_PATH=${1}
APP=$(basename "${APP_PATH}")
BASE_DIR="/home/root"
RM_USER="root"

function kill_remote_app() {
  ssh ${RM_USER}@${REMARKABLE_HOST} killall ${APP} 2> /dev/null
}

function cleanup() {
  kill_remote_app
  #ssh ${RM_USER}@${REMARKABLE_HOST} rm ${BASE_DIR}/${APP}
  ssh ${RM_USER}@${REMARKABLE_HOST} systemctl restart remux
  echo "FINISHED"
  trap - EXIT
  exit 0
}

trap cleanup EXIT
trap cleanup SIGINT

#this is probably brittle, I'm sure it's fine
scp ${APP_PATH} ${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/${APP}
kill_remote_app
echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
ssh ${RM_USER}@${REMARKABLE_HOST} "systemctl stop remux; systemctl stop xochitl"
ssh ${RM_USER}@${REMARKABLE_HOST} "gdbserver :1243 ${BASE_DIR}/${APP} --wrapper /opt/bin/rm2fb-client"
