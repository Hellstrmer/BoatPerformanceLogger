#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GC9A01A.h>
#include <SPI.h>
#include <Preferences.h>
#include <esp_now.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPASyncWebServer.h>
#include <DNSServer.h>

// Forward declarations
String getIndexHTML();

AsyncWebServer server(80);
DNSServer dnsServer;

uint8_t senderMAC[] = {0x88, 0x57, 0x21, 0x21, 0x8E, 0xEC};

struct SensorData {
  float posMM;
};

struct ConfigData {
  float lowerColorMax;
  float upperColorMin;
  bool resetPosition;
};
// Screen defination variables
#define TFT_CS 15
#define TFT_DC 18
#define TFT_RST 14

float lastPos = 0;
const float maxValue = 154.20;

// Prefs memory variables
Preferences prefs;
const long SaveInterval = 5000;
const char *prefsMinColor = "min";
const char *prefsMaxColor = "max";

// Calibration variables
float LowerColorMaxValue = 0;
float UpperColorMinValue = 999;

// Screen declaration
Adafruit_GC9A01A tft(TFT_CS, TFT_DC, TFT_RST);

// ScreenGlobalVariables
int centerX = tft.width() / 2;
int centerY = tft.height() / 2;
int activeColor = 0;
int height = 120;

String Textes[4] = {"HYDROLIFT", "By", "Pluto", "Engineering"};
int TextesPos[4] = {30, 80, 120, 150};
// Delay on textupdate for smoother transitions
const long textUpdateInterval = 100;


volatile SensorData latestData;
volatile bool newDataAvailable = false;

void onDataReceived(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  memcpy((void*)&latestData, data, sizeof(SensorData));
  newDataAvailable = true;
}

void onDataSent(const uint8_t *mac, esp_now_send_status_t status) {
//
}

void sendConfig(float lower, float upper, bool reset = false) {
  ConfigData config = { lower, upper, reset };
  esp_now_send(senderMAC, (uint8_t*)&config, sizeof(config));
}

void initWebServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", getIndexHTML());
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String json = "{\"posMM\":" + String(latestData.posMM, 1) + 
                  ",\"lower\":" + String(LowerColorMaxValue, 1) +
                  ",\"upper\":" + String(UpperColorMinValue, 1) + "}";
        request->send(200, "application/json", json);
  });
  server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("lower", true))
          LowerColorMaxValue = request->getParam("lower", true)->value().toFloat();
        if (request->hasParam("upper", true))
          UpperColorMinValue = request->getParam("upper", true)->value().toFloat();

          sendConfig(LowerColorMaxValue, UpperColorMinValue, false);
          request->send(200, "text/plain", "OK");
  });
  server.on("/ping", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "pong");
  });
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("http://192.168.4.1");
  });
  // Android captive portal detection
  server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("http://192.168.4.1/");
  });
  server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest *request){
    request->redirect("http://192.168.4.1/");
  });
  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
  sendConfig(LowerColorMaxValue, UpperColorMinValue, true);
  request->send(200, "text/plain", "OK");
});
  server.begin();
}

void initESPNow() {
  // WiFi måste sättas innan ESPNow
  WiFi.mode(WIFI_AP_STA);  
  WiFi.softAP("Hydrolift", "Hydrolift");
  dnsServer.start(53, "*", WiFi.softAPIP());
  
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESPNow init failed");
    return;
  }
  
  esp_now_register_recv_cb(onDataReceived);
  
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, senderMAC, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);
}

