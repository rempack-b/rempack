#mostly stolen from rmkit
#copies your app to the tablet, runs it, then waits for interrupt
#before closing it and restarting the remarkable interface

REMARKABLE_HOST="remarkable"
APP=${1}
BASE_DIR="/home/root"
RM_USER="root"

function kill_remote_app() {
  ssh ${RM_USER}@${REMARKABLE_HOST} killall ${APP} 2> /dev/null
}

function cleanup() {
  kill_remote_app
  ssh ${RM_USER}@${REMARKABLE_HOST} rm ${BASE_DIR}/${APP}
  ssh ${RM_USER}@${REMARKABLE_HOST} systemctl restart remux
  echo "FINISHED"
  exit 0
}

trap cleanup EXIT
trap cleanup SIGINT

#this is probably brittle, I'm sure it's fine
scp cmake-build-release-docker/${APP} ${RM_USER}@${REMARKABLE_HOST}:${BASE_DIR}/${APP}
kill_remote_app
echo "RUNNING ${APP}"
#remove rm2fb-client if you're running on a RM1
ssh ${RM_USER}@${REMARKABLE_HOST} "systemctl stop remux; systemctl stop xochitl"
ssh ${RM_USER}@${REMARKABLE_HOST} /opt/bin/rm2fb-client ${BASE_DIR}/${APP}
