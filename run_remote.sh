#mostly stolen directly from rmkit

APP=${1}
BASE_DIR="/home/root"
RM_USER="root"

function kill_remote_app() {
  ssh root@${HOST} killall ${APP} 2> /dev/null
}

function cleanup() {
  kill_remote_app
  echo "FINISHED"
  exit 0
}

trap cleanup EXIT
trap cleanup SIGINT

scp cmake-build-release-docker/${APP} ${RM_USER}@${HOST}:${BASE_DIR}/${APP}
kill_remote_app
echo "RUNNING ${APP}"
ssh ${RM_USER}@${HOST} ${BASE_DIR}/${APP}