String getIndexHTML() {
  return R"rawhtml(
<!DOCTYPE html>
<html lang="sv">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Hydrolift</title>
  <style>
    *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
    :root {
      --bg: #111111;
      --surface: #1a1a1a;
      --border: #2a2a2a;
      --text: #f0f0f0;
      --muted: #666;
      --green: #3a9e6a;
      --yellow: #c9960a;
      --red: #c0392b;
    }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, 'Helvetica Neue', Arial, sans-serif;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      padding-bottom: 48px;
    }
    .header {
      width: 100%;
      background: #000;
      padding: 16px 24px;
      display: flex;
      align-items: center;
      justify-content: space-between;
      border-bottom: 1px solid var(--border);
    }
    .logo { height: 36px; width: auto; }
    .header-title {
      font-size: 11px;
      letter-spacing: 3px;
      text-transform: uppercase;
      color: var(--text);
      font-weight: 400;
    }
    .container { width: 100%; max-width: 400px; padding: 24px 20px 0; }
    .pos-card {
      background: var(--surface);
      border: 1px solid var(--border);
      padding: 24px;
      margin-bottom: 12px;
      border-radius: 8px;
    }
    .pos-label {
      font-size: 10px;
      letter-spacing: 2px;
      text-transform: uppercase;
      color: var(--muted);
      margin-bottom: 6px;
    }
    .pos-value {
      font-size: 64px;
      font-weight: 300;
      line-height: 1;
      letter-spacing: -2px;
      display: flex;
      align-items: baseline;
      gap: 6px;
    }
    .pos-value .unit { font-size: 18px; color: var(--muted); font-weight: 400; letter-spacing: 0; }
    .status-bar {
      margin-top: 20px; height: 50px;
      background: var(--border); border-radius: 2px; overflow: hidden;
    }
    .status-fill {
      height: 100%; width: 0%; border-radius: 2px;
      transition: width 0.4s ease, background 0.4s ease;
      background: var(--green);
    }
    .status-labels {
      display: flex; justify-content: space-between; margin-top: 8px;
    }
    .status-label { font-size: 10px; letter-spacing: 1px; text-transform: uppercase; color: var(--muted); }
    .section {
      background: var(--surface); border: 1px solid var(--border);
      border-radius: 8px; padding: 20px 24px; margin-bottom: 12px;
    }
    .section-title {
      font-size: 10px; letter-spacing: 2px; text-transform: uppercase;
      color: var(--muted); margin-bottom: 16px;
    }
    .field { margin-bottom: 12px; }
    .field:last-child { margin-bottom: 0; }
    .field-label {
      font-size: 11px; color: var(--muted); letter-spacing: 1px;
      text-transform: uppercase; margin-bottom: 6px;
      display: flex; align-items: center; gap: 6px;
    }
    .swatch { width: 8px; height: 8px; border-radius: 50%; }
    .field-input {
      width: 100%; background: var(--bg); border: 1px solid var(--border);
      border-radius: 6px; color: var(--text); font-size: 15px;
      padding: 10px 12px; outline: none; transition: border-color 0.15s;
      -webkit-appearance: none; font-family: inherit;
    }
    .field-input:focus { border-color: #555; }
    .field-input::placeholder { color: var(--muted); }
    .btn {
      width: 100%; padding: 13px; background: #fff;
      border: none; border-radius: 6px; color: #111;
      font-size: 11px; font-weight: 600; letter-spacing: 2px;
      text-transform: uppercase; cursor: pointer; font-family: inherit;
      transition: opacity 0.15s;
    }
    .btn:active { opacity: 0.85; }
    .toast {
      position: fixed; bottom: 24px; left: 50%;
      transform: translateX(-50%) translateY(60px);
      background: #fff; color: #111;
      padding: 10px 20px; border-radius: 2px;
      font-size: 11px; letter-spacing: 2px; text-transform: uppercase;
      transition: transform 0.25s ease; z-index: 100;
    }
    .toast.show { transform: translateX(-50%) translateY(0); }
  </style>
</head>
<body>
  <div class="header">
    <img src="data:image/jpeg;base64,/9j/4AAQSkZJRgABAQAAAQABAAD/2wBDACgcHiMeGSgjISMtKygwPGRBPDc3PHtYXUlkkYCZlo+AjIqgtObDoKrarYqMyP/L2u71////m8H////6/+b9//j/2wBDASstLTw1PHZBQXb4pYyl+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj4+Pj/wAARCABYAKADASIAAhEBAxEB/8QAGgAAAgMBAQAAAAAAAAAAAAAAAAUCAwQBBv/EADkQAAIBAwIDBQMLBAMBAAAAAAECAwAEEQUhEhMxFCJBUXEyYcEGFSMzNDVSYnKBsUJUkaEkgpLx/8QAFAEBAAAAAAAAAAAAAAAAAAAAAP/EABQRAQAAAAAAAAAAAAAAAAAAAAD/2gAMAwEAAhEDEQA/AEtFFFAUUUUBRRRQFFFFAUUUUBRWq30+4uYTLEmVGwycZ9KoeKSJ+CRGVvIigc2WkQNDHLKzOWUNjoKhq2nIkQmt0ChRhlHl50xszyrCLm9whRni2xVM2rWykquZc7YA2NAjsrY3VykQ6Hdj5CvSPZWzqFaBCAMDbFLYdRghJMVkVzjcDrWqLV7Zzh+KM/mFAo1S2jtbvgiBClQcE5rFTrVrSW6lSa3AkXh4e6ffXbTRABxXTZP4FPxoElFbdQsHs5MjLRN7LfA1ioCirY7aeVeKOJ2XpkLmp9iuv7eX/wAmgz0VJ0eNuF1ZT5EYrqRvISI0ZyPBRmghRUnRkYq6lWHgRg1GgKKsihkmJEUbPjrwjNRdGjcq6lWHUGgjU4ghlQSEhMjiI8q7FDLNnlRs+OvCM4rnKk5vK4G5mccON6D10SosSrGAEA7uOmKpvLiK2i5kgBI9keOaUW82pW8QjSByo6cSE4qq4W/uZRJJA5IGAODag7LLPeTAMC7Z2iXw38at7CIQO03UcH5F3PurOLi6souUI+SW/qK4Y/vWVUkmfCqzsfIZNAwCWGwF9KCMYPDt/FdNg7Lx28sdyg6rnB65rIdPuwM9nf8AxVKtJDJlSyOv7EUGq3uJrWQ8slSPajb/AH6U/s7uO7i402I6r5UpjdNVj4HCrdoMq3g/uNUW07Ws4lAIweGVdz+5oN+uXQSIW64LPu3uFIac3+mTXNwZ4CrK4B3PSsFxp9zbxmSRMKPEEGgcaH9g/wC5rJ8+yht4UI9TWvQ/sH/c1X83aaDkz59ZBQXXix32mGXhweDjUnqKwaB9pl/R8a0ahfwR2pt7ZgxI4e70UVn0D7TL+j40FGsfeUn7fwKw1u1j7yl/b+Kw0Dj5P/WTfpFYtU+8ZvX4Ctvyf+sm/SKxap94zeo/gUG35P8Atz+g+NZruY2+rySqASr5wfStPyf9uf0HxrFqf3hN+qgY2ery3F1HE0aAMcZGav1PUHsmjCIrcQJ3pPpf3jD6/Ctvyg+sg9DQY7m6k1GaJSihvZGPfTljBpNkMLljt72NJNNx84QZ/FTD5QZxB5d74UFa67Nx5aJCvkM5rZewRahZdoiHfC8SnxPuNedr0ei5+bxnpxHFB5+ORopFkQ4ZTkUx1FVeaOZdkuEzjBO/oKWNjiOOmaZz5OmWPnlsden7UDLSJTJYqD1Q8NXX8fNspl/KT/jel2mTdn0+4lxngbIHSl11f3F0SHfCfhXYUDjRPsB/Wa88epq2O5niXhjldV64BxVVBymugfaZP0fGlVWRTSQkmJ2QnYlTigc32lS3V28qyIA2NjnyrM+iTIjMZY8AZ8ax9uuv7iX/ANGg3t0QQbiQg/moNOi3CQ3ZVzgSDAJ8626hpL3FwZoXUFuoakNaI766jXhSdwB4ZzQPNPsxp8MjyyLlt2PgAKQ3UomuZJB0ZiR6UTXU84xLKzDyJ2qmg16X94w+vwrb8oPrIPQ0pR2jcOjFWHQipSzSzEGWRnx04jmg5G5jkV19pSCK9Ewg1azGGwevvU15qpxyPE3FG7KfMHFAyXQpuPDSoE8xkmtt7PFp9lyIz3+HhUePqaTnULsjHaH/AM1n70j+LMf3JoBFZ3VFGWY4ApnfssckMAIxbx77kZJ9KIIV02MTzgG4YfRx+XvNURwyXN1y8kuxzIfKga6VAPm7Ei5EmSQfGl+o6WbcGaE5iHUHqtOUntkAjWaPu7Y4hWDXLleQkKMDxnJwfAUGKHu2MbK8CEu2TIoOenuNQtu/qS8fLk6+yvdO3liiCWI26xymHCsSA6sTv6V1TCk4lSeFSP6Qj46YoO8lAk0qAGJ4iyZ34TkZHqKgp5FrE6KvMlYjiYA4AxtvUo2SOGSFbuMpJ1BRtvTaiIwpGY2uIpEJzwsjbHzGKCu8VgI3dIwWBBaMghsenStAVF1eQctSqhjwkbezVUximCr2iJET2VWNsCpF4zctP2mLiYEEcDY3GKAMCJDcOg4o2jDRk9R3hkeo6VKPu2UJV4EJLZMigk7+hqCMiW8kAu4yj4zlG29NqkrwCFY2lgkCZwWR87+lBC17+o98xts24UcPsnwxUblmMeDLbuM9I1AP8CpoYY7jmpPCNiOHgfHTFEnZnXAeBD5qj5/3QXIjmK2WFrdSyZKuoJJyfdVMQhfVcRqOWScAjbp5etRflvy/+Wg5a8KkI3nn41PmR9q7R2mIP4/RtgnGM0EbIRG3n5yjhJVeLG65zvUbiEw2qKygOJGBPnsP9V1ViWJ4xdR4cgnuN4ft76lIySwRxPdxkR5weBs/xQSkLQ3HZ4VhCoBnmBe9tvkmqeN7S/LIqxMCcAnIXP8A9q4SQkLzJYJGUYDMj5x78dap5kT3pkuGMidSUGM7bUE4xNdT/R8UkjDvO3h5j0pvFY9lsZFiP0zKe976q0/UrVm5KxiD8O+xplLIkUZeRgqjqTQePrlW3LI9xI0QIQsSM1VQFFFFAUUUUBRRRQFFFFAUUUUBRRRQFFFFAUUUUBVsk8sqqskjMq9AT0oooKqKKKD/2Q==" class="logo" alt="Pluto Engineering">
    <span class="header-title">Hydrolift</span>
  </div>
  <div class="container">
    <div class="pos-card">
      <div class="pos-label">Aktuell höjd</div>
      <div class="pos-value"><span id="pos">---</span><span class="unit">mm</span></div>
      <div class="status-bar"><div class="status-fill" id="fill"></div></div>
      <div class="status-labels">
        <span class="status-label" id="lower-ind">--</span>
        <span class="status-label" id="upper-ind">--</span>
      </div>
    </div>
    <div class="section">
      <div class="section-title">Kalibrering</div>
      <div class="field">
        <div class="field-label"><div class="swatch" style="background:#c9960a"></div>Undre gräns</div>
        <input class="field-input" type="number" id="lower" step="0.1" inputmode="decimal" placeholder="mm">
      </div>
      <div class="field">
        <div class="field-label"><div class="swatch" style="background:#c0392b"></div>Övre gräns</div>
        <input class="field-input" type="number" id="upper" step="0.1" inputmode="decimal" placeholder="mm">
      </div>
    </div>
    <button class="btn" onclick="saveConfig()">Spara</button>
    <button class="btn" style="margin-top:100px; background:#1a1a1a; color:#666; border:1px solid var(--border);" onclick="resetPos()">Nollställ position</button>
  </div>
  <div class="toast" id="toast">Sparat</div>
  <script>
    const MAX = 154.2;
    function update(d) {
      document.getElementById('pos').textContent = d.posMM.toFixed(1);
      document.getElementById('lower-ind').textContent = 0 + ' mm';
      document.getElementById('upper-ind').textContent = 154 + ' mm';
      const fill = document.getElementById('fill');
      fill.style.width = Math.min(100, Math.max(0, d.posMM / MAX * 100)) + '%';
      fill.style.background = d.posMM > d.upper ? '#c0392b' : d.posMM < d.lower ? '#c9960a' : '#3a9e6a';
      document.getElementById('lower').placeholder = d.lower.toFixed(1) + ' mm';
      document.getElementById('upper').placeholder = d.upper.toFixed(1)  + ' mm';
    }
    setInterval(() => { fetch('/status').then(r=>r.json()).then(update).catch(()=>{}); }, 200);
    function saveConfig() {
      const body = new FormData();
      const l = document.getElementById('lower').value;
      const u = document.getElementById('upper').value;
      if (l) body.append('lower', l);
      if (u) body.append('upper', u);
      fetch('/config', {method:'POST', body}).then(() => {
        const t = document.getElementById('toast');
        t.classList.add('show');
        setTimeout(() => t.classList.remove('show'), 2000);
        document.getElementById('lower').value = '';
        document.getElementById('upper').value = '';
      });      
    }
      function resetPos() {
      fetch('/reset', {method:'POST'}).then(() => {
        const t = document.getElementById('toast');
        t.textContent = 'NOLLSTÄLLD';
        t.classList.add('show');
        setTimeout(() => { t.classList.remove('show'); t.textContent = 'Sparat'; }, 2000);
      });
    }
  </script>
</body>
</html>
  )rawhtml";
}


