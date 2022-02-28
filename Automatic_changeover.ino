void setup(){
  pinMode(13,OUTPUT);
  }

  void loop(){
 for(int i=1; i<=10;i++){
  digitalWrite(13, HIGH);
  delay(300);
  digitalWrite(13,LOW);
  delay(300);   
 }
 for(int i=1; i<=5;i++){
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13,LOW);
  delay(100);   
 }
 for(int i=1; i<=10;i++){
  digitalWrite(13, HIGH);
  delay(50);
  digitalWrite(13,LOW);
  delay(50);   
 }
 for(int i=1; i<=8;i++){
  digitalWrite(13, HIGH);
  delay(100);
  digitalWrite(13,LOW);
  delay(30);   
 }
 for(int i=1; i<=7;i++){
  digitalWrite(13, HIGH);
  delay(200);
  digitalWrite(13,LOW);
  delay(50);   
 }
 int k=1;
 do{
 for (int i=0; i<=255; i++){
  analogWrite(5,i);
  delay(10);
 }
 for (int i=255; i>=0; i--){
  analogWrite(5,i);
  delay(10);
 }
 k++;
 }while(k<=5);
 }
