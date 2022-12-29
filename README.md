# thirteenf
Project for analysis of holdings by large money managers.

### Project Setup
- You can install all the dependeices defined in `CMakeLists.txt` to setup the project.
- Alternatively us docker to automatically setup the environment. 

### Dockersetup
  - Make sure docker is installed and running. 
  
  ```shell
 xps√ ~ % docker --version
Docker version 20.10.22, build 3a2c30b
  ```
- Download the Dockerfile and a shell script to build an image
```
wget https://github.com/varkeyg/thirteenf/blob/main/Dockerfile
wget https://github.com/varkeyg/thirteenf/blob/main/build_run.sh
```
-  Run the `build_run.sh`

Now you can access the container via command line or use vscode to access the container. Install the plugin's 