void UpdateText(float pos, unsigned long &lastUpdate)
{
  if (millis() - lastUpdate > textUpdateInterval)
  {
    const int Text_Padding_X = 40;
    const int text_Padding_Y = 4;
    // Update the text on the lower part of the screen
    tft.setTextColor(GC9A01A_WHITE);
    tft.setTextSize(3);

    // Calculate the text area and place it at the right place on the screen
    String valueStr = String(pos, 0);
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(valueStr, 0, 0, &x1, &y1, &w, &h);
    int valueX = centerX - w / 2;
    int valueY = centerY - h / 2 + 80;

    // Clear only the text area
    tft.fillRect(valueX - Text_Padding_X, valueY, w + 60, h + text_Padding_Y, GC9A01A_BLACK);
    // Print the position
    tft.setCursor(valueX, valueY);
    tft.print(valueStr);
    // Print the helptext
    tft.setTextSize(2);
    tft.setCursor(centerX - 15, valueY + 30);
    tft.print("mm");
    lastUpdate = millis();
  }
}

void EmptyMiddleScreen()
{
  // Empty middle part of screen
  int x = -10;
  int y = centerY - (height / 2);
  tft.fillRect(x, y, 250, height, GC9A01A_BLACK);
}

void drawBoxes(float Pos)
{
  const float min = 0;
  // Draw the box for showing lift
  int color = GC9A01A_GREEN;

  // Check the position against the calibrated values
  if (Pos > UpperColorMinValue)
  {
    color = GC9A01A_RED;
  }
  else if (Pos < LowerColorMaxValue)
  {
    color = GC9A01A_YELLOW;
  }
  else
  {
    color = GC9A01A_GREEN;
  }

  // Position control
  int x = -10;
  int y = centerY - (height / 2);
  if (Pos > maxValue)
    Pos = maxValue;
  if (Pos < min)
    Pos = min;

  // Calculate the current and previous width to update the color correctly
  int widthNow = map(Pos * 100, min * 100, maxValue * 100, 0, 250);
  int widthPrev = map(lastPos * 100, min * 100, maxValue * 100, 0, 250);

  // Update the color of full rect if the color changed between the thresholds
  if (activeColor != color)
  {
    tft.fillRect(x, y, widthNow, height, color);
  }
  // Update only the new pixels if the box gets bigger
  if (widthNow > widthPrev)
  {
    tft.fillRect(x + widthPrev, y, widthNow - widthPrev, height, color);
  }
  // remove color if the box gets smaller
  else if (widthNow < widthPrev)
  {
    tft.fillRect(x + widthNow, y, widthPrev - widthNow, height, GC9A01A_BLACK);
  }
  // Update lastPos and LastActiveColor
  lastPos = Pos;
  activeColor = color;
}

