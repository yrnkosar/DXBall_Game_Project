#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// enum yapısı
enum ScreenMenuState {
  MENU_BASLAT, 
  MENU_CIKIS
};

enum GameStatus {
  OYUN_MENU, 
  OYUN_RUN, 
  OYUN_CIKIS
};

// global değişkenler
ScreenMenuState selectedMenuOption = MENU_BASLAT;
GameStatus currentGameStatus = OYUN_MENU;
// Buton pinleri
const int downButtonPin = 34;  // Aşağı hareket için buton
const int upButtonPin = 35;    // Yukarı hareket için buton
const int startButtonPin = 36; // Başlatma/onaylama için buton

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define PLATE_WIDTH 30
#define PLATE_HEIGHT 4
#define PLATE_SPEED 6 // PLATE_SPEED değeri artırıldı
#define BALL_SIZE 2
#define BALL_SPEED_X 2
#define BALL_SPEED_Y 2
#define WINDOW_SIZE 1 // Kayan pencere boyutu
#define BRICK_ROWS 3
#define BRICK_COLUMNS 6
#define BRICK_WIDTH 16
#define BRICK_HEIGHT 6
#define BRICK_GAP 2

bool bricks[BRICK_ROWS][BRICK_COLUMNS]; // Tuğla matrisi
int potDegers[WINDOW_SIZE]; // Potansiyometre değerlerinin saklanacağı dizi
int potIndex = 0;           // Kayan pencere dizisindeki mevcut indeks
int platePosition =SCREEN_WIDTH / 2 - PLATE_WIDTH/2;
int ballPosX = SCREEN_WIDTH / 2;
int ballPosY = SCREEN_HEIGHT - 10;
int ballSpeedX = BALL_SPEED_X;
int ballSpeedY = -BALL_SPEED_Y;
int actLevel = 1; 
int lives = 3;// Oyuncunun canları

// Işık sensörü pini
const int lightSensorPin = 31; 
const int someThreshold = 512; 
unsigned long previousMillis = 0;
const long interval = 20; 
int score = 0;

// LED pinleri (can göstergesi için)
const int ledPins[3] = {31, 32, 33};
bool isLoseLive = false;
int liveUpX, liveUpY;

int digit[10] = {
  0b0111111, //0
  0b0000110, //1
  0b1011011, //2
  0b1001111, //3
  0b1100110, //4
  0b1101101, //5
  0b1111101, //6
  0b0000111, //7
  0b1111111, //8
  0b1101111  //9
};
int segmentPins[] = {24, 25, 26, 27, 28, 29, 30};
int digit1, digit2; // her segmenti kontrol etmek için atama

