#include <NewPing.h>            // Mesafe sensörünü kullanabilmek için kullandýðým kütüphane
#include <Wire.h>               // LCD Ekran için kullandýðým kütüphaneler
#include <LiquidCrystal_I2C.h>   
#include "Timer.h"				// Kendi yazdýðým kronometre kütüphanesi

// Joystick için kullandýðým pinler 

#define J_BUTTON 8
#define J_VRX A0
#define J_VRY A1

// Mesafe sensörü için kullandýðým pinler

#define TRIGGER_PIN 6
#define ECHO_PIN 7

// Þýnav için kullanýlan bazý sabitler

#define UPPER_LIMIT 40     // Üst limit 
#define LOWER_LIMIT 12.5  // Alt limit
#define MAX_DISTANCE 100 // Sensör tarafýndan ölçülebilecek maksimum mesafe

#define INTERVAL 250    // Zaman aralýðý

// Plank için kullanýlan bazý sabitler

#define P_UPPER_LIMIT 30 
#define P_LOWER_LIMIT 10
#define UPPER_LIMIT_WARN "DOWN!" // 30cm > üzeri için verilen uyarý
#define LOWER_LIMIT_WARN "UP!"  // 10cm < üzeri için verilen uyarý

// Plank seçeneði seçildi mi diye kontrol etmek için
bool isPlankStart = false; 


// Þýnav seçeneði seçildi mi diye kontrol etmek için
bool isPushUpsStart = false;

// Lcd nesnesi tanýmlama
LiquidCrystal_I2C lcd(0x27, 16, 2); 


// Mesafe sensörü tanýmlama
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 


// 250 mS aralýklarla iþlem yaptýrmak için zaman kontrolü 
unsigned long currentTime, startTime = 0; 


// Yukarý ve aþaðý eðilme için kontrol (Þýnav)
bool isUp = false, isDown = false; 


// Mesafe ve þýnav sayýsý deðiþkenleri
int distance = 0, numberOfPushUps = 0; 


// Seçenekler
String options[2] = { "1.Push-Up", "2.Plank" }; 


// Mevcut seçenek deðiþkeni
int currentOptions = 1;

// Timer nesnesi tanýmlama
Timer t1;

void setup() 
{	
 
  /* Joystick butonunda pullup baðlantýsý kullanýldý.
     Buton basýldýðýnda(0), basýlmazken(1) deðeri dönüyor. */
  
  pinMode(J_BUTTON, INPUT_PULLUP); 


  lcd.backlight(); // Lcd ýþýðýný aktif etme
  lcd.init(); // Lcd'yi baþlatma
}

void loop() {

   
  /* Arduino açýldýktan sonra geçen saniye milisaniye cinsinden 
     currentTime deðiþkeninin içerisinde tutuluyor. */
  
  currentTime = millis(); 
  
  
  // 250 mS'de bir if'in içerisindeki komutlar kontrol ediliyor.

  if (currentTime - startTime >= INTERVAL) {

    startTime = currentTime;

	
	// Joystick ile seçenekler arasýnda gezinme sistemi
	
    if(!isPushUpsStart && !isPlankStart) {
	
	// Saða git
    if (analogRead(J_VRX) > 1000 && currentOptions != 2) 
        currentOptions++;
        
    // Sola git    
    if (analogRead(J_VRX) < 50 && currentOptions != 1) 
        currentOptions--;
	
	// Seç
    if(digitalRead(J_BUTTON) == 0) {

    if(currentOptions == 1)
       isPushUpsStart = true;

    else 
       isPlankStart = true;
       
    }

    // Mevcut seçenek deðiþkeni kontrol ediliyor.
    
    switch(currentOptions) {
    
    case 1:
    lcd.clear();
    lcd.print(options[0]);
    break;


    case 2:
    lcd.clear();
    lcd.print(options[1]);
    break;

    default:

    break;

    }

    }

	// Þýnav seçilmiþse ekrana þýnav sayýsý yazdýrýlýyor.
	
    if(isPushUpsStart && currentOptions == 1) {

    lcd.clear();
    lcd.print(numberOfPushUps);

    }
    
  }

  // Þýnav algoritmasý

  if(currentOptions == 1 && isPushUpsStart) {
  
  // Mesafe distance deðiþkenin cm cinsinden içerisine yazdýrýlýyor
  
  distance = sonar.ping_cm();


  // Menüye dönüþ için tasarlandý.
  
  if(analogRead(J_VRY) > 1000) 
  {

  isPushUpsStart = false;
  isPlankStart = false;
  currentOptions = 1;

  }

  // Kiþinin eðilip eðilmediðini kontrol eden yapý
  
  if (distance < LOWER_LIMIT && distance != 0)
    isDown = true;

  // Kiþinin kalkýp kalkmadýðýný kontrol eden yapý
  
  if (isDown && distance > UPPER_LIMIT)
    isUp = true;

  // Þýnav doðru yapýlmýþþa þýnav sayýsý bir artar.
  
  if (isUp && isDown) {

    isUp = false;
    isDown = false;

    numberOfPushUps++;
  }

  }

  // Plank algoritmasý
  
  if(currentOptions == 2 && isPlankStart) {
  
  t1.run();  // Zamanlayýcý baþlatýldý.

  distance = sonar.ping_cm();

  /*
  Serial.print("Mesafe:");
  Serial.println(distance);
  
  Serial.print("GeÃ§en sÃ¼re:");
  Serial.println(t1.time);
  */

  // Menüye dönüþ için tasarlandý.
  
  if(analogRead(J_VRY) > 1000) 
  {

  isPushUpsStart = false;
  isPlankStart = false;
  currentOptions = 2;

  }

  /* Kiþi 30 cm'in aþaðýsýna indiðinde uyarý veriliyor
   	 ve kronometre duruyor. */
  
  if(distance < P_LOWER_LIMIT) 
  {
  
  // Zamaný durdur
  t1.stop(); 
  
  // Uyarý ekrana yazdýrýlýyor.
  lcd.clear();
  lcd.print(LOWER_LIMIT_WARN);
  

  }

  // Kiþi 10 ve 30 cm aralýðýnda iken süre sayýlýyor.
  
  if(distance >= P_LOWER_LIMIT && distance <= P_UPPER_LIMIT) 
  {

  // Zamaný baþlat
  t1.start();

  // Zaman ekrana yazdýrýlýyor.
  lcd.clear();
  lcd.print(t1.time);

  }
  
  /* Kiþi 30 cm'in yukarýsýna çýktýðýnda uyarý veriliyor
   	 ve kronometre duruyor. */
        
  if (distance > P_UPPER_LIMIT) 
  {
     
  t1.stop();
 
  lcd.clear();
  lcd.print(UPPER_LIMIT_WARN);
 

  }     

  }

}