void startUpBox(float Pos)
{
  // //
  // Function to draw out the box in a nice way at startup
  // //
  int x = -10;
  int y = centerY - (height / 2);
  tft.fillRect(x, y, 200, height, GC9A01A_BLACK);
  for (int i = 0; i < Pos; i++)
  {
    drawBoxes(i);
    delay(10);
  }
}

void saveToPrefs(unsigned long &lastSave)
{
  // Save all values to prefs memory at a given time
  if (millis() - lastSave > SaveInterval)
  {
    prefs.putFloat(prefsMinColor, LowerColorMaxValue);
    prefs.putFloat(prefsMaxColor, UpperColorMinValue);
    lastSave = millis();
  }
}

void StartDisplay()
{
  // //
  // Function to start the display in a nice way
  // Start it up
  // Clean all pixels
  // Slowly draw the box area
  // Write out all text
  // //
  pinMode(TFT_RST, OUTPUT);
  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(50);
  tft.begin();
  tft.fillScreen(GC9A01A_BLACK);
  delay(100);

  tft.setTextColor(GC9A01A_WHITE);
  tft.setTextSize(2);
  int16_t x1, y1;
  uint16_t w, h;
  //tft.getTextBounds(lowerText, 0, 0, &x1, &y1, &w, &h);
  int valueX = centerX - w / 2;
  int valueXLower = centerX - w / 2;

  lastPos = -999;
  int x = -10;
  int y = centerY - (height / 2);
  
  // Write texts.
  for (int i = 0; i < 4; i ++){
    tft.getTextBounds(Textes[i], 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(centerX - w / 2, TextesPos[i]);
  tft.println(Textes[i]);
  }
  delay(2000);
  
  tft.drawRect(x, y - 1, 252, height + 2, GC9A01A_WHITE);
  SensorData data;
  memcpy(&data, (void*)&latestData, sizeof(SensorData));
  startUpBox(data.posMM);
}

void setup()
{
  Serial.begin(115200);
  delay(1000);
  initESPNow();
  initWebServer();

  //Start the prefs memory and ready out all values
  prefs.begin("hydrolift", false);
  LowerColorMaxValue = prefs.getFloat(prefsMinColor, 0);
  UpperColorMinValue = prefs.getFloat(prefsMaxColor, 0);

  SPI.begin(20, -1, 19, 15);
  // Function for starting the display
  StartDisplay();
  // Filtertime
  delay(100);
}

void loop()
{  
  static unsigned long lastSave = 0;
  static unsigned long lastUpdate = 0;

  dnsServer.processNextRequest();

  if (newDataAvailable) {
    newDataAvailable = false;
    SensorData data;
    memcpy(&data, (void*)&latestData, sizeof(SensorData));

    if (data.posMM < 0) data.posMM = 0;
    else if (data.posMM > maxValue) data.posMM = maxValue;

    UpdateText(data.posMM, lastUpdate);
    drawBoxes(data.posMM);
  }

  saveToPrefs(lastSave);
}
