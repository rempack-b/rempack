echo "running docker"
cmake . -B cmake-build-docker --preset Release
cmake --build cmake-build-docker
chmod 777 -R cmake-build-docker