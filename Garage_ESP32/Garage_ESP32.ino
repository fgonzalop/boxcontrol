// ideaspark ESP32 + OLED 0.96" SSD1306 (128x64, I2C 0x3C) + módulo RGB Keyestudio + nRF24L01
//
// Conexión nRF24L01 -> ESP32 (bus VSPI)
//   GND  -> GND
//   VCC  -> 3V3  (¡nunca 5V! condensador 10-100uF entre VCC y GND recomendado)
//   CE   -> GPIO 4
//   CSN  -> GPIO 5
//   SCK  -> GPIO 18
//   MOSI -> GPIO 23
//   MISO -> GPIO 19
//   IRQ  -> sin conectar
#include <WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RF24.h>

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_ADDR 0x3C

// Módulo RGB (R G B -), cátodo común
#define LED_R 25
#define LED_G 26
#define LED_B 27

// nRF24L01
#define RF_CE  4
#define RF_CSN 5
#define RF_LEN 21  // tamaño del mensaje de los nodos BoxDomotic

const char* WIFI_SSID = "desconectada";
const char* WIFI_PASS = "casapepe.";

Adafruit_SSD1306 display(128, 64, &Wire, -1);
RF24 radio(RF_CE, RF_CSN);
const byte RF_ADDR[6] = "BoxDo";  // misma dirección que BoxDomoticNode / MasterNode

bool radioOk = false;
byte mensaje[32] = {0};
uint32_t recibidos = 0;
unsigned long ultimoRx = 0;

void mostrar() {
  display.clearDisplay();
  display.setTextSize(1);

  // La OLED es bicolor: filas 0-15 amarillas (fijo en hardware), 16-63 azules.
  // Título centrado en la franja amarilla; los datos van en la zona azul.
  const char* titulo = "BOX DOMOTIC MASTER";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(titulo, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - w) / 2, (16 - h) / 2);
  display.print(titulo);

  // Línea 1: WiFi
  display.setCursor(0, 16);
  if (WiFi.status() == WL_CONNECTED) {
    display.print("WiFi ");
    display.print(WiFi.localIP());
  } else {
    display.print("WiFi conectando...");
  }

  // Línea 2: radio
  display.setCursor(0, 26);
  if (!radioOk) {
    display.print("RF24 NO DETECTADO");
  } else {
    display.printf("RF24 OK  rx:%lu", (unsigned long)recibidos);
  }

  // Último mensaje recibido
  if (recibidos > 0) {
    display.setCursor(0, 36);
    display.printf("Nodo %d  PIR %d", mensaje[0], mensaje[4]);
    display.setCursor(0, 46);
    display.printf("T %d.%d  Lux %d.%d", mensaje[5], mensaje[6], mensaje[7], mensaje[8]);
    display.setCursor(0, 56);
    display.printf("R %d%d%d%d%d%d  hace %lus", mensaje[9], mensaje[10], mensaje[11],
                   mensaje[12], mensaje[13], mensaje[14], (millis() - ultimoRx) / 1000);
  }
  display.display();
}

void led(bool r, bool g, bool b) {
  digitalWrite(LED_R, r);
  digitalWrite(LED_G, g);
  digitalWrite(LED_B, b);
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  led(false, false, false);

  Wire.begin(OLED_SDA, OLED_SCL);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.setTextColor(SSD1306_WHITE);

  // Radio (SPI por defecto: SCK 18, MISO 19, MOSI 23)
  radioOk = radio.begin() && radio.isChipConnected();
  if (radioOk) {
    radio.setPALevel(RF24_PA_LOW);  // bajo para evitar brownout (alimentación USB)
    radio.openWritingPipe(RF_ADDR);
    radio.openReadingPipe(1, RF_ADDR);
    radio.startListening();
    Serial.println("RF24 inicializado");
    radio.printPrettyDetails();
  } else {
    Serial.println("RF24 no responde: revisa cableado y 3V3");
  }
  led(!radioOk, radioOk, false);  // rojo = sin radio, verde = radio OK

  // El ESP32 reintenta la conexión solo si se cae
  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);  // menos pico de consumo -> sin brownout
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  mostrar();
}

void loop() {
  static wl_status_t ultimoEstado = WL_IDLE_STATUS;
  static unsigned long ultimoRefresco = 0;

  wl_status_t estado = WiFi.status();
  if (estado != ultimoEstado) {
    ultimoEstado = estado;
    if (estado == WL_CONNECTED) {
      Serial.print("WiFi conectado, IP: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.printf("WiFi estado: %d\n", estado);
    }
  }

  if (radioOk && radio.available()) {
    radio.read(mensaje, RF_LEN);
    recibidos++;
    ultimoRx = millis();

    Serial.print("RX:");
    for (int i = 0; i < RF_LEN; i++) Serial.printf(" %d", mensaje[i]);
    Serial.println();

    led(false, false, true);  // destello azul al recibir
    mostrar();
    delay(50);
    led(false, true, false);
  }

  if (millis() - ultimoRefresco > 1000) {
    ultimoRefresco = millis();
    mostrar();
  }
}
