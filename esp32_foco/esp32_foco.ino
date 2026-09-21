#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

// ====== CONFIGURACIÓN WI-FI ======
//const char* ssid = "DESKTOP-TSJBC22-6549";
const char* ssid = "HONOR X7b";
//const char* password = "12345678900";
const char* password = "123456789";

// ====== SERVIDOR ======
const char* serverUrl = "https://control-foco.vercel.app/api/esp";

// ====== PINES ======
const int LDR_PIN = 34;
const int RELAY_PIN = 5;

// ====== UMBRAL ======
const int UMBRAL_LUZ = 100;

void setup() {
  Serial.begin(115200);

  pinMode(LDR_PIN, INPUT);

  // Igual que tu código original:
  // el relé comienza apagado/flotando.
  pinMode(RELAY_PIN, INPUT);

  Serial.println("\nConectando a Wi-Fi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\n¡Wi-Fi conectado!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // ====== LEER LDR ======
  int valorLDR = analogRead(LDR_PIN);

  Serial.print("AO: ");
  Serial.print(valorLDR);

  if (WiFi.status() != WL_CONNECTED) {
    // Si no hay Wi-Fi, usar lógica local
    bool oscuro = (valorLDR > UMBRAL_LUZ);
    if (oscuro) {
      Serial.println(" -> OSCURO (Modo Local)");
      pinMode(RELAY_PIN, OUTPUT);
      digitalWrite(RELAY_PIN, HIGH);
    } else {
      Serial.println(" -> LUZ (Modo Local)");
      pinMode(RELAY_PIN, INPUT);
    }
  } else {
    Serial.println(); // Salto de línea si hay Wi-Fi
  }

  // ====== ENVIAR LDR AL SERVIDOR ======
  if (WiFi.status() == WL_CONNECTED) {

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;

    String url = String(serverUrl) + "?ldr=" + String(valorLDR);

    Serial.print("Enviando: ");
    Serial.println(url);

    if (http.begin(client, url)) {

      int httpCode = http.GET();

      if (httpCode > 0) {

        Serial.print("HTTP: ");
        Serial.println(httpCode);

        String response = http.getString();

        Serial.print("Servidor: ");
        Serial.println(response);

        // --- APLICAR ESTADO SEGÚN RESPUESTA ---
        // Se busca el valor de "relay" en el JSON devuelto
        if (response.indexOf("\"relay\": 1") != -1 || response.indexOf("\"relay\":1") != -1) {
          Serial.println("-> Comando API: ENCENDER");
          pinMode(RELAY_PIN, OUTPUT);
          digitalWrite(RELAY_PIN, HIGH);
        } else if (response.indexOf("\"relay\": 0") != -1 || response.indexOf("\"relay\":0") != -1) {
          Serial.println("-> Comando API: APAGAR");
          pinMode(RELAY_PIN, INPUT);
        }

      } else {

        Serial.print("Error HTTP: ");
        Serial.println(http.errorToString(httpCode));

      }

      http.end();

    } else {

      Serial.println("No se pudo iniciar conexión HTTPS");

    }
  } else {

    Serial.println("Wi-Fi desconectado...");

  }

  delay(500);
}
