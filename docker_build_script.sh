#this is meant to be run inside the docker container
#running this on your host machine will most likely fail
cmake . -B cmake-build-docker --preset Release
cmake --build cmake-build-docker
chmod 777 -R cmake-build-docker