#include <TM1637Display.h>
#define CLK1 4  //diplej 1
#define DIO1 6 

#define CLK2 8  //displej 2
#define DIO2 9

#define ENC_A 2   //rotacni enkoder
#define ENC_B 3

//#define switchPin 7

int switchState = HIGH;

int btn1 = 5;     
int btn2 = 13;
int bzucak = 12;
int switchPin = 7;

TM1637Display diplej1(CLK1, DIO1);
TM1637Display diplej2(CLK2, DIO2);

unsigned long cas1 = 10; //sekund
unsigned long cas2 = 10;

bool button1Last = 1;
bool button2Last = 1;
bool switchLast = 1;
bool win = false;
bool setTime = false;

//pocet tahu
int moves1 = 0;
int moves2 = 0;

bool player1 = false;
bool player2 = false;


unsigned long _lastIncReadTime = micros(); 
unsigned long _lastDecReadTime = micros(); 
int _pauseLength = 25000;
int _fastIncrement = 10;

volatile int counter = 0;
int lastCounter = 0;



void Init();
void Delay(int x);
void CheckBtns();
void Win();




void setup() { 

  Serial.begin(9600);
  
  // I/O
  pinMode(btn1,INPUT);
  pinMode(btn2,INPUT);
  pinMode(bzucak, OUTPUT);

  
  //diplej
  diplej1.setBrightness(15);
  diplej1.showNumberDec(0,true);
  diplej2.setBrightness(15);
  diplej2.showNumberDec(0,true);

  digitalWrite(bzucak, HIGH);  //vypnuti bzucaku


  pinMode(ENC_A, INPUT_PULLUP);   //nastaveni pinu pro enkoder
  pinMode(ENC_B, INPUT_PULLUP);
  pinMode(switchPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A), read_encoder, CHANGE);    //preruseni pro enkoder
  attachInterrupt(digitalPinToInterrupt(ENC_B), read_encoder, CHANGE);


  delay(10);

  Init();
}






void loop() {

     
  uint8_t vypis1[] = { 0, 0, 0, 0 };
  uint8_t vypis2[] = { 0, 0, 0, 0 };
   
  vypis1[0] = diplej1.encodeDigit(cas1/600);
  vypis1[1] = diplej1.encodeDigit((cas1/60)%10);
  vypis1[2] = diplej1.encodeDigit((cas1%60)/10);
  vypis1[3] = diplej1.encodeDigit((cas1%60)%10);

  vypis2[0] = diplej2.encodeDigit(cas2/600);
  vypis2[1] = diplej2.encodeDigit((cas2/60)%10);
  vypis2[2] = diplej2.encodeDigit((cas2%60)/10);
  vypis2[3] = diplej2.encodeDigit((cas2%60)%10);

  if(cas1 == 0 || cas2 == 0)
  {
    player1 = false;
    player2 = false;    
     
    setTime = true; 
    win = true;
    Win(vypis1, vypis2);
  }

  diplej1.setSegments(vypis1);
  diplej2.setSegments(vypis2);

  if(player1 && cas1 > 0) cas1--;  
  if(player2 && cas2 > 0) cas2--;
  
    
  Delay(1000);  
  /*vypis1[1] += 128;
  diplej1.setSegments(vypis1);
  vypis2[1] += 128;
  diplej2.setSegments(vypis2);*/
   
}


void Delay(int x)
{
  unsigned long start = millis();

  while (millis() - start <= x) {
       CheckBtns();
       if(setTime)Counter();
  }

}

void Counter()
{  
    if(counter != lastCounter){
      Serial.println(counter);

      if(counter > lastCounter)
      {
        cas1 += 10;
        cas2 += 10;
      }
      else 
      {
        cas1 += -10;
        cas2 += -10;
      }

      lastCounter = counter;
    }
}

void CheckBtns()
{
  int button1State = digitalRead(btn1);
  int button2State = digitalRead(btn2);
  int switchState = digitalRead(switchPin);
 
  if(win && !switchState && switchLast)  
  //if(win && (button1State || button2State))
  {
    cas1 = 180;
    cas2 = 180;
    win = false;
    
    player1 = false;
    player2 = false;
    Serial.print("Znova\n");
  }
  else if(win && ((button1State && !button1Last ) || (button2State && !button2Last)))
  {
      diplej1.showNumberDec(moves1,false);
      diplej2.showNumberDec(moves2,false);

      delay(2000);
  }
  else if(button1State && !button1Last && !player2)
  {    
    player1 = false;
    player2 = true;
    setTime = false;
    moves1++;
    //zahraj(1500, 400);
    Serial.print("Button1\n");
  }
  else if(button2State && !button2Last && !player1)
  {
    player1 = true;
    player2 = false;
    setTime = false;
    moves2++;
    //zahraj(1500, 400);
    Serial.print("Button2\n");
  }
 

  
  button1Last = button1State;
  button2Last = button2State;
  switchLast = switchState;
}


void zahraj(int frekvence, int doba) {

  tone(bzucak, frekvence, doba);
  Delay(200);
  digitalWrite(bzucak, HIGH);

}


void Win(uint8_t x[], uint8_t y[])
{ 
  while(win)
  {
    diplej1.clear();
    diplej2.clear();
    Delay(250);
    diplej1.setSegments(x);
    diplej2.setSegments(y);
    Delay(400);
  }
    
}

//--------------------------------------------------
void Init()
{
  
  delay(1000);
  diplej1.clear();
  diplej2.clear();
  for(int i=9;i>=0;i--){
    for(int j=0; j<=4; j++){      
      diplej1.showNumberDec(i,false,j);
      diplej2.showNumberDec(i,false,j);
      delay(40);
    }
    diplej1.clear();
    diplej2.clear();
    delay(100);   
  }

  diplej1.showNumberDec(0,true);delay(100);
  diplej1.clear();delay(100);
  diplej1.showNumberDec(0,true);delay(100);
  diplej1.clear();delay(100);
  diplej2.showNumberDec(0,true);delay(100);
  diplej2.clear();delay(100);
  diplej2.showNumberDec(0,true);delay(100);
  diplej2.clear();delay(100);
  delay(600);
}

//Enkoder
//-------------------------------------------------

void read_encoder() {

  static uint8_t old_AB = 3;  // Lookup table index
  static int8_t encval = 0;   // Encoder value  
  static const int8_t enc_states[]  = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0}; // Lookup table

  old_AB <<=2;  // Remember previous state

  if (digitalRead(ENC_A)) old_AB |= 0x02; // Add current state of pin A
  if (digitalRead(ENC_B)) old_AB |= 0x01; // Add current state of pin B
  
  encval += enc_states[( old_AB & 0b00001111 )];    // maskovani prvních 4 bitů na 0



  // Update counter if encoder has rotated a full indent, that is at least 4 steps
  if( encval > 3 ) {        // Four steps forward
    int changevalue = 10;
    if((micros() - _lastIncReadTime) < _pauseLength) {                  //zrychlena inkrementace
      changevalue = _fastIncrement * changevalue; 
    }
    _lastIncReadTime = micros();
    counter = counter + changevalue;  
    //cas1 += changevalue;  
    //cas2 += changevalue;
    encval = 0;
  }
  else if( encval < -3 ) {        // Four steps backward
    int changevalue = -10;
    if((micros() - _lastDecReadTime) < _pauseLength) {
      changevalue = _fastIncrement * changevalue; 
    }
    _lastDecReadTime = micros();
    counter = counter + changevalue;            
    //cas1 += changevalue;
    //cas2 += changevalue;
    encval = 0;
  }
} 
