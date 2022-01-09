#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(D2, D1); // The serial connection to the GPS device

const char* wifi_ssid = "SL"; //ssid of your wifi
const char* wifi_password = "12121212"; //password of your wifi

float latitude , longitude, vib,speed;
int year , month , date, hour , minute , second;
String date_str , time_str , lat_str , lng_str, vib_str,speed_str;
int pm;
int vs =D5; //vibration sensor
boolean connectedwifi = false;

#define mqtt_server "20.124.199.242"
#define mqtt_port 1883
#define mqtt_user "ahmed"
#define mqtt_password "password"
#define in_topic "vibration/in"
#define out_topic "vibration"
#define location_long_topic "lon"
#define location_lat_topic "lat"
#define device_id "id"
#define location_speed_topic "speed"

WiFiClient espClient;
PubSubClient client;
WiFiServer server(80);

void setup()
{
  pinMode(vs, INPUT);
  pinMode(D8, OUTPUT);
  Serial.begin(115200);
  ss.begin(9600);
  setup_wifi();
  client.setClient(espClient);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}


void loop()
{
 
  if (!client.connected()) {
    reconnect();
  }else{
      digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);                       // wait for a second
      digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
      delay(100);                       // wait for a second

  }
  client.loop();

 long measurement =vibration(); // Reading digital data from the D5 Pin of the NodeMCU ESP8266.

  while (ss.available() > 0) //while data is available
    if (gps.encode(ss.read())) //read gps data
     {  Serial.println("inside cond 1");
        displayInfo();
      if (gps.location.isValid()) //check whether gps location is valid
      {Serial.println("inside cond 2");
        latitude = gps.location.lat();
        lat_str = String(latitude , 6); // latitude location is stored in a string
        
        longitude = gps.location.lng();
        lng_str = String(longitude , 6); //longitude location is stored in a string
      
        speed = gps.speed.kmph();
        speed_str = String(speed); //longitude location is stored in a string
        
        Serial.println(measurement);
        client.publish(out_topic, String(measurement).c_str(), true);
        //delay(500);
        client.publish(location_long_topic, lat_str.c_str(), true);
        //delay(1000);
        client.publish(location_lat_topic, lng_str.c_str(), true);
        //delay(1000);
         client.publish(device_id,"ESP8266Blackbox", true);
        //delay(1000);
        client.publish(location_speed_topic, speed_str.c_str(), true);
        delay(1000);
      }

        
         // Publishes 
        Serial.println(measurement);
        client.publish(out_topic, String(measurement).c_str(), true);
   
    
    }
 


  
}


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);                       // wait for a second
     digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);                       // wait for a second
    digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);  
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect("ESP8266Blackbox", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
   Serial.print("Message arrived [");
   Serial.print(topic);
   Serial.print("] ");
 for (int i = 0; i < length; i++) {
  char receivedChar = (char)payload[i];
  Serial.print(receivedChar);
 }
 Serial.println();
}
long vibration(){
  long measurement=pulseIn (vs, HIGH);//wait for the pin to get HIGH and returns measurement
  return measurement;
}

void displayInfo()
{
   Serial.println(gps.satellites.value());
     Serial.println(gps.location.lat());
  Serial.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
