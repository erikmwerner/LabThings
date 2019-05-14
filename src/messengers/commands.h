#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#define MAX_FUNCTIONS 64 // TK why?
// Function enumeration
enum : uint8_t {
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
};

#endif //End __COMMANDS_H__ include guard
