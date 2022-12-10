# thirteenf
Project for analysis of holdings by large money managers

#Setup
1. Download the Dockerfile
2. `docker build -t tf_image .`
3. `docker run --name tf_container -v /Users/gvarkey/workspace:/home/gvarkey/workspace -it tf_image`
4. checkout the package to /Users/gvarkey/workspace from host computer (this can avoid copying ssh key to container)

