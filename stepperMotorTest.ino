#include <Stepper.h>
#include <LiquidCrystal.h>
#include <time.h>
#include <Arduino_FreeRTOS.h>

#define portCHAR char

#undef autonomousSwitching
#define manualSwitching



const uint16_t stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution
// for your motor

uint8_t buttonPin = 2;
uint8_t lcdD4Pin = 6;
uint8_t lcdD5Pin = 5;
uint8_t lcdD6Pin = 4;
uint8_t lcdD7Pin = 3;
uint8_t lcdEPin = 12;
uint8_t lcdRSPin = 13;
volatile uint8_t switchDirection = 0;
uint8_t flag = 1;
uint8_t toggleDirection = 0;
enum directions{Clockwise, CounterClockwise};

void TaskMotor( void *pvParameters );
//void TaskTimer( void *pvParameters );

// initialize the stepper library on pins 8 through 11:
Stepper myStepper(stepsPerRevolution, 8,10,11,9);
LiquidCrystal lcd(lcdRSPin, lcdEPin, lcdD4Pin, lcdD5Pin, lcdD6Pin, lcdD7Pin);

void setup() {

  pinMode(buttonPin, INPUT_PULLUP);
  cli();//stop interrupts

  attachInterrupt(digitalPinToInterrupt(buttonPin), pinPushedISR, RISING);

//set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  Serial.begin(9600);
  lcd.begin(16,2);
  lcd.print("hello, world");

  sei();//allow interrupts

  
  //xTaskCreate(TaskTimer, (const portCHAR *)"T", 128, NULL, 1, NULL); 
  xTaskCreate(TaskMotor, (const portCHAR *)"M", 128, NULL, 1, NULL);
  
}

void loop() {
 
}

#ifdef autonomousSwitching
void TaskMotor(void *pvParameters) {
  (void) pvParameters;
 
  myStepper.setSpeed(9);
  for(;;) {
    switchDirection = 1;
    myStepper.step(stepsPerRevolution);
    
    
    switchDirection = 0;
    myStepper.step(-stepsPerRevolution);
    
    vTaskDelay(1);
  }
}
#endif

#ifdef manualSwitching
void TaskMotor(void *pvParameters) {
  (void) pvParameters;
 
  myStepper.setSpeed(9);
  for(;;) {
    if(toggleDirection) {
      myStepper.step(1);
      switchDirection = 1;
    }
    else {
      myStepper.step(-1);
      switchDirection = 0;
    }
  }
}
#endif
ISR(TIMER1_COMPA_vect){//timer1 interrupt 1Hz toggles pin 13 (LED)
    if(flag) {
      lcd.setCursor(0,0);
      lcd.clear();
      if(switchDirection){
        lcd.print("CounterClockwise");
        Serial.print("Counter\n");
      }
      else {
        lcd.print("Clockwise");
        Serial.print("Clockwise\n");
      }
      vTaskDelay(1);
    }
}

void pinPushedISR(){
  toggleDirection = !toggleDirection;
}








//===========================================example code===========================================
