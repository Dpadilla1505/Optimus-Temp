/* 
 * Project MidTerm Optimus Temp
 * Author: Damien Padilla
 * Date: 10/30/2026
 * For comprehensive documentation and examples, please visit:
 * https://docs.particle.io/firmware/best-practices/firmware-template/
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "neopixel.h"
#include "IoTClassroom_CNM.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_BME280.h"


//intro ( LEDSTRIP, leddisplay, BME280,(2) BUTTONS (auto & manual)(&)(hue LOW to HIGH + Wemo turn on when HIGH and off when LOW))
const int NEOSTRIP = D2; // 8 neopixel strip
const int PIXELCOUNT = 8;
const int ENCODER_CLK = D8;
const int ENCODER_DT = D9;
const int RLED = D13;
const int GLED = D14;
const int BLED = D15;

Button blueButton(A1); // auto to manual
Button redButton(D3); //hue buttom/
Button greenButton(D4); // on and off
Button EncSwButton(D16); //submit button in game

bool redButtonOnOff;
//display
const int SCREEN_W = 128;
const int SCREEN_H =64;
const int OLED_RESET = -1;
const int OLEDADDRESS = 0x3C;

SYSTEM_MODE(MANUAL);
Adafruit_NeoPixel strip(PIXELCOUNT, SPI1, WS2812B);
Adafruit_BME280 bme;
Adafruit_SSD1306 display( SCREEN_W, SCREEN_H, &Wire, OLED_RESET);

Encoder gameEnc(ENCODER_CLK, ENCODER_DT);
//hue bulbs
const int BULB1 = 3;
const int  BULB2 = 4;

//system modes game to auto
enum SystemMode {AUTO_MODE, MANUAL_MODE};
systemMode currentMode = AUTO_MODE;

//color match game
unsigned int targetColor;
unsigned int userColor;
bool puzzleMatch = false;
bool lastModeButtonState = HIGH;

bool systemPowerOn = true;
bool modeToggle = false;
// int encoderSelectionIndex = 0;
// int lastClkState;
// int targetColorIndex =0;
// unsigned long lastModePress =0;
// const unsigned long DEBOUNCE_DELAY =250;

struct ColorHub {
    const char* name;
    int r; int g; int b;
    int hueValue;
}
ColorHub Colors[8] = {
    {"Coldest" , 0, 0, 255, 45000},
    {"Cold" , 0, 128, 255, 48000},
    {"Cold" , 0, 200, 200, 51000},
    {"Cold", 100, 200, 255, 54000},
    {"Room Temp", 200, 200, 0, 10000},
    {"Room Temp", 255, 128, 0, 7000},
    {"Hot", 255, 64, 0, 3000},
    {"Hot", 255, 0, 0, 0}
};

const unsigned char check_bitmap[]= // need to put in
const unsigned char xmark_bitmap[]=  //need to put in

//game control
bool gameInitialized = false;
bool puzzleChecked = false;
bool puzzleCorrect = false;
int encoderSelectionIndex = 0;

void wemoWrite(int unitIndex, int state);
void setHue(int bulbNum, bool power, int hue, int brightness);
void handleAutoMode();
void handleManualMode();


// //manual brightness level
// enum BrightnessLevel {LOW_BRIGHT, HIGH_BRIGHT};
// BrightnessLevel manualBrightness = LOW_BRIGHT;


// //brightness levels
// const int brightnessLevels[] = {30, 120 , 255};
// const char* brightnessLabels[] = {"LOW, MED , HIGH"};
// currentBrightnessIdx = 1 ; 

// //variables
// int targetColorIndex =0;
// int userSelectColorIndex = 0;
// int lastClkState;
// bool gameWon = FALSE;

// //positon change
// unsigned long lastModePress = 0;
// unsigned long lastHuePress = 0;
// const unsigned long DEBOUNCE_DELAY =250;

// void setStripColor(r , g , b){
//     for(i = 0; i <PIXELCOUNT; i++) {
//         neostrip.setPixelColor(i, r , g , b);
//     }
//     neostrip.show();


void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected,15000);

    WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork");
    WiFi.connect();
        while(WiFi.connecting()) {
            Serial.printf(".");
            delay(500);
        }
        Serial.printf("\nNetwork Up\n");

    Serial.printf("\n\n");
    //pinmode config
    pinMode(RLED, OUTPUT);
    pinMode(GLED, OUTPUT);
    pinMode(BLED, OUTPUT);

    //NEOPIX talking to my display
    strip.begin();
    strip.show();
    bme.begin(0x76);
    display.begin(SSD1306_SWITCHCAPVCC, OLEDADDRESS);
    display.display(display);
    // lastClkState = digitalRead(ENCODER_CLK);
    wemoWrite (0,HIGH);
    wemoWrite (1, LOW);

    strand(Time,now());
}

void loop() {
    //mode button select
    if(greenButton.isClicked()) {
        systemPowerOn = !systemPowerOn;
    }
    if(blueButton.isClicked()) {
        modeToggle = !modeToggle;
    }


    if(!systemPowerOn) { // when system power is off do this
        display.clearDisplay();
        display.display();
        strip.clear();
        strip.show();
    }
    
    
    if(systemPowerOn) { // reading mode button
//mode toggle
        if(modeToggle == HIGH) {
         handleAutoMode();
        }


        if (modeToggle == LOW) {
         handleManualMode();
        }
   
     }
 }

void handleAutoMode () {
            float tempF = (bme.readTemperature() *9.0/5.0) +32.0;
            float bounded = constrain(tempF, 50.0, 95.0);
            int activePixels = map(bounded, 50, 95, 1);
            for(int i = 0; i < 8; i++) {

                for(i < activePixels) {
                    neostrip.setPixelColor(i, Colors[i].r, Colors[i].g, Colors[i].b);
                }
            }
      
            strip.show();
            int currentHue = Colors[constrain(activePixels -1, 0, 7)].hueValue;
            setHue(BULB2, HIGH, currentHue, 255);
            const char* rangeLabel = colors[constrain(activePixels -1, 0 , 7)].name;
            display.clearDisplay();
            //banner
            display.setTextSize(1);
            display.setCursor(0,0);
             display.printf(" SYSTEM MODE : AUTO");
            display.drawFastHLine(0, 10, 128, WHITE);

            //visiable
            display.setTextSize(2);
            display.setCursor(16, 16);
            display.printf("%.1f F", tempF);

            // //zone label
            // display.setTextSize(1);
            // display.setCursor( 16, 38);
            // display.printf("Status: %s", );

            //matching neopixels
            display.drawRect(16, 52, 96, 7, WHITE); //outline
            int barFillWidth = map((int)bounded, 50 , 95, 0, 92);
            display.fillRect(18, 54, barFillWidth, 3, WHITE); //fill 

            display.display();
}

void handleManualMode(){
            bool gameIntialized =false; 
            int targetColorIndex = rand () % 8;

            //start of the game (turning on manual mode)
            if(!gameIntialized){
                targetColorIndex = rand() & 8 ;
                puzzleMatch = false:
                encoderSelectionIndex = 0;
                gameEnc.Write(0);

                wemoWrite(0, HIGH);
                wemoWrite(1, LOW);
                gameIntialized = true;
            }
            // encoder can select index
            long encoderVal = gameEnc.read()  / 2;
            encoderSelectionIndex = encoderVal5 % 8;
            if(encoderSelectionIndex < 0){
                encoderSelectionIndex += 8;
            }
            //try to send moving hue on the neopixel to the hue lights
            if(redButton.isClicked() && !puzzleMatch){
                setHue (3, HIGH, Colors[encoderSelectionIndex].hueVal, 255); //bulb1
                setHue (4, HIGH, Colors[encoderSelectionIndex].hueVal, 255); //bulb2
            }
            //update neopixel
            strip.clear();
            if(puzzleMatch) {
                for( i = 0; i < PIXELCOUNT; i++){
                    strip.setPixelColor(i, Colors[targetColorsIndex].r, Colors[targetColorIndex].g, Colors[targetColorsIndex];
                }
            else{
                strip.setPixelColor(encoderSelectionIndex, Colors [encoderSelectionIndex].r, Colors[encoderSelectionIndex].g, Colors(encoderSelectionIndex).b);
            }
            strip.show();
            }
            // submit button
            if(EncButton.isClickeda() && !puzzleMatch) {
                puzzleMatch = true;
                wemoWrite(0,LOW);
                wemoWrite(1, HIGH);

                //win condiiton
                setHue(3, HIGH, 21845, 255);
                setHue (4, HIGH, 21845, 255);
            }         
           }
            //display it
            display.cleardisplay();
            display.setTextSize(1);
            display.setCursor(0,0);
            display.printf("SYSTEMMODE: MANAUL");
            display.drawFastHLine(0,10,128,WHITE);
            
            if(puzzleMatch) {
                display.setTextSize(2);
                display.setCursor(10, 20);
                display.printf("YOU WIN!");
                display.setTextSize(1);
                display.setCursor(10, 45);
                display.printf("Matched: %s", Colors[targetColorIndex].name);
                
                display.drawRect(100, 24, 20, 20, WHITE);
                display.drawLine(104, 34, 109, 39, WHITE); 
                display.drawLine(109, 39, 117, 28, WHITE);   
            }
            else {
                display.setTextSize(1);
                display.setCursor(0, 16);
                display.printf("Target: %s", Colors[targetColorIndex].name);
                display.setCursor(0, 30);
                display.printf("Slot Choice: %d", encoderSelectionIndex + 1);
                display.setCursor(0, 44);
                display.printf("Color: %s", Colors[encoderSelectionIndex].name);
                display.setCursor(0, 56);
                display.printf("[Red=Preview | Enc=Sub]");
                
                display.drawRect(104, 24, 16, 16, WHITE);
                display.drawLine(108, 28, 116, 36, WHITE); 
                display.drawLine(116, 28, 108, 36, WHITE); 
            }
            display.display();
            if(modeToggle == HIGH) {
                gameInitialized = false;
            }
        