void setup(){ 
  Serial.begin(19200);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(startButtonPin, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Ekranın başlatılması
  display.display();
  delay(500); //  beklet
  display.clearDisplay();
  showMenuScreen();

  for (int i = 0; i < 7; i++) {
    pinMode(segmentPins[i], OUTPUT);
  }
  // Basamak kontrol pinleri
  pinMode(50, OUTPUT); // 1. basamak kontrol pini
  pinMode(52, OUTPUT); // 2. basamak kontrol pini
  // LED pinlerini çıkış olarak başlat ve yak
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH); // Başlangıçta bütün LED'ler yanık durumda 
  }
  display.clearDisplay();
  randomSeed(analogRead(0)); // Rastgele sayı üreticisini başlat
  // Tuğla matrisini başlat
  for (int i = 0; i < BRICK_ROWS; i++)
  {
    for (int j = 0; j < BRICK_COLUMNS; j++)
    {
      bricks[i][j] = true;
    }
  }
  showCurrentScore();
}
void showScoreMultiplexed() {
  digit1 = score / 10; // Onlar basamağı
  digit2 = score % 10; // Birler basamağı

  for (int k = 0; k < 20; k++) {
    // 1. basamağı göster
    digitalWrite(50, HIGH); // 1. basamağı aktif et
    digitalWrite(52, LOW);  // 2. basamağı pasif et
    dis(digit1); // Onlar basamağını göster
    delay(5); // Çok kısa bir gecikme

    // 2. basamağı göster
    digitalWrite(52, HIGH); // 2. basamağı aktif et
    digitalWrite(50, LOW);  // 1. basamağı pasif et
    dis(digit2); // Birler basamağını göster
    delay(5); // Çok kısa bir gecikme
  }
}
void loop() {
  switch(currentGameStatus) {
    case OYUN_MENU:
      handleMenu();
      break;
    case OYUN_RUN:
      handleGame();
      break;
    case OYUN_CIKIS:
      handleExit();
      break;
    default:
      break;
  }

}
void handleMenu() {
  switch (selectedMenuOption) {
    case MENU_BASLAT:
      if (isButtonPressed(upButtonPin)) {
        selectedMenuOption = MENU_CIKIS;
        showMenuScreen();
      } else if (isButtonPressed(downButtonPin)) {
        // No action needed when down button is pressed in MENU_START
      } else if (isButtonPressed(startButtonPin)) {
        currentGameStatus = OYUN_RUN;
        initializeGame();
      }
      break;
      
    case MENU_CIKIS:
      if (isButtonPressed(upButtonPin)) {
        // No action needed when up button is pressed in MENU_EXIT
      } else if (isButtonPressed(downButtonPin)) {
        selectedMenuOption = MENU_BASLAT;
        showMenuScreen();
      } else if (isButtonPressed(startButtonPin)) {
        currentGameStatus = OYUN_CIKIS;
      }
      break;
  }
}

void handleGame() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    updateBallPosition();
    previousMillis = currentMillis;
  }
   if (isLoseLive) {
    moveLifeUp();
  }
  int potDeger = analogRead(A0);
  updatePaddlePosition(potDeger);
  if (ballPosY >= SCREEN_HEIGHT) {
    evaluateBallDrop();
    
    if (lives <= 0) {
      endGame();
      return;
    }
  }

  checkBricks();

  display.clearDisplay();
  renderPaddle();
  drawBall();
  renderBricks();
  display.display();
  showScoreMultiplexed();
}

void handleExit() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("Oyunumuza\ngosterdiginiz\nilgi icin\ntesekkur ederiz."));
  display.display();

  delay(3000); // 3 saniye beklet
}

void initializeGame() {
  score = 0;
  lives = 3; // Oyuncunun can sayısını 3'e ayarla
  refreshLifeIndicators();
  // Oyun başladığında ilk ayarlar burada yapılacak
  display.clearDisplay();
  // Oyunun başlangıç ekranı vs. ayarlamalar
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("Oyun Basliyor..."));
  display.display();
  delay(1000); // Ekranı bir süre göster

  // Oyun başlangıç değerlerinin ayarlanması
  // Örneğin, topun başlangıç konumu, puanın sıfırlanması vs.
}
void showMenuScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  if (selectedMenuOption == MENU_BASLAT) {
    display.println(F("> BASLAT <"));
    display.println(F("  CIKIS"));
  } 
  else {
    display.println(F("  BASLAT"));
    display.println(F("> CIKIS <"));
  }
  display.display();
}

void renderPaddle()
{
  display.fillRect(platePosition, SCREEN_HEIGHT - PLATE_HEIGHT, PLATE_WIDTH, PLATE_HEIGHT, WHITE);
}

void erasePaddle()
{
  display.fillRect(platePosition, SCREEN_HEIGHT - PLATE_HEIGHT, PLATE_WIDTH, PLATE_HEIGHT, BLACK);
}

void updatePaddlePosition(int potDeger)
{
   erasePaddle();
   platePosition = map(potDeger, 0, 1023, 0, SCREEN_WIDTH - PLATE_WIDTH);
   renderPaddle();
}

