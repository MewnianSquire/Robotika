#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define leftMotorA 5
#define leftMotorB 4
#define rightMotorA 0
#define rightMotorB 2

const char* ssid2 = "RC CAR";
const char* password2 = "";

String header;
bool motorRunning = 0;


const char* html = R""""(
HTTP/1.1 200 OK
Content-type:text/html

<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>RC</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            display: flex;
            flex-direction: column;
            justify-content: flex-start;
            align-items: center;
            height: 100vh;
        }

        .center-line {
            position: absolute;
            background-color: #000;
        }

        .vertical-line {
            left: 50%;
            width: 2px;
            height: 100%;
            transform: translateX(-50%);
        }

        .horizontal-line {
            top: 50%;
            height: 2px;
            width: 100%;
            transform: translateY(-50%);
        }

        .diamond-container {
            display: flex;
            justify-content: center;
            align-items: center;
            width: 100vw; 
            height: 100vh; 
        }

        .diamond-container-inner {
            position: relative;
            width: 200px;
            height: 200px;
        }

        .diamond-button {
            position: absolute;
            width: 150px;
            height: 50px;
            background-color: #ff36d7;
            border: none;
            color: white;
            font-size: 16px;
        }

        .diamond-button:active {
            background-color: #fc90e7;
        }

        .diamond-button-top {
            top: -50px;
            left: 25px;
        }

        .diamond-button-bottom {
            bottom: 50px;
            left: 25px;
        }

        .diamond-button-left {
            top: 25px;
            left: -60px;
        }

        .diamond-button-right {
            top: 25px;
            right: -60px;
        }

        .title-main {
            text-align: center;
            font-family: Arial, sans-serif;
            margin-top: 20px; 
            color: #000000;
            margin-top: 20px; 
            text-shadow: 
                -2px -2px 0 #fff,  
                2px -2px 0 #fff,
                -2px 2px 0 #fff,
                2px 2px 0 #fff;
        }
    </style>
</head>

<body>
    <div class="title-main">
        <h1>Remote Control Car</h1>
    </div>
    <div class="diamond-container">
        <div class="diamond-container-inner">
            <button class="diamond-button diamond-button-top" onmousedown="turnOn(1)" onmouseup="turnOff(1)" ontouchstart="turnOn(1)" ontouchend="turnOff(1)">Maju</button>
            <button class="diamond-button diamond-button-bottom" onmousedown="turnOn(2)" onmouseup="turnOff(2)" ontouchstart="turnOn(2)" ontouchend="turnOff(2)">Mundur</button>
            <button class="diamond-button diamond-button-left" onmousedown="turnOn(3)" onmouseup="turnOff(3)" ontouchstart="turnOn(3)" ontouchend="turnOff(3)">Kiri</button>
            <button class="diamond-button diamond-button-right" onmousedown="turnOn(4)" onmouseup="turnOff(4)" ontouchstart="turnOn(4)" ontouchend="turnOff(4)">Kanan</button>
        </div>
    </div>

    <script>
        function turnOn(buttonId) {
            (buttonId == 1) ? forward() : (buttonId == 2) ? reverse() : (buttonId == 3) ? left() : right();
        }

        function turnOff(buttonId) {
            motorOff();
        }

        function forward(){
            fetch("/motor/forward");
        }

        function reverse(){
            fetch("/motor/reverse");
        }

        function left(){
            fetch("/motor/left");
        }

        function right(){
            fetch("/motor/right");
        }

        function motorOff(){
            fetch("/motor/off")
        }
    </script>
</body>

</html>
)"""";

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  analogWrite(0, 0);
  analogWrite(2, 0);

  //This is to make RC as a Wi-Fi access point, connect your device directly to RC
  Serial.println("Setting AP (Access Point)…");
  WiFi.softAP(ssid2);
  WiFi.softAPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);


  server.begin();
}

void loop() {
  WiFiClient client = server.accept();

  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println(html);
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }


        if (currentLine.endsWith("GET /motor/forward")&& motorRunning == 0) {
          bothForward();
          motorRunning = 1;
          delay(10);
        }
        if (currentLine.endsWith("GET /motor/reverse")&& motorRunning == 0) {
          bothReverse();
          motorRunning = 1;
          delay(10);

        }
        if (currentLine.endsWith("GET /motor/left")&& motorRunning == 0) {
          turnLeft();
          motorRunning = 1;
          delay(10);

        }
        if (currentLine.endsWith("GET /motor/right") && motorRunning == 0) {
          turnRight();
          motorRunning = 1;
          delay(10);
          
        }
        if (currentLine.endsWith("GET /motor/off") && motorRunning == 1) {
          stopMotors();
          motorRunning = 0;
          delay(10);
        }
      }
    }
    client.stop();
  }
}

void bothForward(){
  //Move RC forwards
  analogWrite(leftMotorA, 1023);
  analogWrite(leftMotorB, 0);
  analogWrite(rightMotorA, 1023);
  analogWrite(rightMotorB, 0);

}

void bothReverse(){
  //Move RC backwards
  analogWrite(leftMotorA, 0);
  analogWrite(leftMotorB, 1023);
  analogWrite(rightMotorA, 0);
  analogWrite(rightMotorB, 1023);

}

void turnRight(){
  //Use this to make RC spin in place clockwise
  // analogWrite(leftMotorA, 1023);
  // analogWrite(leftMotorB, 0);
  // analogWrite(rightMotorA, 0);
  // analogWrite(rightMotorB, 1023);

  //Use this to make RC turn right with one motor
  analogWrite(leftMotorA, 1023);
  analogWrite(leftMotorB, 0);
  analogWrite(rightMotorA, 0);
  analogWrite(rightMotorB, 0);
}

void turnLeft(){
  //Use this to make RC spin in place counter-clockwise
  // analogWrite(leftMotorA, 0);
  // analogWrite(leftMotorB, 1023);
  // analogWrite(rightMotorA, 1023);
  // analogWrite(rightMotorB, 0);

  //Use this to make RC turn left with one motor
  analogWrite(leftMotorA, 0);
  analogWrite(leftMotorB, 0);
  analogWrite(rightMotorA, 1023);
  analogWrite(rightMotorB, 0);
}

void stopMotors(){
  //This will make RC stand still
  analogWrite(leftMotorA, 0);
  analogWrite(leftMotorB, 0);
  analogWrite(rightMotorA, 0);
  analogWrite(rightMotorB, 0);
}
