Lab Things Command Messenger Reference
=======================================
ASCII Serial
------------
Messages are sent from the master to the slave between start of message (SOM) and end of message (EOM) characters. Data fields are delimited by a separator (SEP) character. The default delimiting characters are < , > for SOM, SEP, and EOM, respectively. The delimiting characters are forbidden from use in data fields. Data in each data field should be represented by printable ASCII characters.

### Examples:
Write HIGH to a digital output on port 5:
`<7,5,1>`

Set motor with ID = 3 to -101.5 RPM:
`<18,3,-101.5>`

The first data field contains the command to be executed. Data in subsequent fields depends on the command implementation, and can be accessed using the functions
getNextArgInt()
getNextArgLong()
getNextArgULong()
getNextArgDouble()
getNextArgChar()

Well behaved implementations should always respond to commands either with an acknowledgement or an error. Data from the slave to the master should follow a similar delimiting format, but is implementation dependent.

Binary Serial
------------
Data packets are sent between the master and the slave using Serial Line IP (SLIP) packet framing (RFC 1055) with a 32-bit checksum. The standard SLIP frame markers are used:

Hex |	Decimal |	Type Definition	 | Desctiption
----|---------|------------------|------------
0xC0 | 192 | END | Frame end
0xDB | 219 |   ESC | Frame escape
0xDC | 220 | ESC_END | Transposed frame end
0xDD | 221 | ESC_ESC |  Transposed frame escape

Messages always begin and end with END markers.

### Examples:
Write HIGH to a digital output on port 5:
`\xC0\x07\x05\x01\crc\crc\crc\crc\xC0`

Set motor with ID = 3 to -101.5 RPM:
`\xC0\x12\x03\xC2\xCB\x00\x00\crc\crc\crc\crc\xC0`

Function codes
------------
Function implementation is hardware dependent. Some functions may not be implemented, and the implementation can vary depending on the application. The same general format should be followed. Well-behaved implementations should at least implement the general categopry of functions.

Categopry | Name | Type Definition   | Decimal  | Hex | Description
----------|------|-------------------|----------|-----|------------|
General | Read ID	| `Read_ID` | `0` | `0x00` | Read a unique identifier from the slave
General |Write Name | `Write_Name` | `1`| `0x01` | Store a name on the slave
General |Read Name | `Read_Name` | `2`| `0x02` | Read the name of the slave
General | Read Version | `Read_Version` | `3` | `0x03` | Read the version of the Firmware. Default returns `LT_VERSION`
General	| Read Device Count	| `Read_Device_Count` | `4` | `0x04`	| Read the number of devices connected to a controller
General | Read Device Type | `Read_Device_Type` | `5` |`0x05`	|
General | Read System Status | `Read_Status` | `6` | `0x06` |
General | Acknowledge | `Acknowledge` | `7` | `0x07` |
General | Error | `Error` | `8` | `0x08` |
GPIO |Write Digital Output |`Write_Digital_Output` |`9` |`0x09` |Set the data on a digital output pin or port
GPIO |Read Digital Output |`Read_Digital_Output` |`10` |`0x0A` |Read the current state of a digital output pin or port
GPIO |Read Digital Input |`Read_Digital_Input` |`11` |`0x0B` |Read the value of a digital input pin
GPIO |Write PWM |`Write_PWM` |`12` |`0x0C` |Set the value of an analog output
GPIO |Read PWM |`Read_PWM` |`13` |`0x0D` |Read the current state of an analog output
GPIO |Read ADC |`Read_ADC` |`14` |`0x0E` |Read a value from an analog input
GPIO |Write Sensor Setting |`Write_Sensor_Setting` |`15` |`0x0F` |Write a setting to a sensor. Common examples include: if polling is enabled, the polling interval, sensor resolution
GPIO |Read Sensor Setting |`Read_Sensor_Setting` |`16` |`0x10` |Read a sensor setting
GPIO |Read Sensor Value |`Read_Sensor_Value` |`17` |`0x11` |Read a value from a sensor
GPIO |Reserved | |`18` |`0x12` |
GPIO |Reserved | |`19` |`0x13` |
Motors |Write Speed |`Write_Speed` |`20` |`0x14` |Set the speed of a motor
Motors |Read Speed |`Read_Speed` |`21` |`0x15` |Read the speed of a motor
Motors |Write Position |`Write_Position` |`22` |`0x16` |Set the position of a motor
Motors |Read Position |`Read_Position` |`23` |`0x17` |Read the position of a motor
Motors |Write Setting |`Write_Setting` |`24` |`0x18` |Write a setting of a motor
Motors |Read Setting |`Read_Setting` |`25` |`0x19` |Read a setting of a motor
Motors |Reserved | |`26` |`0x1A` |
Motors |Reserved | |`27` |`0x1B` |
Protocols |Enqueue |`Enqueue` |`28` |`0x1C` |Add a function to the queue
Protocols |Read from queue |`Read_From_Queue` |`29` |`0x1D` |Read a functions from the queue
Protocols |Dequeue |`Dequeue` |`30` |`0x1E` |Remove a function from the queue
Protocols |Queue available |`Queue_Available` |`31` |`0x1F` |Read the number of spaces available in the queue
Protocols |Queue capacity |`Queue_Capacity` |`32` |`0x20` |Read the capacity of the queue
Protocols |Start Protocol |`Start_Protocol` |`33` |`0x21` |Start a protocol
Protocols |Stop Protocol |`Stop_Protocol` |`34` |`0x22` |Stop a protocol
Protocols |Reset Protocol |`Reset_Protocol` |`35` |`0x23` |Reset a protocol
Protocols |Reserved | |`36` |`0x24` |
Protocols |Reserved | |`37` |`0x25` |
Time |Time Sync |`Time_Sync` |`38` |`0x26` |Send clock sync time
Time |Time Follow-up |`Time_Followup` |`39` |`0x27` |Send clock sync response time
Time |Delay Request |`Delay_Request` |`40` |`0x28` |Request transmission delay time
Time |Delay Response |`Delay_Response` |`41` |`0x29` |Respond with transmission delay time
User |User function |`User_function` |`42` |`0x2A` |Functions 42 and above are for application-specific use