void drawBall()
{
  display.fillCircle(ballPosX, ballPosY, BALL_SIZE, WHITE);
}

void eraseBall()
{
  display.fillCircle(ballPosX, ballPosY, BALL_SIZE, BLACK);
}

void updateBallPosition() {
  eraseBall();
  ballPosX += ballSpeedX;
  ballPosY += ballSpeedY;

  // Ekran kenarlarına çarpma kontrolü
  if (ballPosX <= 0 || ballPosX >= SCREEN_WIDTH - BALL_SIZE) {
    ballSpeedX = -ballSpeedX;
  }
  if (ballPosY <= 0) {
    ballSpeedY = -ballSpeedY;
  }
  else if (ballPosY >= SCREEN_HEIGHT - BALL_SIZE) {
    // Top ekranın altına düştüğünde can kaybını işle ve topu yeniden konumlandır
    evaluateBallDrop();
  }
  // Plakaya çarptığında yönünü değiştir
  if (ballPosY + BALL_SIZE >= SCREEN_HEIGHT - PLATE_HEIGHT &&
      ballPosX + BALL_SIZE >= platePosition && ballPosX <= platePosition + PLATE_WIDTH) {
    ballSpeedY = -ballSpeedY;
  }

  // Tuğlaları kontrol et ve kır
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      if (bricks[i][j]) {
        int brickX = j * (BRICK_WIDTH + BRICK_GAP);
        int brickY = i * (BRICK_HEIGHT + BRICK_GAP);
        // Topun tuğla içine girdiğini kontrol et
        if (ballPosX + BALL_SIZE > brickX && ballPosX < brickX + BRICK_WIDTH &&
            ballPosY + BALL_SIZE > brickY && ballPosY < brickY + BRICK_HEIGHT) {
          bricks[i][j] = false; // Tuğlayı yok et
          score++; // Skoru artır
          showCurrentScore(); // Skoru ekrana yazdır
          ballSpeedY = -ballSpeedY; // Yönü değiştir
          processBrickCollision(i, j); // Tuğla vurulduğunda özel etkileri işle
          break; // Döngüden çık, çünkü bir tuğla zaten kırıldı
        }
      }
    }
  }

  drawBall();
}
void renderBricks()
{
  for (int i = 0; i < BRICK_ROWS; i++)
  {
    for (int j = 0; j < BRICK_COLUMNS; j++)
    {
      if (bricks[i][j])
      {
        int brickX = j * (BRICK_WIDTH + BRICK_GAP);
        int brickY = i * (BRICK_HEIGHT + BRICK_GAP);
        display.fillRect(brickX, brickY, BRICK_WIDTH, BRICK_HEIGHT, WHITE);
      }
    }
  }
}
void showCurrentScore() {
  int digit1 = score / 10; // Onlar basamağı
  int digit2 = score % 10; // Birler basamağı
  
  digitalWrite(30, HIGH);
  digitalWrite(29, LOW);
  dis(digit1); // Onlar basamağını yazdır

  delay(1);

  // Eğer skor 10 veya daha fazlaysa, onlar basamağını göster
  if (digit2 > 0) {
    digitalWrite(29, HIGH); // Önceki hane kapalı
    digitalWrite(30, LOW);  // Bu hane açık
    dis(digit2);
    delay(1); // Gözlemleyebilmek için yeterli süre
  }

}

void dis(int num) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(segmentPins[i], bitRead(digit[num], i) ? HIGH : LOW); // Ortak katot varsayımıyla
  }
}
void setupLevel(int level) {
  float speedIncreaseFactor=1.0+0.2*(level-1);//increase speed by 20% each level
  ballSpeedX = BALL_SPEED_X * speedIncreaseFactor;
  ballSpeedY = -BALL_SPEED_Y * speedIncreaseFactor;

  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      bricks[i][j] = false; // Önce tüm tuğlaları pasif hale getir
    }
  }
