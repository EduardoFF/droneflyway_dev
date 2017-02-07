 alias mavros-xbeebridge="docker run -d \
	--name=mavros-xbeebridge \
	--net=host \
	--restart=always \
	--privileged \
	-e ROS_MASTER_URI=http://192.168.201.4:11311 \
	-e ROS_IP=192.168.201.4 \
	-e ROS_HOSTNAME=192.168.201.4 \
	-e ROS_NAMESPACE=/`hostname`/r1 \
	dfw:mavros-xbeebridge \
	rosrun mavros mavros_node _fcu_url:=tcp://127.0.0.1:12345"
