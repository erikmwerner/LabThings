#include <LabThings.h>
Device_Manager<5> device_manager;

LT_Encoder p1encoder(device_manager.registerDevice(), 5, 6, true);
LT_DebouncedButton p1button(device_manager.registerDevice(), 7, true);

LT_Encoder p2encoder(device_manager.registerDevice(), 2, 3, true);
LT_DebouncedButton p2button(device_manager.registerDevice(), 4, true);

// 1306 OLED on Hardware i2c with 2-writes per refresh
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);
UiContext context(&u8g2);
Ui menu(device_manager.registerDevice(), context);
MenuScreen screen_menu(NULL, &context, "Menu");//, 0, 0, 128, 64);
MenuScreen screen_game(&screen_menu, &context, "Game");//, 0, 0, 128, 64);

const uint8_t SCREEN_H = 64;
const uint8_t PADDLE_W = 24;
TextItem<8> text_status(&screen_menu, context.fontLarge(), "Pong", AlignHCenter | AlignVCenter);
LineItem paddlePlayer2(&screen_menu, 120, 0, 120, PADDLE_W);
LineItem paddlePlayer1(&screen_menu, 8, 0, 8, PADDLE_W);



void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

ISR (PCINT2_vect) // handle pin change interrupt for D0 to D7
{    
     p1encoder.handleInterrupt();
     p1button.handleInterrupt();
     p2encoder.handleInterrupt();
     p2button.handleInterrupt();
}
 
void setup() {
  pciSetup(5);
  pciSetup(6);
  pciSetup(7);
  
  // put your setup code here, to run once:
  //screen_menu.addMenu(&screen_menu);
  screen_menu.addChild(&text_status);
  screen_menu.addChild(&paddlePlayer1);
  screen_menu.addChild(&paddlePlayer2);
  //text_status.setPos(12,24);
  menu.setCurrentScreen(&screen_menu);

  device_manager.attachDevice(&p1encoder);
  device_manager.attachDevice(&p1button);
  device_manager.attachDevice(&p2encoder);
  device_manager.attachDevice(&p2button);
  device_manager.attachDevice(&menu);

  p1encoder.setValueChangedCallback(onPlayer1Move);
  p1button.setButtonPressedCallback(onPlayer1Button);
  p2encoder.setValueChangedCallback(onPlayer2Move);
  p2button.setButtonPressedCallback(onPlayer2Button);

  p1encoder.setDebounceInterval(5000);//5ms

  //Serial.begin(115200);
  //Serial.println("Hello world. This is lab pong.");
}

void loop() {
  // put your main code here, to run repeatedly:
  device_manager.loop();
}

void onPlayer1Move() {
  static int8_t last_value_1 = p1encoder.position();
  int8_t delta = p1encoder.position() - last_value_1;
  movePaddle(&paddlePlayer1, delta);
  last_value_1 = p1encoder.position();
}

void onPlayer2Move() {
  static int8_t last_value_2 = p2encoder.position();
  int8_t delta = p2encoder.position() - last_value_2;
  movePaddle(&paddlePlayer2, delta);
  last_value_2 = p2encoder.position();
}


void onPlayer1Button() {
  //Serial.println("p1b");
}

void movePaddle(LineItem* paddle, int8_t dy) {
  int8_t last_pos = paddle->top();
  if ( dy < 0 ) {
    if ( last_pos <= 1 ) {
      
    }
    else {
      paddle->move( 0, dy );
    }
  }
  else {
    if ( last_pos >= ( 63 - PADDLE_W) ) {
      
    }
    else {
      paddle->move( 0, dy );
    }
  }
}

void onPlayer2Button() {
  //Serial.println("p2b");
}
