V tomto návode si ukážeme, ako pomocou mikrokontroléra ESP32 vieme využiť jeho funkcie. V tomto prípade, sa nám ESP32 po zapnutí najprv pripojí ku WiFi sieti (na to použijeme prenosný 4G WiFi router) a prostredníctvom neho do siete internet. Túto funkcionalitu potrebujeme z dôvodu, že ESP32 bude nastavené, aby pri každom meraní zobrazil aktuálny dátum a čas.

Súčasťou tohto projektu je aj OLED displej, ktorý slúži na zobrazovanie údajov. Po zapnutí nás privíta Úvodná hláška, následne nám displej ukáže, že sa pokúša pripojiť ku Wi-Fi sieti, a následne či sa pripojil. Následne vykoná pomocou senzora DHT11 meranie teploty a vlhkosti. Zároveň počas merania, sa nám okrem červenej statusovej diódy, ktorá na ESP32 svieti neustále (signalizuje, že ESP32 je napájaný, a v stave "ON") rozsvieti modrá (iba v čase merania). 

Namerané hodnoty zobrazí na OLED displeji, vrátane dátumu a času, stiahnutého pomocou časového severu pool.ntp.org. V predposlednom riadku, zobrazuje vždy posledné namerané hodnoty, aby si užívateľ vedel porovnať zmenu. Zároveň vpravo dole, zobrazuje silu signálu Wi-Fi v percentách.

Programový kód v ESP32 je nastavený tak, aby realizoval meranie každé 2 minúty. V prípade potreby je možné meranie zrealizovať okamžite - po stlačení tlačidla označeného "EN" na ESP32. 



Na to, aby sme celý tento projekt zrealizovali potrebujeme: 

-mikrokontrolér ESP32
-DHT11 modul teploty a vlhkosti
-displej 0,96" OLED
-830 bodové nepájive pole
-7x prepojovací M-M Dupont kábel
-1x prepojovací microUSB kábel (na prepojenie ESP32 s PC na nahratie kódu, potom na napájanie)
-zdroj napätia (USB nabíjačka, USB port v notebooku pre napájanie ESP32, displeja a senzora teploty)
-Wi-Fi router, na ktorý sa nám ESP32 bude pripájať, v našom prípade (4G mobilný WiFi router) a SIMkartu s dátovým programom (čo umožňuje využitie kdekoľvek, kde je signál mobilného operátora)
-PC a Software Arduino (bezplatne stiahhnuteľný), prostredníctvom ktorého realizujeme nahranie kódu do ESP32



Zdrojový kód ESP32 programu:



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

  display.println("Senzor");

  display.setTextSize(1);  // Menší text

  display.setCursor(0, 20);

  display.println("teploty, v1.00");

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





