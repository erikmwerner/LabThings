#ifndef __COMMANDS_H__
#define __COMMANDS_H__

//< This sets the size of the callback array in the MessageHandler class
//< The default value of 64 leaves room for up to 22 user-defined function
//< codes, but the maxiumum can be increased to 256 (214 user codes)
//< if needed and memory is not an issue (0 to 255 in an unsigned 8 bit int)
#define MAX_FUNCTIONS 64

namespace LT {
  enum FN_CODE : uint8_t {
    Read_ID               = 0x00, // (0) Read a unique identifier from the slave
    Write_Name            = 0x01, // (1) Store a name on the slave
    Read_Name             = 0x02, // (2) Read the name of the slave
    Read_Version          = 0x03, // (3) Read the version of the Firmware. Default returns LT_VERSION
    Read_Device_Count     = 0x04, // (4)
    Read_Device_Type      = 0x05, // (5)
    Read_Status           = 0x06, // (6)
    Acknowledge           = 0x07, // (7) Acknowledge receipt and execution of command
    Error                 = 0x08, // (8) Report an error
    Write_Digital_Output  = 0x09, // (9) Set the data on a digital output pin or port
    Read_Digital_Output   = 0x0A, // (10) Read the current state of a digital output pin or port
    Read_Digital_Input    = 0x0B, // (11) Read the value of a digital input pin
    Write_PWM             = 0x0C, // (12) Set the value of an analog output
    Read_PWM              = 0x0D, // (13) Read the current state of an analog output
    Read_ADC              = 0x0E, // (14) Read a value from an analog input
    Write_Sensor_Setting  = 0x0F, // (15) Write a setting to a sensor
    Read_Sensor_Setting   = 0x10, // (16) Read a sensor setting
    Read_Sensor_Value     = 0x11, // (17) Read a value from a sensor
    Write_Speed           = 0x12, // (18) Set the speed of a motor
    Read_Speed            = 0x13, // (19) Read the speed of a motor
    Write_Position        = 0x14, // (20) Set the position of a motor
    Read_Position         = 0x15, // (21) Read the position of a motor
    Write_Setting         = 0x16, // (22) Write a setting of a motor
    Read_Setting          = 0x17, // (23) Read a setting of a motor
    // (24) reserved
    // (25) reserved
    // (26) reserved
    Go_To                 = 0x1B, // (27) Go to a process step in the queue
    Enqueue               = 0x1C, // (28) Add a function to the queue
    Read_From_Queue       = 0x1D, // (29) Read a functions from the queue
    Dequeue               = 0x1E, // (30) Remove a function from the queue
    Queue_Info            = 0x1F, // (31) Read the number of spaces available in the queue
    Start_Process         = 0x20, // (32) Start a process
    Stop_Process          = 0x21, // (33) Stop a process
    Reset_Process         = 0x22, // (34) Reset a process
    Interrupt_Process     = 0x23, // (35) Pause and interrupt the current process
    Process_Info          = 0x24, // (36) Request information about a process step
    // (37) reserved
    Time_Sync             = 0x26, // (38) Send clock sync time
    Time_Followup         = 0x27, // (39) Send clock sync response time
    Delay_Request         = 0x28, // (40) Request transmission delay time
    Delay_Response        = 0x29, // (41) Respond with transmission delay time
    User_function         = 0x2A // (42) Functions 42 and above are for application-specific use
  };
}

#endif //End __COMMANDS_H__ include guard
