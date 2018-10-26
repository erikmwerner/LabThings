#include <LabThings.h>
#define N_DEVICES 1
Device_Manager<N_DEVICES> device_manager;
ASCII_Serial messenger(Serial);
Message_Handler handler;
LT_Digital_Output output(device_manager.registerDevice(), 8);
void setup() {
  messenger.setMessageReceivedCallback(onMessageReceived);
  // put your setup code here, to run once:
  handler.attachFunction(Fn_Set_Digital_Output, onSetDigitalOutput);
  handler.attachFunction(Fn_Get_Digital_Output, onGetDigitalOutput);
  device_manager.attachDevice(&output);
  Serial.begin(115200);
  Serial.println("hello world");
}

void loop() {
  // put your main code here, to run repeatedly:
  device_manager.loop();
  messenger.loop();
}
void onMessageReceived(int msg_id) {
  Serial.println();
  Serial.println(msg_id);
  handler.handleMessage(msg_id);
}

void onSetDigitalOutput() {
  int id = messenger.getNextArgInt();
  int value = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if (LT_Digital_Output *valve = d->instance()) {
      valve->setValue(value);
      Serial.print("Set output: ");
      Serial.print(value);
      Serial.println();
    }
  }
}

void onGetDigitalOutput() {
  int id = messenger.getNextArgInt();
  int value = messenger.getNextArgInt();
  if (LT_Device* d = device_manager.device(id)) {
    if (LT_Digital_Output *valve = d->instance()) {
      //Serial<<"<,"<<Fn_Get_Digital_Output<<","<<valve->UDID()<<","<<valve->getValue()<<">"<<endl;
      Serial.print("<");
      Serial.print(Fn_Get_Digital_Output);
      Serial.print(",");
      Serial.print(valve->UDID());
      Serial.print(",");
      Serial.print(valve->getValue());
      Serial.print(">");
    }
  }
}
