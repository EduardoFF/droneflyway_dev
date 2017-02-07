alias xbeebridge="docker run \
	-d --name=xbee_bridge \
	--net=host --restart=always \
	--privileged dfw:xbee_bridge \
	xbee_bridge --ip 127.0.0.1"
