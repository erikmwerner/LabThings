#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#define MAX_FUNCTIONS 64 // TK why?

// Function enumeration
  /*enum LTF : uint8_t {
    // System functions for reading and writing controller and device information
    Fn_Set_Controller_Name      = 0, // Set Controller Name
    Fn_Get_Controller_Name      = 1, // Get Controller Name
    Fn_Get_Controller_FW        = 2, // Get Controller Firmware Version
    Fn_Get_Device_Count         = 3, // Get Attached Device Count
    //Fn_Get_Device_Type          = 4, // Get Device Type TK add type support
    Fn_Set_Device_Name          = 4, // Set Device Name
    Fn_Get_Device_Name          = 5, // Get Device Name
    Fn_Get_System_Status        = 6, // Get System Status

    // Support for simple analog and digital sensor inputs
    // Digital sensors have a resolution of 1-bit
    // Suports 12-bit oversampling of the ADC
    // Sensor examples include: buttons, encoders, potentiometers, temperature...
    Fn_Set_Polling_Interval     = 7, // Set Sensor Polling Interval
    Fn_Get_Polling_Interval     = 8, // Get Sensor Polling Interval
    Fn_Set_Sensor_Resolution    = 9, // Set Sensor Resolution
    Fn_Get_Sensor_Resolution    = 10, // Get Sensor Resolution
    Fn_Get_Sensor_Value         = 11, // Get Sensor Value

    // Support for simple digital outputs
    // Can write Boolean values to a pin or bytes to a port
    Fn_Set_Digital_Output       = 12, // Set Digital Output
    Fn_Get_Digital_Output       = 13, // Get Digital Output
    Fn_Set_Port_Byte            = 14, // Set Port Value
    Fn_Get_Port_Byte            = 15, // Get Port Value

    // Support for simple PWM outputs
    Fn_Set_PWM                  = 16,	// Set PWM Value
    Fn_Get_PWM                  = 17,	// Get PWM Value

    // Support for stepper motors
    Fn_Set_Stepper_RPM           = 18,	// Set RPM
    Fn_Get_Stepper_RPM           = 19,	// Get RPM
    Fn_Set_Stepper_Position      = 20,	// Set Position
    Fn_Get_Stepper_Position      = 21,	// Get Position
    Fn_Set_Stepper_Resolution    = 22,	// Set Resolution
    Fn_Get_Stepper_Resolution    = 23,	// Get Resolution
    Fn_Set_Stepper_Enabled       = 24,	// Set Enabled
    Fn_Get_Stepper_Enabled       = 25,	// Get Enabled

    // Support for DC motors
    Fn_Set_Speed                = 26,	// Set Speed
    Fn_Get_Speed                = 27,	// Get Speed

    // Support for protocol buffering
    Fn_Queue_Function           = 28,	// Queue Function
    Fn_Get_Queued_Function      = 29,	// Get Function
    Fn_Get_Queue_Count          = 30,	// Get Queue Count
    Fn_Get_Queue_Available      = 31,	// Get Queue Availability

    // Support for starting, stopping, and resetting all devices
    Fn_Start                   = 32,	// Start Protocol
    Fn_Stop                    = 33,	// Stop Protocol
    Fn_Reset                   = 34,	// Reset Protocol

    // Slots 35 through MAX_FUNCTIONS reserved for custom user commands
    // MAX_FUNCTIONS can be reduced to save on memory if necessary
    Fn_Reserved
  };*/
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
    // (18) reserved
    // (19) reserved
    Write_Speed           = 0x14, // (20) Set the speed of a motor
    Read_Speed            = 0x15, // (21) Read the speed of a motor
    Write_Position        = 0x16, // (22) Set the position of a motor
    Read_Position         = 0x17, // (23) Read the position of a motor
    Write_Setting         = 0x18, // (24) Write a setting of a motor
    Read_Setting          = 0x19, // (25) Read a setting of a motor
    // (26) reserved
    // (27) reserved
    Enqueue               = 0x1C, // (28) Add a function to the queue
    Read_From_Queue       = 0x1D, // (29) Read a functions from the queue
    Dequeue               = 0x1E, // (30) Remove a function from the queue
    Queue_Available       = 0x1F, // (31) Read the number of spaces available in the queue
    Queue_Capacity        = 0x20, // (32) Read the capacity of the queue
    Start_Protocol        = 0x21, // (33) Start a protocol
    Stop_Protocol         = 0x22, // (34) Stop a protocol
    Reset_Protocol        = 0x23, // (35) Reset a protocol
    // (36) reserved
    // (37) reserved
    Time_Sync             = 0x26, // (38) Send clock sync time
    Time_Followup         = 0x27, // (39) Send clock sync response time
    Delay_Request         = 0x28, // (40) Request transmission delay time
    Delay_Response        = 0x29, // (41) Respond with transmission delay time
    User_function         = 0x2A // (42) Functions 42 and above are for application-specific use
  };
}

#endif //End __COMMANDS_H__ include guard
