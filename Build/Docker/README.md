# Dockerfiles for Supported Platforms

This folder contains Dockerfiles for building images that can be used
for building TeamTalk binaries. The generated Docker images will
contains the necesssary tools for building TeamTalk binaries.

Use `docker-compose` to build the Docker images, e.g.:

`docker-compose build ubuntu22`

Afterwards start a container using the image like this:

`docker-compose run --rm ubuntu22`

The TeamTalk5 repository is now available in the container's
`/TeamTalk5` folder.

To build TeamTalk run the following commands:

`cd /TeamTalk5`
`source env.sh`
`make -C Build ubuntu22`
