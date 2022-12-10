
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

# mysql-client 

RUN apt-get install -y build-essential cmake git openssh-server unzip tar\
    gdb pkg-config valgrind systemd-coredump g++ gcc python3 python3-pip curl zip ninja-build software-properties-common\
    libssl-dev vim gnupg zsh clangd clang-tidy clang-format

RUN pip3 install requests mysql-connector-python

RUN useradd -m gvarkey
RUN usermod -aG sudo gvarkey
RUN mkdir -p /home/gvarkey/.ssh
COPY id_rsa /home/gvarkey/.ssh/id_rsa
RUN chown -R gvarkey:gvarkey /home/gvarkey/.ssh
RUN echo "Host github.com\n\tStrictHostKeyChecking no\n" >> /home/gvarkey/.ssh/config
USER gvarkey
CMD ["/bin/bash"]



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
RUN vcpkg install arrow
RUN vcpkg install parquet
RUN vcpkg install parquet
RUN vcpkg install libcuckoo
RUN vcpkg install jemalloc

WORKDIR "/home/gvarkey/gremlin"
RUN wget https://www.apache.org/dyn/closer.lua/tinkerpop/3.6.1/apache-tinkerpop-gremlin-console-3.6.1-bin.zip
RUN wget https://www.apache.org/dyn/closer.lua/tinkerpop/3.6.1/apache-tinkerpop-gremlin-server-3.6.1-bin.zip
RUN unzip apache-tinkerpop-gremlin-console-3.6.1-bin.zip
RUN unzip apache-tinkerpop-gremlin-server-3.6.1-bin.zip



WORKDIR "/home/gvarkey/workspace"
RUN git clone git@github.com:varkeyg/thirteenf.git


