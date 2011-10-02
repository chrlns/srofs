#include <SROFS.h>

SROFS srofs = SROFS();

void setup() {
  Serial.begin(9600);
  if(srofs.begin()) {
    Serial.println("SDCard initialized.");
  } else {
    Serial.println("Error initializing SDCard!");
  }
  delay(1000);
}

void loop() {
  // Read file "index.html" from SD Card
  SROFS_File file;
  if(srofs.open("index.html", &file)) {
    Serial.println("index.html opened");
    Serial.println(file.idx); 
  } else {
    Serial.println("Error opening index.html");
  }
  delay(1000);
  
  char c;
  while(file.read((uint8_t*)&c, 1) != -1) {
    Serial.print(c);
    delay(100); 
  }
  
  file.close();
}
