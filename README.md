# water-level-controller-monitor

This project was created in PlatformIO with freeRTOS for two different ESP8266. One is water level monitor and the other is water level controller. Multiple monitors can be deployed with single controller to monitor and control the level of water in a tank. This systesm works when all the devices are connected to same AP.

Once the hostname for all monitors and controller are set, the monitors dynamically discovers their controller's IP address. The montiors broadcasts a solicaition message over UDP. Once the controller receives this messages it advertises itself to the monitor. Thus, both discovering each other's IP address.

The controller saves the states of all the water tank as reported by the monitors and turns on/off the water pump as required.
