#include <ESP8266WiFi.h>
#include <DHT.h>
#include <Servo.h>
// #include <SoftwareSerial.h>

#define GAS_SENSOR A0
#define DHTPIN D7  
#define DHTTYPE DHT11
#define TRIG D3    
#define ECHO D2    
#define SERVO_PIN D8 

#define IN1 D6
#define IN2 D5
#define IN3 D1
#define IN4 D0

#define LED5 D4
int state5 = 1;

DHT dht(DHTPIN, DHTTYPE);
Servo myServo;
WiFiServer server(80);
// SoftwareSerial Arduino(D0, D1); // NodeMCU TX=D0, RX=D1

void setup() {
    Serial.begin(115200);
    // Arduino.begin(9600);
    
    WiFi.softAP("Multipurpose_Rover", "12345678");
    server.begin();
  
    dht.begin();
    myServo.attach(SERVO_PIN);
  
    pinMode(GAS_SENSOR, INPUT);
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
  
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(LED5, OUTPUT);
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        if (state5) {
            digitalWrite(LED5, LOW);
            delay(50);
            digitalWrite(LED5, HIGH);
            delay(50);

        }

        String request = client.readStringUntil('\r');
        client.flush();

        if (request.indexOf("/F=") != -1) {
            Serial.print("F");
            forward();
        }
        if (request.indexOf("/L=") != -1) {
            Serial.print("L");
            left();
        }
        if (request.indexOf("/S=") != -1) {
            Serial.print("S");
            stopMotors();
        }
        if (request.indexOf("/R=") != -1) {
            Serial.print("R");
            right();
        }
        if (request.indexOf("/B=") != -1) {
            Serial.print("B");
            backward();
        }

        if (request.indexOf("/led5=") != -1) {
            state5 = request.substring(request.indexOf("=") + 1).toInt();
            // Serial.println("Led5=" + String(state5));
            digitalWrite(LED5, state5);
        }

        if (request.indexOf("/servo=") != -1) {
            int pos = request.substring(request.indexOf("=") + 1).toInt();
            myServo.write(pos);
            // Serial.println("Servo=" + String(pos));
        }
        
        if (request.indexOf("/data") != -1) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/plain");
            client.println();
            client.println(String(dht.readTemperature()) + "," +
                           String(dht.readHumidity()) + "," +
                           String(getDistance()) + "," +
                           String(map(analogRead(GAS_SENSOR), 0, 1023, 0, 100)));
            return;
        }

        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: text/html");
        client.println();

        client.println("<style>");
        client.println("body { font-family: Arial, Helvetica, sans-serif; text-align: center; background-color: #f4f4f4; padding: 20px; }");
        client.println("h1 { color: #333; font-size: 64px; }");
        client.println("hr { width: 100%; border: 1px solid #007bff; }");
        client.println("p { font-size: 40px; }");
        client.println("#info { font-size: 40px; margin-bottom: 10px; }");
        client.println("button {");
        client.println("  background-color: #007bff; color: white; border: none; padding: 10px 20px;");
        client.println("  margin: 10px; font-size: 28px; border-radius: 5px; cursor: pointer;");
        client.println("  width: 90%; max-width: 280px; height: 70px; font-weight: bold");
        client.println("}");
        client.println("#controls { height: 150px; font-size: 32px; border-radius: 10px; }");
        client.println("#stopBut { height: 150px; width: 200; font-size: 32px; border-radius: 10px; }");
        client.println("button:hover { background-color: #0056b3; }");
        client.println("input[type='range'] { width: 80%; margin: 10px; }");
        client.println("@media (max-width: 600px) { button { font-size: 28px; padding: 8px 15px; } }");
        client.println(".footer {");
        client.println("  font-family:'Lucida Console', 'Courier New', monospace; font-size: 18px; position: fixed; left: 0; bottom: 0;");
        client.println("  width: 100%; background-color: #777; color: white; text-align: center;");
        client.println("}");
        client.println("</style>");

        client.println("<h1>Multi-purpose Rover</h1>");
        client.println("<hr>");

        client.println("<p id='info'>");
        client.println("<i>Temperature</i> : <span id='temp'></span> C<br>");
        client.println("<i>Humidity</i> : <span id='hum'></span> %<br>");
        client.println("<i>Distance</i> : <span id='dist'></span> cm<br>");
        client.println("<i>Gas Level %</i> : <span id='gas'></span>");
        client.println("</p>");
        
        client.println("<p>");
        client.println("<button onclick='updateLed5(1)'>LED5 ON</button>   <button onclick='updateLed5(0)'>LED5 OFF</button>");
        client.println("</p>");
        client.println("<p style='text-size: 40px;'>");
        client.println("Servo Angle: <input type='range' min='0' max='180' onchange='updateServo(this.value)'><br>");
        client.println("</p>");

        client.println("<p>");
        client.println("<button id='controls' onclick='updateF()'>FORWARD</button><br>");
        client.println("<button id='controls' onclick='updateL()'>LEFT</button>  <button id='stopBut' onclick='updateS()'>STOP</button>  <button id='controls' onclick='updateR()'>RIGHT</button><br>");
        client.println("<button id='controls' onclick='updateB()'>BACKWARD</button><br>");
        client.println("</p>");
        // client.println("<script>function updateServo(angle){fetch('/servo='+angle);}</script>");
        // client.println("<script>function updateSpeed(speed){fetch('/speed='+speed);}</script>");

        client.println("<script>");
        client.println("function updateF(){fetch('/F=');}");
        client.println("function updateL(){fetch('/L=');}");
        client.println("function updateS(){fetch('/S=');}");
        client.println("function updateR(){fetch('/R=');}");
        client.println("function updateB(){fetch('/B=');}");
        client.println("function updateLed5(state){fetch('/led5='+state);}");
        client.println("function updateServo(angle){fetch('/servo='+angle);}");
        client.println("function updateData() {");
        client.println("fetch('/data').then(res => res.text()).then(data => {");
        client.println("let values = data.split(',');");
        client.println("document.getElementById('temp').innerText = values[0];");
        client.println("document.getElementById('hum').innerText = values[1];");
        client.println("document.getElementById('dist').innerText = values[2];");
        client.println("document.getElementById('gas').innerText = values[3];");
        client.println("});");
        client.println("}");
        client.println("setInterval(updateData, 2000);");  // Update every 2 second
        client.println("</script>");
        client.println("<div class='footer'>COPYRIGHT &copy 2025 SS</div>");
    }
}

float getDistance() {
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    return pulseIn(ECHO, HIGH) * 0.034 / 2;
}

void forward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void backward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void left() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
}

void right() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
}

void stopMotors() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}
