
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
    libssl-dev vim gnupg zsh clangd clang-tidy clang-format bash-completion 

RUN pip3 install requests mysql-connector-python

RUN useradd -m apache
RUN usermod -aG sudo apache
#RUN mkdir -p /home/apache/.ssh
#COPY id_rsa /home/apache/.ssh/id_rsa
#RUN chown -R apache:apache /home/apache/.ssh
#RUN echo "Host github.com\n\tStrictHostKeyChecking no\n" >> /home/apache/.ssh/config
USER apache
CMD ["/bin/bash"]



ENV VCPKG_FORCE_SYSTEM_BINARIES=1 
WORKDIR /home/apache
RUN git clone https://github.com/Microsoft/vcpkg.git
WORKDIR /home/apache/vcpkg
RUN ./bootstrap-vcpkg.sh
ENV PATH="/home/apache/vcpkg:${PATH}"

RUN vcpkg install curl
RUN vcpkg install tinyxml2
RUN vcpkg install gtest
RUN vcpkg install nlohmann-json
RUN vcpkg install glog
RUN vcpkg install tsl-ordered-map
RUN vcpkg install rocksdb
RUN vcpkg install boost
RUN vcpkg install sqlitecpp
RUN vcpkg install spdlog


WORKDIR "/home/apache/gremlin"
RUN wget https://dlcdn.apache.org/tinkerpop/3.6.1/apache-tinkerpop-gremlin-server-3.6.1-bin.zip
RUN wget https://dlcdn.apache.org/tinkerpop/3.6.1/apache-tinkerpop-gremlin-console-3.6.1-bin.zip
RUN unzip apache-tinkerpop-gremlin-server-3.6.1-bin.zip
RUN unzip apache-tinkerpop-gremlin-console-3.6.1-bin.zip



WORKDIR "/home/apache/workspace"
RUN git clone https://github.com/varkeyg/thirteenf.git


## Create the image
#docker build -t dev_img .
#docker run --name devenv_container --mount source=workspace,target=/home/apache/workspace \
#source=data,target=/home/apache/data -it devenv_image

#--mount source=workspace,target=/home/apache/workspace source=data,target=/home/apache/data
