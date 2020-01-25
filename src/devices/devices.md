# Lab Things Devices
The Lab Things device classes provide base functions for most common input and output operations. Device-specific implementations can be defined by subclassing on the device classes or callback functions for the device.

## Device Manager
The DeviceManager class implements a simple cooperative multitasking scheduler that is included to make performing multiple tasks on the microprocessor as simple as possible. The class is responsible for two functions: assigning a unique ID to each device used by the program and sharing processor time between multiple devices.

The unique ID is an 8-bit unsigned integer that is stored as a constant class member of all devices that subclass LT_Device. This allows messaging or other serialization classes, such as the ASCIISerial or BinarySerial messaging classes, to easily identify and access a device. It is also used to index a pointer to the device in an array of device pointers that are managed by the DeviceManager instance.

The device manager shares processor time between devices using a simple cooperative multitasking scheduler. Once devices have a unique ID and have been registered with the DeviceManager, the update() method of each device will be called every time the update() method of the DeviceManager is called. Devices are updated sequentially in the opposite order of their unique-id.

Due to the cooperative multitasking scheme, it is important that devices do not rely heavily on the delay() function or consume too much processor time during one update cycle. If this should occur, subsequent devices will not be updated until the offending device returns from its update function, yielding processor time back to the scheduler.
