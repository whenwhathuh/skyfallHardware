const int switchPin = 5;
const int ledPin = 4;
int switchState = 0;

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
}//end setup()

void loop() {
  switchState = digitalRead(switchPin);

  if(switchState == HIGH){
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }

}//end loop()
