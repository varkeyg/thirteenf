# thirteenf
Project for analysis of holdings by large money managers

#Setup
1. Download the Dockerfile
2. `docker build -t mw_image .`
3. `docker run --name mw_container -v /Users/gvarkey/workspace:/home/gvarkey/workspace -it mw_image`
4. `docker start -i mw_container`
