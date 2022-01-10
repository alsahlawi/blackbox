#include <TinyGPS++.h> // library for GPS module
#include <SoftwareSerial.h> // library replacing defult RX,TX
#include <PubSubClient.h> // library for MQTT comunication
#include <ESP8266WiFi.h> // library for connecting to the Wifi

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(D2, D1); // The serial connection to the GPS device, connected at D1,D2 digital pins

const char* wifi_ssid = "SL"; //ssid of wifi
const char* wifi_password = "12121212"; //password of wifi

float latitude , longitude, vib,speed; // geolocation variables
int year , month , date, hour , minute , second;  // geolocation variables
String date_str , time_str , lat_str , lng_str, vib_str,speed_str;  // geolocation variables
int pm; 
int vs =D5; //vibration sensor connected at digital pin D5
boolean connectedwifi = false; //flag to blink the led

// MQTT connection details
#define mqtt_server "20.124.199.242"
#define mqtt_port 1883
#define mqtt_user "ahmed"
#define mqtt_password "password"
#define out_topic "vibration"         // vibration topic
#define location_long_topic "lon"     // Longitude topic
#define location_lat_topic "lat"      // Latitude topic
#define device_id "id"                // Blackbox Device ID
#define location_speed_topic "speed"  // Speed topic

// Defining wifi and MQTT clients (Objects)
WiFiClient espClient;
PubSubClient client;

void setup()
{
  // set digital pins for input and output
  pinMode(vs, INPUT);
  pinMode(D8, OUTPUT);
  // set serial monitor to see the results in the arduino serial monitor
  Serial.begin(115200);
  ss.begin(9600);
  // setup wifi function
  setup_wifi();
  // assign communication objects to MQTT clients
  client.setClient(espClient);
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

}

// will run infinitly 
void loop()
{
 
  if (!client.connected()) {
    // attempt to reconnect function
    reconnect();
  }else{
      // Blinking if the device is already connected
      digitalWrite(D8, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(100);               // wait for 0.1 second
      digitalWrite(D8, LOW);    // turn the LED off by making the voltage LOW
      delay(100);               // wait for 0.1 second
  }
  // MQTT will continue to push messages as long as it's connected
  client.loop();

 long measurement =vibration(); // Reading digital data from the D5 Pin to get vibration.

  // this is to check if the gps sensor is sending data.
  while (ss.available() > 0) //while data is available
    if (gps.encode(ss.read())) //read gps data
     { 
        displayInfo(); // display gps info in serial monitor (for debugging)
      if (gps.location.isValid()) //check whether gps location is valid
      {
        
        latitude = gps.location.lat(); // latitude location is stored
        lat_str = String(latitude , 6); // latitude location is stored in a string
        
        longitude = gps.location.lng();  //longitude location is stored
        lng_str = String(longitude , 6); //longitude location is stored in a string
      
        speed = gps.speed.kmph(); //speed of vehicle is stored
        speed_str = String(speed); //speed of vehicle is stored in a string
        
        // Client objectes used to push the stored data, followed by 1 second delay to ensure full data sent.
        Serial.println(measurement);
        client.publish(out_topic, String(measurement).c_str(), true); 
        client.publish(location_long_topic, lat_str.c_str(), true);
        client.publish(location_lat_topic, lng_str.c_str(), true);
        client.publish(device_id,"ESP8266Blackbox", true);
        client.publish(location_speed_topic, speed_str.c_str(), true);
        delay(1000);
      }

        // this will still push vibration even if the GPS is lost.
        Serial.println(measurement);
        client.publish(out_topic, String(measurement).c_str(), true);
   
    
    }
 


  
}

// Setup wifi function
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  // Setup wifi credintials
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
// Reconnect function will ensure the MQTT client is always connected, will try to reconnect if disconnect.
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

// this function is for listening if Node-red pushes back a signal (for future work, for working with output sensors).
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

//Mesuring vibration from the sensor if signals detected.
long vibration(){
  long measurement=pulseIn (vs, HIGH);//wait for the pin to get HIGH and returns measurement
  return measurement;
}

// this function is just for testing GPS connectivity by printing GPS info in Serial Monitor.
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
