sudo groupadd docker
sudo usermod -aG docker ${USER}
sudo chown "$USER":"$USER" /home/"$USER"/.docker -R
sudo chmod g+rwx "$HOME/.docker" -R


# Build the image. Make sure to run in the directory where the Dockerfile file is available
sudo docker build -t dev_img .

# Remove the container if it already exists
sudo docker rm /dev_container

# Create a volume for data on the parent host
sudo docker volume create data

# create volume for source code on the parent host
sudo docker volume create workspace


# Run the container & Mount the volumes
sudo docker run --name dev_container \
--mount source=workspace,target=/home/apache/workspace \
--mount source=data,target=/home/apache/data -it dev_img
