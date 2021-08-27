//RGB LED-Stripe per UDP Telegrammen steuern

//Bibiotheken einbinden
#include <WiFi.h>
#include <PubSubClient.h>



#define red 4
#define green 21
#define blue 23
#define led 2

//Variablen deklarieren
const int freq = 5000;
const int resolution=8;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;
unsigned int redbrightness = 0;
unsigned int greenbrightness = 0;
unsigned int bluebrightness = 0;
boolean state =false;

//WLAN-Daten
const char* ssid = "Bauer - Home";
const char* password = "Florentine2019!";
uint8_t mqtt_server[]={192,168,1,234};

WiFiClient espClient;
PubSubClient client(espClient);
long lstmsg=0;
char msg[50];


void wifi_connect(){
  digitalWrite(led, LOW);
  delay(50);
  Serial.print("Verbinden mit ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\n verbunden. Aktuelle IP: ");
  Serial.print(WiFi.localIP());
  digitalWrite(led, HIGH);
}

void ledtest(){
  Serial.println("LED-Test gestartet.");
  Serial.println("rot");
  ledcWrite(redChannel, 255);
  delay(1000);
  Serial.println("grün");
  ledcWrite(redChannel, 0);
  ledcWrite(greenChannel, 255);
  delay(1000);
  Serial.println("blue");
  ledcWrite(greenChannel, 0);
  ledcWrite(blueChannel, 255);
  delay(1000);
  Serial.println("alle LED´s");
  ledcWrite(redChannel, 255);
  ledcWrite(greenChannel, 255);
  ledcWrite(blueChannel, 255);
  delay(1000);
  ledcWrite(redChannel, redbrightness);
  ledcWrite(greenChannel, greenbrightness);
  ledcWrite(blueChannel, bluebrightness);
  Serial.println("LED-Test beendet.");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start");

  //LED-PWM-Kanäle festlegen
  ledcSetup(redChannel, freq, resolution);
  ledcSetup(greenChannel, freq, resolution);
  ledcSetup(blueChannel, freq, resolution);
   
  //LED-PWM-Kanäle Pins zuweisen
  ledcAttachPin(red, redChannel);
  ledcAttachPin(green, greenChannel);
  ledcAttachPin(blue, blueChannel);

  //interne LED
  
  digitalWrite(led, LOW);
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  digitalWrite(led, HIGH);
  delay(150);
  digitalWrite(led, LOW);
  delay(150);
  
  wifi_connect();
  ledtest();
  
  client.setServer(mqtt_server, 1883);
  Serial.println("server gesetzt");
  client.setCallback(callback);
  Serial.println("callback  gesetzt");
}


void reconnect() {
  // Schleife zum wieder verbinden
  while (!client.connected()) {
    Serial.print("Wiederaufbau der Verbindung...");
    
    if (client.connect("Sideboard")) {
      Serial.println("verbunden");
      // Subscribe
      client.subscribe("wohnen/sideboard/output");
      client.subscribe("wohnen/sideboard/r");
      client.subscribe("wohnen/sideboard/g");
      client.subscribe("wohnen/sideboard/b");
      Serial.println("Topics subscribed");
    } else {
      Serial.print("Fehlgeschlagen, rc=");
      Serial.print(client.state());
      Serial.println(" neuer Versuch in 5 Sekunden");
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (state){
    ledcWrite(redChannel, redbrightness);
    ledcWrite(greenChannel, greenbrightness);
    ledcWrite(blueChannel, bluebrightness);
  }
  else
  {
    ledcWrite(redChannel, 0);
    ledcWrite(greenChannel, 0);
    ledcWrite(blueChannel, 0);
  }


  long now = millis();
 // lstmsg=now+10000;
  if (now - lstmsg > 5000) {
    lstmsg = now;

    char redString[50];
    itoa(redbrightness,redString,10);
    char greenString[50];
    itoa(greenbrightness,greenString,10);
    char blueString[50];
    itoa(bluebrightness,blueString,10);

//    String redwert = String((char*)redString);
//    String greenwert = String((char*)greenString);
//    String bluewert = String((char*)blueString);
    client.publish("wohnen/sideboard/state/r", redString);
    
    client.publish("wohnen/sideboard/state/g", greenString);
    
    client.publish("wohnen/sideboard/state/b", blueString);
    if (state){
      client.publish("wohnen/sideboard/state/state", "on");
    }
    else{
      client.publish("wohnen/sideboard/state/state", "off");
    }
    

  }
}


void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("NAchricht eingeangen von topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  String TOPIC =String(topic);
 // String PAYLOAD =String((char*)messageTemp);
  int wert;
    if (TOPIC== "wohnen/sideboard/r"){
      wert = messageTemp.toInt();
      redbrightness=wert;
      Serial.print("rot gesetzt auf: ");
      Serial.println(redbrightness);
    }
  else if (TOPIC== "wohnen/sideboard/g"){
      wert = messageTemp.toInt();
      greenbrightness=wert;
      Serial.print("grün gesetzt auf: ");
      Serial.println(greenbrightness);
    }
  else if (TOPIC== "wohnen/sideboard/b"){
      wert = messageTemp.toInt();
      bluebrightness=wert;
      Serial.print("blau gesetzt auf: ");
      Serial.println(bluebrightness);
    }
  else if (TOPIC== "wohnen/sideboard/output"){
      if (messageTemp  == "on"){
        state=true;
        Serial.println("LED angeshcxaltet");
      }
      else{
        state=false;
        Serial.println("LED ausgeschaltet");
      }
    }


  

}
