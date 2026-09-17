#include <WiFi.h>
#include <HTTPClient.h>

// ====== CONFIGURACIÓN WI-FI ======
const char* ssid = "TU_NOMBRE_DE_WIFI";
const char* password = "TU_PASSWORD_WIFI";

// IP de la computadora donde corre el servidor Python Flask
// (Averíguala en Windows con 'ipconfig' o en Linux/Mac con 'ifconfig')
const char* serverUrl = "http://192.168.1.50:5000/api/esp";

// ====== PINES ======
const int LDR_PIN = 34;
const int RELAY_PIN = 5;

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  
  // Inicializar relé apagado
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Si tu módulo de relé es activo bajo, usa HIGH

  Serial.println("\nConectando a Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n¡Conectado!");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    int valorLDR = analogRead(LDR_PIN);

    HTTPClient http;
    // Enviamos el valor actual del LDR al servidor Python
    String url = String(serverUrl) + "?ldr=" + String(valorLDR);
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      String response = http.getString();
      Serial.print("LDR: ");
      Serial.print(valorLDR);
      Serial.print(" | Respuesta Servidor: ");
      Serial.println(response);

      // Buscamos si el servidor nos dijo relay:1 o relay:0
      if (response.indexOf("\"relay\":1") > 0) {
        digitalWrite(RELAY_PIN, HIGH); // Encender foco
      } else {
        digitalWrite(RELAY_PIN, LOW);  // Apagar foco
      }
    } else {
      Serial.print("Error en petición HTTP: ");
      Serial.println(httpCode);
    }
    http.end();
  } else {
    Serial.println("Wi-Fi desconectado...");
  }

  delay(1000); // Consulta cada segundo
}