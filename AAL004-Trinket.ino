/****************************************************************************** 
Amplification, offset correction, and temperature-compensation of 
an NVE GMR sensor using an ATtiny85 Trinket board. Sensor OUT- to PB3/A3; 
OUT+ to PB4/A2; PWM output on PB0; digital threshold output to red LED (PB1).
Rev. 2/18/18
******************************************************************************/
int sensor; //Signed integer; 50 mV/V = 512
int sensor_offset=511; //Dummy value to flag for update
int temperature; //deg. K
const int tempco = 4000; //Sensor tempco in ppm/deg. C
const int sensor_threshold = 128; //Sensor digital threshold

void setup() {
  pinMode(1, OUTPUT); //Red LED on PB1
  ADCSRA &= ~(_BV(ADATE) |_BV(ADIE)); //Clear ADC auto trigger and interrupt enable
  ADCSRA |= _BV(ADEN); //Enable ADC
  ADCSRB = _BV(7); //Set ADC to bipolar mode (ADCSRB bit 7 = 1)
}
void loop() {
//Read Sensor
  ADMUX = 0x07 | _BV(REFS2); //Differential input (A2-A3)*20; Vcc reference
  delay(1); //1 ms settling time
  sensor = getADC();
  if (sensor>511) sensor-=1024; //Convert 10-bit 2's complement to signed integer
  if (abs(sensor_offset)>256) sensor_offset=sensor; //Update offset first-time through 
  sensor-=sensor_offset; //Correct for offset
  if (sensor<0) sensor=0; //Limit sensor values to avoid PWM overruns 
  if (sensor>511) sensor=511;
  
//Read temperature
  ADMUX = 0xF | _BV(REFS1); //Enable temp sensor (ADC4); 1.1V reference
  delay(1); //1 ms settling time
  temperature = getADC();

//Temp. compensate Sensor (long integer math uses less memory than floating point)
  sensor = long(sensor)*(long(temperature-300L)*long(tempco)+1000000L)/1000000L;

//PWM and digital outputs
  analogWrite(0, sensor/2); //Output the compensated sensor value (8 bits)
//analogWrite(0, temperature/2); //Output the temperature (512 deg. K = 3.3V)
  digitalWrite(1, sensor>sensor_threshold); //Turn on LED if threshold reached
 }
//ADC subroutine
int getADC() {
  ADCSRA |=_BV(ADSC); //Start conversion
  while((ADCSRA & _BV(ADSC))); //Wait for conversion
  return ADC;
}
