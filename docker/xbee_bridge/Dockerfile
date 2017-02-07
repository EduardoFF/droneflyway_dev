FROM osrf/ubuntu_armhf:trusty

MAINTAINER Eduardo Feo eduardo@idsia.ch

USER root
ENV HOME /home/root

RUN apt-get update && apt-get install -y \
    build-essential \
    git
    
RUN rm -rf /var/lib/apt/lists/*


RUN set -x \
&& git clone https://github.com/attie/libxbee3.git /tmp/libxbee3 \
&& ( cd /tmp/libxbee3  && git checkout ) \
&& cd /tmp/libxbee3 && make configure && make install \
&& rm -rf /tmp/libxbee

RUN set -x \
&& https://github.com/EduardoFF/droneflyway_dev.git /home/root \
&& ( cd /home/root/droneflyway_dev  && git checkout 13aa0264  ) \
&& cd /home/root/droneflyway_dev/xbee_bridge && make &&  make install 


