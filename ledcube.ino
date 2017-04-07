//static definition of number of LED rows, columns and layers
/*
example:
   e-------f
  /|      /|
 / |     / |
a--|----b  |
|  g----|--h
| /     | /
c-------d
a,b,c,d are on the same row
a,c,g,e are on the same column
a,b,e,f are on the same layer
a,c are on the same pillar
*/
#define numlayers 3
#define numrows 3
#define numcolumns 3

#define numleds numrows*numlayers*numcolumns


//LEDs on each layer of the cube have common ground/cathode/(-)
//LEDs on each vertical "pillar" have common anode/(+)

//pins that control individual "pillars" 
int pillars[numrows][numcolumns] = {
                 { 11, 13, 12},  
                 { 8, 10, 9 },
                 { 5, 7, 6},
              };
              
//pins that control individual layers
int layers[numlayers] = {2, 3, 4};

// 3-dimensional array onto which the current state of each LED is saved
boolean cube[numrows][numcolumns][numlayers];

//current layer being lit up - controlled by the multiplexing function
int currentlayer = 0;

//get values of the current multiplexed layer onto the respective pillar pins
void updatelayer(){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
    //HIGH-ON for pillars
      if (cube[x][y][currentlayer]==true) {
        digitalWrite(pillars[x][y],HIGH);
      }
      else {
        digitalWrite(pillars[x][y],LOW);
      }
    }
  }
}

//interrupt calls this function
ISR(TIMER1_COMPA_vect) {//Interrupt at freq of 1kHz to measure reed switch
  currentlayer=currentlayer+1;
  if (currentlayer>=numlayers) currentlayer=0;
  for (int i=0;i<numlayers;i++){
  //HIGH-OFF for layers
    if (i==currentlayer) digitalWrite(layers[i], LOW);
    else digitalWrite(layers[i], HIGH);
  }
  updatelayer();
}

void blinkcolumn(int led, int waittime){
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(waittime);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW 
}

// the setup routine runs once when you press reset:
void setup() {      
  //SET INTERRUPTS 
  //This handles the multiplexing of layers
  cli();//stop interrupts
  //set timer1 interrupt
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set timer count 
  OCR1A = 90;// = (16*10^6) / ((DESIREDFREQUENCY)*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
      
  // initialize the digital pins as outputs
  for (int i=0;i<numcolumns;i++){
    for (int j=0;j<numrows;j++){
      pinMode(pillars[i][j], OUTPUT);
    }
  }
  for (int i=0;i<numlayers;i++){
    pinMode(layers[i], OUTPUT);
  }
  Serial.begin(9600);
  
  randomSeed(analogRead(A0)+analogRead(A1)*analogRead(A2));
  
  sei();//allow interrupts
}

//turns a specified pillar on
void pillaron(int x, int y){
  for (int i=0;i<numlayers;i++){
    cube[x][y][i]=true;
  }
}

//turns a specified pillar off
void pillaroff(int x, int y){
  for (int i=0;i<numlayers;i++){
    cube[x][y][i]=false;
  }
}

//Serially prints the current state of the cube - for debugging mostly
void cubestate(){
  Serial.print("\n=====\n");
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      for (int z=0;z<numlayers;z++){
         if (cube[z][y][x]==true) {
          Serial.print("1 ");
        }
        else {
          Serial.print("0 ");
        }
      }
      Serial.print("\n");
    }
    Serial.print("\n=====\n");  
  }
  Serial.println(" \n\n");
}

//light each pillar on and off consecutively
void cyclepillars(int delaytime){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      pillaron(x,y);
      //cubestate();
      delay(delaytime);
      pillaroff(x,y);
      //cubestate();
      //delay(delaytime);
    }
  }
}

//turn a specific led on
void ledon(int x, int y, int z){
  cube[x][y][z]=true;
}

//turn a specific led off
void ledoff(int x, int y, int z){
  cube[x][y][z]=false;
}

//randombug
//some stuff will not work properly unless an external serial call is done
//wtf - help
void bugfix(){Serial.print(" ");}

//turn each individual LED on and then off
void cycleleds(int delaytime){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      for (int z=0;z<numlayers;z++){
        ledon(x,y,z);
        delay(delaytime);
        //cubestate();
        bugfix();
        ledoff(x,y,z);
        //delay(delaytime);
        //cubestate();
      }
    }
  }
}

//turn all LEDS on, one at a time, randomly
void randomon(int delaytime){
  int counter=0;
  while (counter<numleds){
    int x=random(numrows);
    int y=random(numcolumns);
    int z=random(numlayers);
    if (cube[x][y][z]==false){
      cube[x][y][z]=true;
      counter++;
      delay(delaytime);
    }
  }
}

//turn all LEDS off, one at a time, randomly
void randomoff(int delaytime){
  int counter=0;
  while (counter<numleds){
    int x=random(numrows);
    int y=random(numcolumns);
    int z=random(numlayers);
    if (cube[x][y][z]==true){
      cube[x][y][z]=false;
      counter++;
      delay(delaytime);
    }
  }
}

//turn all leds on
void cubeon(){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      for (int z=0;z<numlayers;z++){
        ledon(x,y,z);
      }
    }
  }
}

//turn all leds off
void cubeoff(){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      for (int z=0;z<numlayers;z++){
        ledoff(x,y,z);
      }
    }
  }
}

//turn a specified layer on
void layeron(int sellayer){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      ledon(x,y,sellayer);
    }
  } 
}

//turn a specified layer off
void layeroff(int sellayer){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numcolumns;y++){
      ledoff(x,y,sellayer);
    }
  } 
}

//cycle through all layers consecutively
void cyclelayers(int delaytime){
  for (int z=0;z<numlayers;z++){
    layeron(z);
    delay(delaytime);
    layeroff(z);
    //delay(delaytime);
  }
}

//turn a specified row on
void rowon(int selrow){
  for (int x=0;x<numcolumns;x++){
    for (int y=0;y<numlayers;y++){
      ledon(selrow,x,y);
    }
  } 
}

//turn a specified row off
void rowoff(int selrow){
  for (int x=0;x<numcolumns;x++){
    for (int y=0;y<numlayers;y++){
      ledoff(selrow,x,y);
    }
  } 
}

//cycle through all rows consecutively
void cyclerows(int delaytime){
  for (int z=0;z<numlayers;z++){
    rowon(z);
    delay(delaytime);
    rowoff(z);
    //delay(delaytime);
  }
}

//turn a specified column on
void columnon(int selcolumn){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numlayers;y++){
      ledon(x,selcolumn,y);
    }
  } 
}

//turn a specified column off
void columnoff(int selcolumn){
  for (int x=0;x<numrows;x++){
    for (int y=0;y<numlayers;y++){
      ledoff(x,selcolumn,y);
    }
  } 
}

//cycle through all columns consecutively
void cyclecolumns(int delaytime){
  for (int z=0;z<numcolumns;z++){
    columnon(z);
    delay(delaytime);
    columnoff(z);
    //delay(delaytime);
  }
}


//main loop - visualisation functions should be called here
void loop() {
  int runspeed=200;
  cubeon();
  delay(runspeed*2);
  cubeoff();
  delay(runspeed*2);
  cyclerows(runspeed);
  delay(runspeed);
  cyclecolumns(runspeed);
  delay(runspeed);
  cyclelayers(runspeed);
  delay(runspeed);
  cyclepillars(runspeed);
  delay(runspeed);
  cycleleds(runspeed/2);
  delay(runspeed);
  randomon(runspeed/2);
  delay(runspeed);
  randomoff(runspeed/2);
  delay(runspeed*5);
}
