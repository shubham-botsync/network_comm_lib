\***\*\*\*\*\*\*** MQTTT **\*\***\*\*\***\*\***

- Download tar file (and not git repo) from the official link

- Mosquitto installation -

https://github.com/jpmens/mosquitto-auth-plug/issues/386

- Compiling a source file -

g++ -o foo main.cpp -lmosquitto (use -I option to include 'mosquitto_dir/include' if not working)

- Access logs at /var/log/mosquitto/mosquitto.log

* Reference

https://github.com/darcyg/presence-sensor-bluetooth/blob/master/sensor_common/mosquittohandler.cpp
https://mosquitto.org/api/files/mosquitto-h.html
