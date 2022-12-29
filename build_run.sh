# Build the image. Make sure to run in the directory where the Dockerfile file is available
docker build -t dev_img .

# Remove the container if it already exists
docker rm /dev_container

# Create a volume for data on the parent host
docker volume create data

# create volume for source code on the parent host
docker volume create workspace


# Run the container & Mount the volumes
docker run --name dev_container \
--mount source=workspace,target=/home/apache/workspace \
--mount source=data,target=/home/apache/data -it dev_img
