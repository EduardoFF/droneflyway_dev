FROM ros:jade-ros-base

MAINTAINER Eduardo Feo eduardo@idsia.ch

RUN apt-get update && apt-get install -y \
    ros-jade-mavros
    
USER root
ENV HOME /home/root


ENTRYPOINT ["/ros_entrypoint.sh"]