// Seviyeye özel tuğla düzenleri
  switch (level) {
    case 1:
      // Tüm tuğlalar aktif
      for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
          bricks[i][j] = true;
        }
      }
      break;
    case 2:
      // Sadece ilk 3 sütunu aktifleştir
      for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < 3; j++) { 
          bricks[i][j] = true;
        }
      }
      break;
    case 3:
      // Ortada bir sütun
      for (int i = 0; i < BRICK_ROWS; i++) {
        bricks[i][2] = true; // Sadece orta sütun
      }
      break;
    case 4:
      // Çapraz düzen
      for (int i = 0; i < BRICK_ROWS; i++) {
        bricks[i][i] = true;
        bricks[i][BRICK_COLUMNS-i-1] = true;
      }
      break;
    case 5:
      // X şekli
      for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
          if (i == j || BRICK_ROWS-i-1 == j) {
            bricks[i][j] = true;
          }
        }
      }
      break;
    default:
      // Varsayılan olarak tüm tuğlaları aktif yap
      for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLUMNS; j++) {
          bricks[i][j] = true;
        }
      }
      break;
  }

  // Tuğlaları çiz
  renderBricks();
  // Can göstergesini ve skoru güncelle
  refreshLifeIndicators();
  showCurrentScore();
  // Topu ve platformu sıfırla
  initializeBallAndPaddle();
}
void initBricksCustom(bool customLayout[BRICK_ROWS][BRICK_COLUMNS]) {
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      bricks[i][j] = customLayout[i][j];
    }
  }
}
void resetLevel() {
  initializeBallAndPaddle();
  // Tuğlaları yeniden ayarla
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      bricks[i][j] = true;
    }
  }
  initializeBallAndPaddle();
  refreshLifeIndicators(); // LED'leri güncelle
}

void resetPlate() {
  platePosition = SCREEN_WIDTH / 2 - PLATE_WIDTH / 2; // Platformun X pozisyonunu ekranın ortasına ayarla
}
void initializeBallAndPaddle() {
  initializeBallPosition(); // Topu sıfırla
  resetPlate(); // Platformu sıfırla
}
void endGame() {
  setupLevel(actLevel);
  displayScoreAndReset(); // Skoru göster ve sıfırla
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds("Oyunu Kaybettiniz!", 0, 0, &x, &y, &w, &h);

  // Metni ekranın ortasına yerleştir
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
  display.println("Oyunu Kaybettiniz!");

  display.display();
  delay(2000); // 2 saniye bekleyelim
  display.clearDisplay();

  currentGameStatus = OYUN_MENU;
  showMenuScreen();
}

void boostBallVelocity() {
  // Topun hızını %20 artır
  ballSpeedX *= 1.8;
  ballSpeedY *= 1.8;
}
bool isButtonPressed(int buttonPin) {
  static unsigned long lastDebounceTime = 0;
  const long debounceDelay = 50;
  
  bool pressed = false;
  if (digitalRead(buttonPin) == LOW) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      pressed = true;
      lastDebounceTime = millis();
    }
  }
  return pressed;
}

// Can göstergesini güncelle
void refreshLifeIndicators() {
  for (int i = 0; i < 3; i++) {
    if (i < lives) {
      digitalWrite(ledPins[i], HIGH); // Can varsa LED'i yak
    } else {
      digitalWrite(ledPins[i], LOW); // Can yoksa LED'i söndür
    }
  }
}
// Top aşağı düştüğünde can kaybını işle
void evaluateBallDrop() {
  lives--; // Canı azalt
  refreshLifeIndicators(); // Can göstergesini güncelle
  if (lives > 0) {
    initializeBallPosition(); // Topu sıfırla ve oyunu devam ettir
  }
}

