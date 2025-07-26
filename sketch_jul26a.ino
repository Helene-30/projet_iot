#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <LiquidCrystal.h>  // Librairie LCD

// === Broches LCD (RS, E, D4, D5, D6, D7) ===
LiquidCrystal lcd(D1, D2, D3, D4, D6, D7);

// === Pin de réception IR ===
const uint16_t RECV_PIN = D5;  // D5 = GPIO14

// === Configuration WiFi ===
const char* ssid = "ESITEC_ETUDIANT";
const char* password = "Esitec2025++";

// === Adresse du serveur PHP ===
const char* host = "10.1.0.174";
const String endpoint = "/projet_IOT/ajouter_badge.php";

// === Récepteur IR ===
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  Serial.begin(115200);
  delay(100);

  // Init LCD
  lcd.begin(16, 2);
  lcd.print("Demarrage...");

  // Connexion WiFi
  Serial.print("Connexion au WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\n Connecté au WiFi !");
  lcd.clear();
  lcd.print("WiFi connecte");

  // Activation du récepteur IR
  irrecv.enableIRIn();
  Serial.println(" En attente du code IR...");
  lcd.setCursor(0, 1);
  lcd.print("Attente IR...");
}

void loop() {
  if (irrecv.decode(&results)) {
    unsigned long rawCode = results.value;

    lcd.clear();

    if (rawCode == 0xFFFFFFFF || rawCode == 0x0 || rawCode == 0xFFFFFFFFFFFFFFFF) {
      Serial.println("⚠ Code IR invalide ou répétition ignoré...");
      lcd.print("Code invalide");
    } else {
      String matricule = String(rawCode, DEC);
      Serial.println(" Code IR reçu : " + matricule);
      lcd.print("Matricule:");
      lcd.setCursor(0, 1);
      lcd.print(matricule);

      if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;

        String fullURL = "http://" + String(host) + endpoint + "?matricule=" + matricule;
        Serial.println("➡ Envoi vers : " + fullURL);

        http.begin(client, fullURL);
        int httpCode = http.GET();

        lcd.clear();

        if (httpCode > 0) {
          String response = http.getString();
          Serial.println(" Réponse serveur : " + response);

          if (response.indexOf("accepte") != -1 || response.indexOf("Accepté") != -1) {
            lcd.print(" Accepte");
          } else {
            lcd.print(" Refuse");
          }

        } else {
          Serial.println(" Erreur HTTP : " + String(httpCode));
          lcd.print("Erreur HTTP");
        }
        http.end();
      } else {
        Serial.println(" WiFi non connecté !");
        lcd.print("WiFi perdu !");
      }
    }

    irrecv.resume();  // Prêt pour un autre code
    delay(1500);       // Pause avant effacement
    lcd.clear();
    lcd.print("Attente IR...");
  }
}