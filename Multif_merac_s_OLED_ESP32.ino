#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>

// Wi-Fi pripojenie
const char* ssid = "huawei-wifi";
const char* password = "0903666231";

// Konfigurácia DHT11
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Konfigurácia OLED displeja
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Pin zabudovanej LED
#define BUILTIN_LED 2

// NTP klient na získanie dátumu a času
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // UTC+1

// Premenné na uloženie posledných nameraných hodnôt
float lastTemp = 0.0;
float lastHum = 0.0;

void setup() {
  Serial.begin(115200);

  // Inicializácia DHT senzora
  dht.begin();

  // Inicializácia OLED displeja
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED inicializácia zlyhala!"));
    for (;;);
  }
  display.clearDisplay();

  // Inicializácia zabudovanej LED
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

// Zobraz uvítaciu správu
  display.setTextSize(2);  // Väčší text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Merac");
  display.setTextSize(1);  // Menší text
  display.setCursor(0, 20);
  display.println("teploty s OLED, v1.00");
  display.display();
  display.setCursor(0, 40);
  display.println("Mgr. Nemet Peter,2RSI");
  display.display();
  display.setCursor(0, 50);
  display.println("Internet veci, 2025");
  display.display();
  delay(4000);

  // Pripojenie k Wi-Fi
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pripajam sa ku WiFi...");
  display.display();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Pripajam sa k Wi-Fi...");
  }

  // Po úspešnom pripojení
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pripojene ku WiFi!");
  display.display();
  delay(4000);

  // Inicializácia NTP klienta
  timeClient.begin();
}

void loop() {
  // Rozsvieť LED na modro počas merania
  digitalWrite(BUILTIN_LED, HIGH);

  // Čítanie teploty a vlhkosti
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Chyba pri citani z DHT senzora");
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Chyba senzora!");
    display.display();
    delay(5000);
    digitalWrite(BUILTIN_LED, LOW); // Zhasni LED po chybe
    return;
  }

  // Zaokrúhlenie na jedno desatinné miesto
  temp = round(temp * 10.0) / 10.0;
  hum = round(hum * 10.0) / 10.0;

  // Aktualizuj dátum a čas z NTP
  timeClient.update();
  String currentTime = timeClient.getFormattedTime();
  unsigned long epochTime = timeClient.getEpochTime();
  int currentDay = day(epochTime);
  int currentMonth = month(epochTime);
  int currentYear = year(epochTime);
  String currentDateTime = String(currentDay) + "." + String(currentMonth) + "." + String(currentYear) + " " + currentTime;

  // Výpočet sily Wi-Fi signálu v percentách
  int rssi = WiFi.RSSI(); 
  int wifiStrength = constrain(map(rssi, -100, -50, 0, 100), 0, 100); // Percentá od 0 do 100

  // Zobrazenie údajov na displeji
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(currentDateTime);
  
  display.setCursor(0, 10);
  display.print("Teplota: ");
  display.print(temp, 1);
  display.print((char)247);  // Symbol stupňov „°“
  display.println("C");
  display.setCursor(0, 20);
  display.print("Vlhkost: ");
  display.print(hum, 1);
  display.println(" %");

  // Štvrtý riadok ostáva prázdny
  display.setCursor(0, 30);
  display.println("");

  // Predposledný (5.) riadok: posledné namerané hodnoty
  display.setCursor(0, 40);
  display.print("Predtym: ");
  display.print(lastTemp, 1);
  display.print((char)247);  // Symbol stupňov „°“
  display.print("C/");
  display.print(lastHum, 1);
  display.println("%");

  // Zobrazenie Wi-Fi signálu v pravom dolnom rohu
  display.setCursor(64, 56); // Pravý dolný roh
  display.print("WiFi: ");
  display.print(wifiStrength);
  display.println("%");

  display.display();

  // Aktualizácia posledných nameraných hodnôt
  lastTemp = temp;
  lastHum = hum;

  // Po meraní nechaj LED svietiť na modro počas 3 sekúnd
  delay(3000);

  // Zhasni LED
  digitalWrite(BUILTIN_LED, LOW);

  // Čakaj 2 minúty pred ďalším meraním
  delay(120000);
}