void updateDisplayColors() {
  int lightValue = analogRead(lightSensorPin);
  if (lightValue < someThreshold) {
    // Düşük ışıkta: Arka planı siyah, tuğlaları beyaz yap
    display.clearDisplay(); // Ekranı temizle
    display.setTextColor(WHITE); // Yazı rengini beyaz yap
    display.fillScreen(BLACK); // Arka planı siyah yap
  } else {
    // Yüksek ışıkta: Arka planı beyaz, tuğlaları siyah yap
    display.clearDisplay(); // Ekranı temizle
    display.setTextColor(BLACK); // Yazı rengini siyah yap
    display.fillScreen(WHITE); // Arka planı beyaz yap
  }
  // Renkleri güncelledikten sonra tuğlaları ve diğer oyun nesnelerini tekrar çizmek gerekebilir.
  renderBricks(); // Tuğlaları tekrar çiz
}
// Oyunda bekleme ekranı ve yeni bölüm başlatma
void checkBricks() {
  bool allBricksDestroyed = true;
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      if (bricks[i][j]) {
        allBricksDestroyed = false;
        break;
      }
    }
    if (!allBricksDestroyed) {
      break;
    }
  }

  if (allBricksDestroyed) {
    levelDisComp();
    delay(2000); // 2 saniye bekle
    actLevel++; // Seviyeyi artır
    if (actLevel > 5) {
      actLevel = 1; // 5. seviyeden sonra tekrar 1. seviyeye dön
    }
    setupLevel(actLevel);

    //generateRandomBricks();
    boostBallVelocity();
    currentGameStatus = OYUN_RUN; // Yeni bölümü başlat
  }
}
void moveLifeUp() {
   // Can artırma objesi düşüyor mu kontrol et
  if (isLoseLive) {
    // Can artırma objesini hareket ettir
    liveUpY++;
    // Can artırma objesini çiz
    display.fillRect(liveUpX - 5, liveUpY - 5, 10, 10, WHITE); // Objeyi görselleştir
    display.display();

    // Paletin can artırma objesini yakalayıp yakalamadığını kontrol et
    if (liveUpY >= SCREEN_HEIGHT - PLATE_HEIGHT && liveUpX >= platePosition && liveUpX <= platePosition + PLATE_WIDTH) {
      // Canı artır ve LED ile göster
      lives++;
      refreshLifeIndicators(); // Can göstergesini güncelle
      isLoseLive = false; // Düşme durumunu sıfırla
    } else if (liveUpY >= SCREEN_HEIGHT) {
      // Eğer obje ekranın altına düştüyse
      isLoseLive = false; // Düşme durumunu sıfırla
    }
  }

}
void initializeBallPosition() {
  ballPosX = SCREEN_WIDTH / 2; // Reset ball X position
  ballPosY = SCREEN_HEIGHT / 2; // Reset ball Y position
  ballSpeedX = BALL_SPEED_X; // Reset ball speed X
  ballSpeedY = -BALL_SPEED_Y; // Reset ball speed Y
}
void processBrickCollision(int row, int col) {
  // %10 şansla can artırma objesi yarat
  if (random(10) < 1) {
    isLoseLive = true;
    liveUpX = col * (BRICK_WIDTH + BRICK_GAP) + BRICK_WIDTH / 2; // Can özelliğinin X pozisyonu
    liveUpY = row * (BRICK_HEIGHT + BRICK_GAP) + BRICK_HEIGHT; // Can özelliğinin Y pozisyonu
  }
}

void displayScoreAndReset() {
  // Skoru göster
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(F("Score: "));
  display.println(score);
  // Metnin genişliğini ölç
  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds("Oyunu Kaybettiniz!", 0, 0, &x, &y, &w, &h);
  display.display();
  delay(3000); // 3 saniye bekle
  currentGameStatus = OYUN_MENU; // Ana menüye dön
}
void levelDisComp() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor((SCREEN_WIDTH - (12 * 6)) / 2, SCREEN_HEIGHT / 2 - 8); // 12px harf genişliği, 6 harf "Level " kelimesi
  display.print(F("Level "));
  display.println(actLevel);
  display.display();
}
