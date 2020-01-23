/************************ Adafruit IO Config *******************************/
/*
 * This file based off the Adafruit.io config file
 * 
 */
// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME  "username"
#define IO_KEY       "iokey"

/******************************* WIFI **************************************/

// the AdafruitIO_WiFi client will work with the following boards:
//   - HUZZAH ESP8266 Breakout -> https://www.adafruit.com/products/2471
//   - Feather HUZZAH ESP8266 -> https://www.adafruit.com/products/2821
//   - Feather HUZZAH ESP32 -> https://www.adafruit.com/product/3405
//   - Feather M0 WiFi -> https://www.adafruit.com/products/3010
//   - Feather WICED -> https://www.adafruit.com/products/3056

#define WIFI_SSID       "SSID"
#define WIFI_PASS       NULL

typedef void(*ipCallback) (String, String);

#include "AdafruitIO_WiFi.h"


/**************** Wrapper for Adaruit IO Library ****************/
class LT_MQTT : public AdafruitIO_WiFi, public LT_Device {

    ipCallback m_cxnStatusCallback = NULL;
    bool m_connected = false;
    String mac;
    uint16_t connectionAttempts = 0;
    uint32_t t_last_connection_attempt;

    void tryConnect() {
      if (AdafruitIO_WiFi::status() < AIO_CONNECTED) {
        if ( (micros() - t_last_connection_attempt) > 1000000) {
          ++connectionAttempts;
          connectionStatusChanged(false);
          t_last_connection_attempt = micros();
          Serial.print("connecting to IO...");
          Serial.println(connectionAttempts);
        }
      }
      else {
        Serial.print(" WiFi connected. IP address: ");
        Serial.println(WiFi.localIP());
        connectionStatusChanged(true);
      }
    }

    void connectionStatusChanged(bool isConnected) {
      m_connected = isConnected;
      if (m_cxnStatusCallback != NULL) {
        if (isConnected) {
          
          char ip_buf[24];
          IPAddress addr = WiFi.localIP();
          snprintf(ip_buf, 24, "IP:%03u.%03u.%03u.%03u", addr[0], addr[1], addr[2], addr[3]);
          String ip_string(ip_buf);
          (*m_cxnStatusCallback)(ip_string, mac);
        }
        else {
          String ip_string = "No IP ";
          ip_string.concat(String(connectionAttempts));
          (*m_cxnStatusCallback)(ip_string, mac);
        }
      }
    }

  public:
    LT_MQTT(const int id) : AdafruitIO_WiFi(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS), LT_Device(id) {}

    LT::DeviceType type() {
      return (LT::DeviceType)(LT::UserType + 2);
    }
    virtual void* instance() {
      return this;
    }
    
    void setConnectionStatusChangedCallback(ipCallback c) {
      m_cxnStatusCallback = c;
    }
    
    void begin() {
      AdafruitIO_WiFi::connect();
    }
    
    void update(void) {
      if (m_connected == false) {//AdafruitIO_WiFi::status() < AIO_CONNECTED) {
        tryConnect();
      }
      else {
        //uint32_t t_start = micros();
        AdafruitIO_WiFi::run();
        //uint32_t dt = micros() - t_start;
        //Serial<<"running io took: "<<dt<<" ms";
      }
    }
    
};
