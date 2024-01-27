FROM ghcr.io/toltec-dev/python:latest

RUN pip install okp --break-system-packages

WORKDIR /project
COPY . /project/
RUN rm -rf src/build