/*
    This sketch demonstrates how to set up a simple HTTP-like server.
    The server will set a GPIO pin depending on the request
      http://server_ip/gpio/0 will set the GPIO2 low,
      http://server_ip/gpio/1 will set the GPIO2 high
    server_ip is the IP address of the ESP8266 module, will be
    printed to Serial when the module is connected.
*/

#include <ESP8266WiFi.h>

#ifndef STASSID
#define STASSID "5_pyaterochka_n1042_torgovizal"
#define STAPSK "00003333"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;

// Create an instance of the server
// specify the port to listen on as an argument
WiFiServer server(80);

void setup()
{
  Serial.begin(115200);

  // prepare LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println();
  Serial.println(F("WiFi connected"));

  // Start the server
  server.begin();
  Serial.println(F("Server started"));

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.accept();
  if (!client)
  {
    return;
  }
  Serial.println(F("new client"));

  client.setTimeout(5000); // default is 1000

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(F("request: "));
  Serial.println(req);

  // Match the request
  int val;
  if (req.indexOf(F("/gpio/0")) != -1)
  {
    val = 0;
  }
  else if (req.indexOf(F("/gpio/1")) != -1)
  {
    val = 1;
  }
  else
  {
    Serial.println(F("invalid request"));
    val = digitalRead(LED_BUILTIN);
  }

  // Set LED according to the request
  digitalWrite(LED_BUILTIN, val);

  // read/ignore the rest of the request
  // do not client.flush(): it is for output only, see below
  while (client.available())
  {
    // byte by byte is not very efficient
    client.read();
  }

  // Send the response to the client
  // it is OK for multiple small client.print/write,
  // because nagle algorithm will group them into one single packet
  client.print(F("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>"));
  client.println("<head> <style>");
  client.println("a{font-family: Helvetica; display:inline-block; margin: 2px auto;  text-align: center; color:red; text-decoration:none;}");
  client.println(".container {display:block; width:200px; font-family:serif; font-size:100px; color:rgb(256,0,0); text-align:center;margin:auto;}");

  client.println("button{font-size:26px; color:rgb(256,0,0);border 2mm solid rgba(128,64,128,0.5); width:100%; padding:100px 100px 100px 100px; display:block; font-size:66px; text-align:center; ");
  // client.println(".container {display:block; width:200px; font-family:serif; font-size:100px; color:rgb(256,0,0); text-align:center;margin:auto;}" );
  client.println("");
  client.println("");
  client.println("</style></head>");

  client.println("<body>");

  client.println("<div class='container'>");
  client.print((val) ? F("low") : F("high"));
  client.println("</div>");
  client.print(F("<br><br> <button> <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/1'>OFF</a> </button> <button> <a href='http://"));
  client.print(WiFi.localIP());
  client.print(F("/gpio/0'>ON</a> </button> </body></html>"));

  // The client will actually be *flushed* then disconnected
  // when the function returns and 'client' object is destroyed (out-of-scope)
  // flush = ensure written data are received by the other side
  Serial.println(F("Disconnecting from client"));
}
