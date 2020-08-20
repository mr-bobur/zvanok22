#include <TimeLib.h>
#include <EEPROM.h>
#include <DS1307RTC.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
unsigned long time1, time2;
tmElements_t tm;
int sshh = 1;
int aass = 0;
int sshhq = 1;
int aassw = 0;
int sshhss = 0;
int  chv      = 0;
int  tv       = 0;
int  chsoni   = 0;
int status = 0;
int mode1     = 0;
int passlen   = 0;
char temp1, temp2;
String password;
IPAddress accessPointIP     = IPAddress(192, 168, 1, 1);
int keys[24][2];
bool bayram = true;
int days[24][2] = {
  {1, 1}, {1, 2}, {1, 14}, {3, 8},
  {3, 21}, {5, 9}, {6, 1}, {9, 1},
  {10, 1}, {12, 8}, {12, 29}, {12, 30},
  {12, 31}
};
WiFiServer server(80);
WiFiClient client;

void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  for (int i = 0; i <= 23; i++) {
    keys[i][0] = EEPROM.read(100 + i);
    keys[i][1] = EEPROM.read(150 + i);
  }
  chv = EEPROM.read(51 );
  tv = EEPROM.read(52 );
  chsoni = EEPROM.read(53 );
  status = EEPROM.read(50);
  passlen = EEPROM.read(250);
  for (int i = 0; i < passlen; i++) {
    temp1 = EEPROM.read(251 + i);
    password += temp1;
  }
  Serial.println( );
  Serial.println( password );
 
  pinMode(D7, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(D7, HIGH);
  WiFi.disconnect();
  WiFi.softAPdisconnect(false);
  WiFi.mode(WIFI_STA);
  WiFi.softAPConfig(accessPointIP, accessPointIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("B_QUANTUM AQ-100", password);
  server.begin();
}
void loop() {
  if (time1 + 1000 < millis()) {
    time1 = millis();
    if (RTC.read(tm)) {
      for (int i = 0; i <= 12; i++) {
      if ( tm.Day == days[i][1] && tm.Month == days[i][0]) {
          bayram = false;
      }
      }
      for (int i = 0; i <= 23; i++) {
        if ( status == 2 && tm.Hour == keys[i][0] && tm.Minute == keys[i][1] && tm.Wday != 7  && bayram) {
          sshhss = tm.Second % (chv + tv);
          if (aass == tv) {
            sshh++;
            aass = 0;
          }
          if (sshhss <  chv && sshh <= chsoni ) {
            digitalWrite(D7, LOW);
          }
          if (sshhss >=  chv  && sshhss < tv + chv && sshh <= chsoni ) {
            digitalWrite(D7, HIGH);
            aass++;
          }
        }
        if ( tm.Second == 59) {
          sshh = 1;
        }
      }
      bayram = true;
    }
  }

  if (time2 + 80 < millis()) {
    time2 = millis();
    if (digitalRead(LED_BUILTIN) && status == 2) {
      digitalWrite(LED_BUILTIN, LOW);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
    }
    client = server.available();
    if (!client) {
      return ;
    }
    String request, header1;
    header1 = client.readStringUntil('\n') + "\r\n";
    header1 = client.readStringUntil('\n') + "\r\n";
    request = client.readStringUntil('\n') + "\r\n";
    request = request.substring(7);
    DynamicJsonDocument currentjsonDocument(1024);
    DeserializationError jsonError = deserializeJson(currentjsonDocument, request);
    if (!jsonError) {
      serializeJson(currentjsonDocument, request);
      JsonObject joset = currentjsonDocument.as<JsonObject>();
      joset["mode"]     = mode1         = joset["mode"]     | mode1;
      
      client.flush();
      String s = "HTTP/1.1 200 OK\r\n";
      s += "Content-Type: application/json\r\n\r\n";
      s += "{\"hour\":\"" + String(tm.Hour)  + "\",\"minute\":\"" + String(tm.Minute) + "\"}\r\n";
      s += "\n";
      client.print(s);
      if (mode1 == 1) {
        tm.Second =  joset["second"] ;
        tm.Minute =  joset["minute"] ;
        tm.Hour =  joset["hour"] ;
        tm.Wday =  joset["wday"] ;
        tm.Day =  joset["day"] ;
        tm.Month =  joset["month"] ;
        int asdsa = joset["year"];
        tm.Year =  CalendarYrToTm(asdsa);
        DS1307RTC::write(tm);
      }
      if (mode1 == 3) {
        JsonArray hourss = joset["sethour"];
        JsonArray minss = joset["setmin"];
        int aa, dd;
        for (int i = 0; i <= 23; i++) {
          keys[i][0] =  hourss[i];
          keys[i][1] = minss[i];
          EEPROM.write(100 + i,  hourss[i]);
          EEPROM.write(150 + i,  minss[i] );
        }
        EEPROM.commit();
      }
      if (mode1 == 4) {
        chv = joset[ "chvaqt"];
        tv = joset["tvaqt"];
        chsoni = joset["chson"];
        EEPROM.write(51,  chv);
        EEPROM.write(52,  tv );
        EEPROM.write(53,  chsoni );

        joset["password"] = password      = joset["password"] | password;
        passlen =  password.length();
        EEPROM.write(250,  passlen );
        for (int i = 0; i < passlen; i++) {
          temp1 = password[i];
          EEPROM.write(251 + i, temp1);
        }
        EEPROM.commit();
       }
      if (mode1 == 5) {
        status = joset["status"];
        EEPROM.write(50,  status );
        EEPROM.commit();
      }
    }
  }
}
