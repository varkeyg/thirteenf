
FROM ubuntu:22.04

LABEL "Owner"="Geo Varkey"
LABEL "email"="geo.varkey@gmal.com"



# this is for timezone config
ENV DEBIAN_FRONTEND=noninteractive 
ENV TZ=America/Los_Angeles
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update
RUN apt-get upgrade -y
RUN apt-get update


RUN apt-get install -y build-essential cmake git openssh-server unzip tar\
    gdb pkg-config valgrind systemd-coredump g++ gcc mysql-client python3 python3-pip curl zip ninja-build software-properties-common\
    libssl-dev vim gnupg

RUN pip3 install requests mysql-connector-python

ENV VCPKG_FORCE_SYSTEM_BINARIES=1 
WORKDIR /home/gvarkey
RUN git clone https://github.com/Microsoft/vcpkg.git
WORKDIR /home/gvarkey/vcpkg
RUN ./bootstrap-vcpkg.sh
ENV PATH="/home/gvarkey/vcpkg:${PATH}"

RUN vcpkg install curl
RUN vcpkg install tinyxml2
RUN vcpkg install gtest
RUN vcpkg install nlohmann-json
RUN vcpkg install glog
RUN vcpkg install tsl-ordered-map
RUN vcpkg install rocksdb
RUN vcpkg install boost

WORKDIR "/home/gvarkey/work"
