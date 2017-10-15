/*
 * an Arduino program that recognizes IR remote transmissions 
 *   - measures the timing of the address and command code sequence against their complement  
 *   - prints the code sequence in hexadecimal to the serial monitor
 *   - filters repeat codes
 */

#define IR_INPUT 8
#define FALSE 0
#define CMD_SEQ 1
#define REP_SEQ 2
#define BYTE_SIZE 8

void setup() {
  Serial.begin(9600);
  pinMode(IR_INPUT, INPUT);
  
  Serial.println("please press a button on the remote");
  Serial.println("===================================");
  
  timer1Init(); 
}

void timer1Init()
{
  TCCR1A = 0x00;
  TCCR1B = 0x05; // 16 Mhz / 1024 = 15.625 kHz --> 64 usec per count
  TCCR1C = 0x00;
  TIMSK1 = 0x00; // not using interrupt
  TCNT1 = 0x00; // initial count of 0
}

void loop() {
  int address = 0x00;
  int cmd = 0x00;
  int addressComplement = 0x00;
  int cmdComplement = 0x00; 
  
  int sequence = startSequenceFound();
  
  if(sequence == CMD_SEQ) 
  {   
        address = readByte(address);  
        addressComplement = readByte(addressComplement);
        cmd = readByte(cmd);
        cmdComplement = readByte(cmdComplement);
        
        Serial.print("address: 0x");
        Serial.println(address, HEX);
        if(address + addressComplement != 255)
        {   
            Serial.println("Warning!: check byte is not the inverse of the address");
            Serial.print("complement: 0x");
            Serial.println(addressComplement, HEX);
        }
        
        Serial.print("cmd: 0x");
        Serial.println(cmd, HEX);        
        if(cmd + cmdComplement != 255)
        {   
            Serial.println("Warning!: check byte is not the inverse of the command");
            Serial.print("complement: 0x");
            Serial.println(cmdComplement, HEX);
        }

        Serial.println("========================");
        delay(200);
  }
  else if(sequence == REP_SEQ)
  {
    Serial.println("REPEAT sequence recieved");
    Serial.println("========================");  
  }
}

int startSequenceFound() { 
  int countActiveLength = 0;
  int countInactiveLength = 0;
  
  if(digitalRead(IR_INPUT) == LOW)
  {     
    /* measure the active length*/
    TCNT1 = 0;  
    while(digitalRead(IR_INPUT)!= HIGH){}
    countActiveLength = TCNT1;
    TCNT1 = 0;

    if(countActiveLength > 120 && countActiveLength < 160) // 9 msec = 140 counts
    {  
      
      /* measure the inactive length */
      while(digitalRead(IR_INPUT)!= LOW){} 
      countInactiveLength = TCNT1;
   
      if(countInactiveLength > 65 && countInactiveLength < 85) // 4.5 msec = 70 counts
      {
          return CMD_SEQ;
      }
      else if (countInactiveLength > 25 && countInactiveLength < 45 ) // 2.25 msec = 35 counts
      {
          return REP_SEQ;
      }
      else
      {
        Serial.println("start signal NOT recieved");
      }
    }
  }
  
  return FALSE;
}


int readByte(int code)
{
    int abit;
    
    for(int i = 0; i < BYTE_SIZE; i++)
    {
      abit = readBit();
      if(abit != -1)
        code = code | (abit << i);    
    }
    
    return code;
}

int readBit()
{  
    int bitLength = 0;

    // wait until bit goes HIGH
    while(digitalRead(IR_INPUT) != HIGH){}
    TCNT1 = 0;

    // wait until bit goes LOW
    while(digitalRead(IR_INPUT)!= LOW){}
    bitLength = TCNT1;

    if(bitLength > 6 && bitLength < 12) // 562.5 usec = 9 counts 
      return 0; 
 
    else if(bitLength > 20 && bitLength < 35) // 1687.5 usec = 26 counts
      return 1;
      
    else
      return -1;
}

