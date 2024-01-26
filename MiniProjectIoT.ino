#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <TridentTD_LineNotify.h>

const char* ssid = "Xiaomi 11T Pro";
const char* password = "1212312126";
const char* mqtt_server = "192.168.203.221";

#define LINE_TOKEN  "tGAP2Sibp3C4SLf5jGbRzUYH1y6WDV5kW85brEeRWzY" // Replace with your Line Notify token

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht11(D4, DHT11);

unsigned long previousMillis = 0;
const long interval = 5000;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  dht11.begin();

  LINE.setToken(LINE_TOKEN);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    
    previousMillis = currentMillis;

    float humidity = dht11.readHumidity();
    float temperature = dht11.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    Serial.printf("Humidity: %.2f %%\tTemperature: %.2f °C\tIP: %s\n", humidity, temperature, WiFi.localIP().toString().c_str());

    StaticJsonDocument<200> doc;
    doc["humidity"] = humidity;
    doc["temperature"] = temperature;
    doc["ip"] = WiFi.localIP().toString();

    String jsonStr;
    serializeJson(doc, jsonStr);

    client.publish("dht11", jsonStr.c_str());

    String lineMessage = "Humidity: " + String(humidity) + "%\nTemperature: " + String(temperature) + "°C";

    LINE.notify(lineMessage);
  }
}

