#include <WiFi.h>
#include <WebServer.h>
#include <esp_wifi.h>

const char* ssid = "PhoneDetector";
const char* password = "12345678";

WebServer server(80);

int metalPin = 4;

#define RSSI_THRESHOLD -60
#define MAX_DEVICES 30
#define PHONE_DISTANCE 0.50

bool metalDetected = false;
bool phoneDetected = false;

int probability = 0;

int deviceCount = 0;
float closestDistance = 100;

int currentChannel = 1;

// storage for hotspot names
String detectedSSID[MAX_DEVICES];
float detectedDistance[MAX_DEVICES];
int ssidCount = 0;

// -------- DISTANCE ESTIMATION --------
float estimateDistance(int rssi)
{
  float txPower = -40;
  return pow(10, (txPower - rssi) / 20.0);
}

// -------- WIFI PACKET SNIFFER --------
void sniffer(void* buf, wifi_promiscuous_pkt_type_t type)
{
  if(type != WIFI_PKT_MGMT) return;

  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*)buf;

  int rssi = pkt->rx_ctrl.rssi;

  if(rssi < RSSI_THRESHOLD) return;

  float distance = estimateDistance(rssi);

  deviceCount++;

  if(distance < closestDistance)
  closestDistance = distance;
}

// -------- CHANNEL SWITCH --------
void changeChannel()
{
  currentChannel++;

  if(currentChannel > 13)
    currentChannel = 1;

  esp_wifi_set_channel(currentChannel, WIFI_SECOND_CHAN_NONE);
}

// -------- WIFI HOTSPOT SCAN --------
void scanWiFiHotspots()
{
  ssidCount = 0;

  int n = WiFi.scanNetworks();

  for (int i = 0; i < n; i++)
  {
    String name = WiFi.SSID(i);
    int rssi = WiFi.RSSI(i);

    if(name == "") continue;
    if(rssi < RSSI_THRESHOLD) continue;

    float distance = estimateDistance(rssi);

    if(ssidCount < MAX_DEVICES)
    {
      detectedSSID[ssidCount] = name;
      detectedDistance[ssidCount] = distance;
      ssidCount++;
    }
  }
}

// -------- WEB PAGE --------
void handleRoot() {

String page = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>Phone Detection System</title>

<style>
body{
font-family: Arial;
text-align:center;
background:#111;
color:white;
}

.box{
width:300px;
margin:auto;
padding:20px;
border-radius:10px;
font-size:22px;
}

.red{background:#ff3b3b;}
.yellow{background:#ffc107;color:black;}
.green{background:#4caf50;}
</style>

<script>

function updateData(){

fetch("/data")
.then(response => response.json())
.then(data => {

document.getElementById("metal").innerHTML = data.metal;
document.getElementById("wifi").innerHTML = data.wifi;
document.getElementById("prob").innerHTML = data.prob + "%";

document.getElementById("devices").innerHTML = data.devices;

let box = document.getElementById("box");

box.className="box";

if(data.prob==80) box.classList.add("red");
else if(data.prob>=30) box.classList.add("yellow");
else box.classList.add("green");

});

}

setInterval(updateData,1000);

</script>

</head>

<body onload="updateData()">

<h1>Phone Detection System</h1>

<p>Metal detected: <span id="metal">--</span></p>
<p>WiFi detected: <span id="wifi">--</span></p>

<h3>Detected WiFi / Hotspots</h3>
<div id="devices">None</div>

<div id="box" class="box green">
Probability: <span id="prob">0</span>
</div>

</body>
</html>
)rawliteral";

server.send(200,"text/html",page);
}

// -------- DATA API --------
void handleData(){

String deviceList="";

for(int i=0;i<ssidCount;i++)
{
  deviceList += detectedSSID[i];
  deviceList += " (";
  deviceList += String(detectedDistance[i],2);
  deviceList += " m)";
  deviceList += "<br>";
}

if(deviceList=="") deviceList="None";

String json="{";
json+="\"metal\":\""+String(metalDetected?"YES":"NO")+"\",";
json+="\"wifi\":\""+String(phoneDetected?"YES":"NO")+"\",";
json+="\"prob\":"+String(probability)+",";
json+="\"devices\":\""+deviceList+"\"";
json+="}";

server.send(200,"application/json",json);
}

void setup(){

Serial.begin(115200);

pinMode(metalPin,INPUT);

WiFi.softAP(ssid,password);

Serial.println("Hotspot Started");
Serial.println(WiFi.softAPIP());

server.on("/",handleRoot);
server.on("/data",handleData);

server.begin();

// WIFI SNIFFER
WiFi.mode(WIFI_AP_STA);
WiFi.disconnect();

esp_wifi_set_promiscuous(true);
esp_wifi_set_promiscuous_rx_cb(&sniffer);
}

void loop(){

server.handleClient();

// -------- METAL DETECTION --------
metalDetected = digitalRead(metalPin);

// -------- WIFI DEVICE SCAN --------
deviceCount = 0;
closestDistance = 100;

for(int i=0;i<13;i++)
{
  changeChannel();
  delay(40);
}

scanWiFiHotspots();

if(deviceCount > 0){
  phoneDetected = true;
  Serial.print("WiFi Devices: ");
  Serial.println(deviceCount);
}
else
  phoneDetected = false;

// -------- PROBABILITY --------

if(metalDetected && closestDistance <= PHONE_DISTANCE)
{
  probability = 80;
}

else if(phoneDetected)
{
  probability = 50;
}

else if(metalDetected)
{
  probability = 30;
}

else
{
  probability = 0;
}



}