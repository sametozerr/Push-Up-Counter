#include <NewPing.h>            // Mesafe sens�r�n� kullanabilmek i�in kulland���m k�t�phane
#include <Wire.h>               // LCD Ekran i�in kulland���m k�t�phaneler
#include <LiquidCrystal_I2C.h>   
#include "Timer.h"				// Kendi yazd���m kronometre k�t�phanesi

// Joystick i�in kulland���m pinler 

#define J_BUTTON 8
#define J_VRX A0
#define J_VRY A1

// Mesafe sens�r� i�in kulland���m pinler

#define TRIGGER_PIN 6
#define ECHO_PIN 7

// ��nav i�in kullan�lan baz� sabitler

#define UPPER_LIMIT 40     // �st limit 
#define LOWER_LIMIT 12.5  // Alt limit
#define MAX_DISTANCE 100 // Sens�r taraf�ndan �l��lebilecek maksimum mesafe

#define INTERVAL 250    // Zaman aral���

// Plank i�in kullan�lan baz� sabitler

#define P_UPPER_LIMIT 30 
#define P_LOWER_LIMIT 10
#define UPPER_LIMIT_WARN "DOWN!" // 30cm > �zeri i�in verilen uyar�
#define LOWER_LIMIT_WARN "UP!"  // 10cm < �zeri i�in verilen uyar�

// Plank se�ene�i se�ildi mi diye kontrol etmek i�in
bool isPlankStart = false; 


// ��nav se�ene�i se�ildi mi diye kontrol etmek i�in
bool isPushUpsStart = false;

// Lcd nesnesi tan�mlama
LiquidCrystal_I2C lcd(0x27, 16, 2); 


// Mesafe sens�r� tan�mlama
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); 


// 250 mS aral�klarla i�lem yapt�rmak i�in zaman kontrol� 
unsigned long currentTime, startTime = 0; 


// Yukar� ve a�a�� e�ilme i�in kontrol (��nav)
bool isUp = false, isDown = false; 


// Mesafe ve ��nav say�s� de�i�kenleri
int distance = 0, numberOfPushUps = 0; 


// Se�enekler
String options[2] = { "1.Push-Up", "2.Plank" }; 


// Mevcut se�enek de�i�keni
int currentOptions = 1;

// Timer nesnesi tan�mlama
Timer t1;

void setup() 
{	
 
  /* Joystick butonunda pullup ba�lant�s� kullan�ld�.
     Buton bas�ld���nda(0), bas�lmazken(1) de�eri d�n�yor. */
  
  pinMode(J_BUTTON, INPUT_PULLUP); 


  lcd.backlight(); // Lcd �����n� aktif etme
  lcd.init(); // Lcd'yi ba�latma
}

void loop() {

   
  /* Arduino a��ld�ktan sonra ge�en saniye milisaniye cinsinden 
     currentTime de�i�keninin i�erisinde tutuluyor. */
  
  currentTime = millis(); 
  
  
  // 250 mS'de bir if'in i�erisindeki komutlar kontrol ediliyor.

  if (currentTime - startTime >= INTERVAL) {

    startTime = currentTime;

	
	// Joystick ile se�enekler aras�nda gezinme sistemi
	
    if(!isPushUpsStart && !isPlankStart) {
	
	// Sa�a git
    if (analogRead(J_VRX) > 1000 && currentOptions != 2) 
        currentOptions++;
        
    // Sola git    
    if (analogRead(J_VRX) < 50 && currentOptions != 1) 
        currentOptions--;
	
	// Se�
    if(digitalRead(J_BUTTON) == 0) {

    if(currentOptions == 1)
       isPushUpsStart = true;

    else 
       isPlankStart = true;
       
    }

    // Mevcut se�enek de�i�keni kontrol ediliyor.
    
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

	// ��nav se�ilmi�se ekrana ��nav say�s� yazd�r�l�yor.
	
    if(isPushUpsStart && currentOptions == 1) {

    lcd.clear();
    lcd.print(numberOfPushUps);

    }
    
  }

  // ��nav algoritmas�

  if(currentOptions == 1 && isPushUpsStart) {
  
  // Mesafe distance de�i�kenin cm cinsinden i�erisine yazd�r�l�yor
  
  distance = sonar.ping_cm();


  // Men�ye d�n�� i�in tasarland�.
  
  if(analogRead(J_VRY) > 1000) 
  {

  isPushUpsStart = false;
  isPlankStart = false;
  currentOptions = 1;

  }

  // Ki�inin e�ilip e�ilmedi�ini kontrol eden yap�
  
  if (distance < LOWER_LIMIT && distance != 0)
    isDown = true;

  // Ki�inin kalk�p kalkmad���n� kontrol eden yap�
  
  if (isDown && distance > UPPER_LIMIT)
    isUp = true;

  // ��nav do�ru yap�lm���a ��nav say�s� bir artar.
  
  if (isUp && isDown) {

    isUp = false;
    isDown = false;

    numberOfPushUps++;
  }

  }

  // Plank algoritmas�
  
  if(currentOptions == 2 && isPlankStart) {
  
  t1.run();  // Zamanlay�c� ba�lat�ld�.

  distance = sonar.ping_cm();

  /*
  Serial.print("Mesafe:");
  Serial.println(distance);
  
  Serial.print("Geçen süre:");
  Serial.println(t1.time);
  */

  // Men�ye d�n�� i�in tasarland�.
  
  if(analogRead(J_VRY) > 1000) 
  {

  isPushUpsStart = false;
  isPlankStart = false;
  currentOptions = 2;

  }

  /* Ki�i 30 cm'in a�a��s�na indi�inde uyar� veriliyor
   	 ve kronometre duruyor. */
  
  if(distance < P_LOWER_LIMIT) 
  {
  
  // Zaman� durdur
  t1.stop(); 
  
  // Uyar� ekrana yazd�r�l�yor.
  lcd.clear();
  lcd.print(LOWER_LIMIT_WARN);
  

  }

  // Ki�i 10 ve 30 cm aral���nda iken s�re say�l�yor.
  
  if(distance >= P_LOWER_LIMIT && distance <= P_UPPER_LIMIT) 
  {

  // Zaman� ba�lat
  t1.start();

  // Zaman ekrana yazd�r�l�yor.
  lcd.clear();
  lcd.print(t1.time);

  }
  
  /* Ki�i 30 cm'in yukar�s�na ��kt���nda uyar� veriliyor
   	 ve kronometre duruyor. */
        
  if (distance > P_UPPER_LIMIT) 
  {
     
  t1.stop();
 
  lcd.clear();
  lcd.print(UPPER_LIMIT_WARN);
 

  }     

  }

}
