#define LED_MIN 0
#define LED_HALF 128
#define LED_MAX 255
#define LED_PIN 9
#define ANALOG_IN A0

#define NUM_AVG 20 

int ara[256] = {-1};
int SETTLING_TIME;
int MIN_READING,MAX_READING;


void setup() {
  Serial.begin(9600);

  pinMode(ANALOG_IN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
    /* wait for user input to start*/
  Serial.println("\nPress a key and enter to continue");
  while (Serial.available()<=0);
  while(Serial.available()>0) Serial.read();

  /* delay for 3 seconds */
  delay(3000);

  /* Step 1: Serial Plotter, adjust NUM_AVG for testing */
  //findAvg();
  
  /* Step 2: */
  findSettlingTime();
  
  /* Step 3: */
  findPhotoMinMax(); 
  
  /* Step 4,5,6: */
  fillAra();
  linearGuessLoop();

  /* Step 7: */ 
  testLoop();
}

void findAvg() {
  int total = 0;
  int numReadings = 500;
  while(total < numReadings) {
    getAverage();
    total++;
  }
}

int getAverage() {
  int total = 0;
  
  for(int i = 0; i < NUM_AVG; i++) {
    total += analogRead(ANALOG_IN);   
  }
  int avg = total/NUM_AVG;
  
  return avg;
}

void fillAra() {
  for(int i = 0; i < 256; i++) {
    ara[i] = -1;
  }
}

void findSettlingTime() {
  unsigned long startMillis, endMillis, settlingTime;

  startMillis = millis();
  
  analogWrite(LED_PIN,LED_HALF);
  readLight();
  
  endMillis = millis();
  
  settlingTime = endMillis - startMillis;
  Serial.print("settling time (msec) = ");
  Serial.println(settlingTime);

  SETTLING_TIME = settlingTime;
}

int readLight(){
  int prev = 0;
  int cur = 1;
  
  while(prev != cur) {
    prev = cur;
    cur = getAverage();
  }

  return prev;
}

void findPhotoMinMax() { 
  analogWrite(LED_PIN,LED_MIN);
  delay(SETTLING_TIME);
  MIN_READING = readLight();

  analogWrite(LED_PIN,LED_MAX);
  delay(SETTLING_TIME);
  MAX_READING = readLight(); 
  
  Serial.print("smallest = ");
  Serial.println(MIN_READING);  
  Serial.print("largest = ");
  Serial.println(MAX_READING);
}

void linearGuessLoop() {
  int total, avg;

  Serial.println("\nLinear guess loop:");

  //increment LED brightness from 0 to 255
  for(int i = 0; i < 256; i++) {
    analogWrite(LED_PIN, i);
    //wait for photoresistor to settle
    delay(SETTLING_TIME);

    // take average reading
    avg = getAverage();

    //map reading from the min and max
    int mapReading = map(avg,MIN_READING,MAX_READING,0,255);
    mapReading = constrain(mapReading,0,255);

    //store map reading to array
    ara[mapReading] = i ;

    Serial.print("LED=");
    Serial.print(i);  
    Serial.print("\tLight=");
    Serial.print(avg); 
    Serial.print("\tLinMapped=");
    Serial.println(mapReading);  
  }
  
  // check array for -1 entries
  int lastGood = 0;
  for(int i = 0; i < 256; i++) {
    if(ara[i] == -1) 
      ara[i] = lastGood;
     else
       lastGood = ara[i];
  }
  return ara;
} 

void testLoop() {
  int total, avg;

  Serial.println("\ntest loop");

  //increment LED brightness from 0 to 255
  for(int i = 0; i < 256; i++) {
    analogWrite(LED_PIN, i);
    //wait for photoresistor to settle
    delay(SETTLING_TIME);

    // take average reading
    avg = getAverage();
    
    int mapReading = map(avg,MIN_READING,MAX_READING,0,255);
    mapReading = constrain(mapReading,0,255);
    
    Serial.print("LED=");
    Serial.print(i); 
    Serial.print("\tLight=");
    Serial.print(avg);  
    Serial.print("\tReverseMapped="); 
    Serial.println(ara[mapReading]);
  }
}



