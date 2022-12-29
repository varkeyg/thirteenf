docker build -t dev_img .
docker rm /dev_container
docker run --name dev_container \
--mount source=workspace,target=/home/gvarkey/workspace \
--mount source=data,target=/home/gvarkey/data -it dev_img
