// pin definitions
#define EN_A 9
#define EN_B 4
#define RIGHT_FORWARD 8 // right wheel forward
#define RIGHT_BACKWARD 7 // right wheel reverse
#define LEFT_FORWARD 6 // left wheel forward
#define LEFT_BACKWARD 5 // left wheel reverse


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(EN_A, OUTPUT);
  pinMode(EN_B, OUTPUT);
  pinMode(RIGHT_FORWARD, OUTPUT);
  pinMode(RIGHT_BACKWARD, OUTPUT);
  pinMode(LEFT_FORWARD, OUTPUT);
  pinMode(LEFT_BACKWARD, OUTPUT);
}

void forward(){
  Serial.println("Going forward");
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, HIGH);
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(LEFT_BACKWARD, HIGH);
}
void backward(){
  Serial.println("Going backward");
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(RIGHT_FORWARD, HIGH);
  digitalWrite(RIGHT_BACKWARD, LOW);
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
}
void turnLeft(){
  Serial.println("Turning Left");
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, LOW);
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
}
void turnRight(){
  Serial.println("Turning Right");
  digitalWrite(EN_A, HIGH);
  digitalWrite(EN_B, HIGH);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, HIGH);
  digitalWrite(LEFT_FORWARD, HIGH);
  digitalWrite(LEFT_BACKWARD, LOW);
}
void stop(){
  Serial.println("Stopped");
  digitalWrite(EN_A, LOW);
  digitalWrite(EN_B, LOW);
  digitalWrite(RIGHT_FORWARD, LOW);
  digitalWrite(RIGHT_BACKWARD, LOW);
  digitalWrite(LEFT_FORWARD, LOW);
  digitalWrite(LEFT_BACKWARD, LOW);
}


void loop() {
  // put your main code here, to run repeatedly:
  turnLeft();
  delay(1000);
  stop();
}
