#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPL620uIKDYp"
#define BLYNK_TEMPLATE_NAME "AkvaryumOtomasyonu"
#define BLYNK_AUTH_TOKEN ""
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Servo.h>
#include <ESP8266WiFi.h>  
#include <BlynkSimpleEsp8266.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "";
char pass[] = "";

const int ANALOG_GIRIS_PINI = A0;
const int SU_MOTOR_PINI = D6;
const int LED_YESIL = D7;
const int LED_KIRMIZI = D1;
const int SICAKLIK_PIN = D2;
const int YEMLEME_PINI = D3;
const int HAVA_MOTOR_PINI = D0;
const int DIS_FILTRE_PINI = D8;
const int LAMBA_PINI = D4;

int suSensorDegeri = 0;
int suCikisDegeri = 0;

OneWire oneWire(SICAKLIK_PIN);
DallasTemperature sensors(&oneWire);
Servo servo;

void sicaklikKontrol() {
    sensors.requestTemperatures();
    double sicaklik = sensors.getTempCByIndex(0);

    if (sicaklik == DEVICE_DISCONNECTED_C) {
        String hataMesaji = "Sıcaklık sensörü bağlı değil veya hata veriyor!";
        Blynk.virtualWrite(V2, hataMesaji);
        Serial.println(hataMesaji);
        return;
    }

    Serial.print("Oda sıcaklığı: ");
    Serial.print(sicaklik);
    Serial.println(" derece");

    if (sicaklik > 30) {
        String yuksekSicaklik = "Sıcaklık " + String(sicaklik) + " °C > 30 °C, kırmızı LED yanacak";
        Blynk.logEvent("sicaklik", "Akvaryum sıcaklığı yüksek!!!");
        Blynk.virtualWrite(V2, yuksekSicaklik);
        Serial.println(yuksekSicaklik);
        digitalWrite(LED_YESIL, LOW);
        digitalWrite(LED_KIRMIZI, HIGH);
    } else {
        String dusukSicaklik = "Sıcaklık: " + String(sicaklik) + " °C < 30 °C, yeşil LED yanacak";
        Blynk.virtualWrite(V2, dusukSicaklik);
        Serial.println(dusukSicaklik);
        digitalWrite(LED_YESIL, HIGH);
        digitalWrite(LED_KIRMIZI, LOW);
    }
}

void suSeviyeKontrol() {
    suSensorDegeri = analogRead(ANALOG_GIRIS_PINI);
    suCikisDegeri = map(suSensorDegeri, 0, 1023, 0, 255);
    int esikDeger = 100;

    if (suCikisDegeri > esikDeger) {
        String suSeviyesi = "Su seviyesi: " + String(suCikisDegeri) + " Motor kapatılacak.";
        Blynk.virtualWrite(V0, suSeviyesi);
        Serial.println("Su seviyesi eşik değerden yüksek, motor kapatılacak");
        digitalWrite(SU_MOTOR_PINI, HIGH);
    } else {
        String suSeviyesi = "Su seviyesi: " + String(suCikisDegeri) + " Motor açılacak.";
        Blynk.virtualWrite(V0, suSeviyesi);
        Serial.println("Su seviyesi eşik değerden düşük, motor açılacak");
        digitalWrite(SU_MOTOR_PINI, LOW);
    }

    Serial.print("sensor = ");
    Serial.print(suSensorDegeri);
    Serial.print("\t çıkış = ");
    Serial.println(suCikisDegeri);
}

BLYNK_WRITE(V3) {
    int pinValue = param.asInt();
    Serial.print("Buton durumu: ");
    Serial.println(pinValue);

    if (pinValue == 1) {
        servo.write(90);
        delay(2000);
        Serial.println("Balık yemlendi");
        servo.write(180);
        delay(2000);
    } else {
        servo.write(0);
        delay(2000);
    }
}

BLYNK_WRITE(V4) {
    int pinValue = param.asInt();
    Serial.print("Buton durumu: ");
    Serial.println(pinValue);

    if (pinValue == 1) {
        digitalWrite(LAMBA_PINI, HIGH);
    } else {
        digitalWrite(LAMBA_PINI, LOW);
    }
}

BLYNK_WRITE(V8) {
    int pinValue = param.asInt();
    Serial.print("Buton durumu: ");
    Serial.println(pinValue);

    if (pinValue == 1) {
        digitalWrite(DIS_FILTRE_PINI, HIGH);
    } else {
        digitalWrite(DIS_FILTRE_PINI, LOW);
    }
}

void setup() {
    Serial.begin(115200);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    pinMode(SU_MOTOR_PINI, OUTPUT);
    pinMode(LED_YESIL, OUTPUT);
    pinMode(LED_KIRMIZI, OUTPUT);
    pinMode(YEMLEME_PINI, OUTPUT);
    pinMode(HAVA_MOTOR_PINI, OUTPUT);
    pinMode(DIS_FILTRE_PINI, OUTPUT);
    pinMode(LAMBA_PINI, OUTPUT);

    sensors.begin();
    servo.attach(YEMLEME_PINI);
    servo.write(0);
    delay(2000);
}

void loop() {
    if (Blynk.connected()) {
        Serial.println("Sunucuya bağlı.");
        sicaklikKontrol();
        suSeviyeKontrol();
        Blynk.run();
        delay(2000);
    } else {
        Serial.println("WiFi bağlantısı kesildi, tekrar bağlanılıyor...");
        if (Blynk.connect()) {
            Serial.println("WiFi bağlantısı yeniden kuruldu!");
        } else {
            Serial.println("WiFi bağlantısı kurulamadı!");
        }
        delay(1000);
    }
}
