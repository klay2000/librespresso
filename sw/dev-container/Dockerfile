FROM python:3
WORKDIR /usr/src
# install dependencies for pico sdk
RUN apt update && apt install -y git gcc g++ cmake gcc-arm-none-eabi libnewlib-arm-none-eabi pkg-config build-essential libusb-1.0-0-dev

# define compiler env variables
Env CXX=g++ CC=gcc

# clone pico sdk
RUN git clone https://github.com/raspberrypi/pico-sdk.git

# init submodules
RUN cd pico-sdk && git submodule update --init

# define pico sdk path
ENV PICO_SDK_PATH=/usr/src/pico-sdk

# clone freertos kernel
RUN git clone -b smp https://github.com/FreeRTOS/FreeRTOS-Kernel --recurse-submodules

# define freertos kernel path
ENV FREERTOS_KERNEL_PATH=/usr/src/FreeRTOS-Kernel


# build and install picotool for flashing
RUN git clone https://github.com/raspberrypi/picotool.git &&\
    cd picotool &&\
    mkdir build &&\
    cd build &&\
    cmake .. &&\
    make &&\
    make install