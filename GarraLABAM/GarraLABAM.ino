#include <WiFi.h>
#include <ESP32Servo.h>

#define SERVO1 25 // Pino para o Servo 1 (Braço 1)
#define SERVO2 26 // Pino para o Servo 2 (Garra)
#define SERVO3 32 // Pino para o Servo 3 (Braço 2)
#define SERVO4 33 // Pino para o Servo 4 (Base)
#define INPUT_SENSOR 35

Servo servo1, servo2, servo3, servo4;

const char* ssid = "LABAM-ROBOT";
const char* password = "labam378";

WiFiServer server(80);

String servoBasetxt, servoBraco1txt, servoBraco2txt, servoGarratxt;

int servoBase = 90;
int servoBraco1 = 90;
int servoBraco2 = 90;
int servoGarra = 90;
bool enabled = false;

bool homePosition = false;
bool autoCycle = false;
bool stopCycle = false;

String request;

void setup() {
  Serial.begin(115200);
  
  pinMode(INPUT_SENSOR, INPUT);

  WiFi.softAP(ssid, password);
  
  Serial.print("IP do AP: ");
  Serial.println(WiFi.softAPIP());

  servo1.attach(SERVO1);
  servo2.attach(SERVO2);
  servo3.attach(SERVO3);
  servo4.attach(SERVO4);
  
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  
  if (client) {
    request = client.readStringUntil('\r');
    client.flush();
    
    if (request.indexOf("GET /update?") != -1) {
      if (request.indexOf("enable=") != -1) {
        int start = request.indexOf("enable=") + 7;
        int end = request.indexOf("&", start);
        enabled = (request.substring(start, end) == "true");
      }
      
      if (request.indexOf("servo1=") != -1) {
        int start = request.indexOf("servo1=") + 7;
        int end = request.indexOf("&", start);
        servoBase = request.substring(start, end).toInt();
      }
      
      if (request.indexOf("servo2=") != -1) {
        int start = request.indexOf("servo2=") + 7;
        int end = request.indexOf("&", start);
        servoBraco1 = request.substring(start, end).toInt();
      }
      
      if (request.indexOf("servo3=") != -1) {
        int start = request.indexOf("servo3=") + 7;
        int end = request.indexOf("&", start);
        servoBraco2 = request.substring(start, end).toInt();
      }
      
      if (request.indexOf("servo4=") != -1) {
        int start = request.indexOf("servo4=") + 7;
        int end = request.indexOf(" ", start);
        servoGarra = request.substring(start, end).toInt();
      }
      
      verificarBotoes();

      client.println("HTTP/1.1 200 OK");
      client.println("Content-type: text/plain");
      client.println();
      client.println("OK");
      client.stop();
      
      servoBasetxt = String(servoBase);
      servoBraco1txt = String(servoBraco1);
      servoBraco2txt = String(servoBraco2);
      servoGarratxt = String(servoGarra);

      Serial.println(servoBasetxt + ";" + servoBraco1txt + ";" + servoBraco2txt + ";" + servoGarratxt);
      Serial.println("Posicao Inicial: " + String(homePosition) + "     Ciclo automático: " + String(autoCycle) + "     Parar Ciclo: " + String(stopCycle) + "        Input Sensor: " + String(digitalRead(INPUT_SENSOR)));

        servo2.write(servoGarra);
        servo4.write(servoBase);
        servo1.write(servoBraco1);
        servo3.write(servoBraco2);

      if(homePosition){
        updateServos(90, 51, 57, 90, 500);
        Serial.println("Na posição 1");
        homePosition = false;
      }

    //   if(autoCycle && digitalRead(INPUT_SENSOR) == 1){
    //     while(autoCycle && !stopCycle){
    //             updateServos(90, 90, 90, 90, 500);
    //             Serial.println("Na posição 2");
    //             delay(1000);
    //             updateServos(90, 45, 20, 90, 500);
    //             Serial.println("Na posição 3");
    //             delay(1000);
                
    //             WiFiClient newClient = server.available();
    //             if (newClient) {
    //                 String newRequest = newClient.readStringUntil('\r');
    //                 if (newRequest.indexOf("command=stop_cycle") != -1) {
    //                     stopCycle = true;
    //                 }
    //             newClient.stop();
    //             }
    //     }
    //     autoCycle = false;
    //     stopCycle = false;
    //     updateServos(90, 51, 57, 90, 500);
    //     Serial.println("Na posição 1");
    //     Serial.println("Parou ciclo!");
    //   }
    }
    
    sendHTMLPage(client);
    client.stop();
  }
}

void verificarBotoes(){
    if (request.indexOf("command=home") != -1) {
        homePosition = true;
        autoCycle = false;
        stopCycle = false;
      } else if (request.indexOf("command=start_cycle") != -1) {
        homePosition = false;
        autoCycle = true;
        stopCycle = false;
      } else if (request.indexOf("command=stop_cycle") != -1) {
        homePosition = false;
        autoCycle = false;
        stopCycle = true;
      }
}

void updateServos(int garra, int base, int braco1, int braco2, int time) {
  servo2.write(garra);
  delay(time);
  servo4.write(base);
  delay(time);
  servo1.write(braco1);
  delay(time);
  servo3.write(braco2);
  delay(time);

//   varSpeedServos(2, garra, 100);
//   delay(time);
//   varSpeedServos(4, base, 100);
//   delay(time);
//   varSpeedServos(1, braco1, 100);
//   delay(time);
//   varSpeedServos(3, braco2, 100);

  knowPos();
}

void knowPos(){
    Serial.println("Servo Garra: " + String(servo2.read()) + "    Servo da Base: " + String(servo4.read()) + "    Servo do Braco 1: " + String(servo1.read()) + "    Servo do Braco 2: " + String(servo3.read()));
}

void sendHTMLPage(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  client.println(R"=====(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Controle Avançado L.A.B.A.M</title>
    <style>
        :root {
            --primary: #0a192f;
            --secondary: #172a45;
            --accent: #64ffda;
            --text: #ccd6f6;
            --highlight: #1e90ff;
            --warning: #ff5f56;
            --enabled: #4CAF50;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: var(--primary);
            color: var(--text);
            min-height: 100vh;
            background-image: 
                radial-gradient(circle at 25% 25%, rgba(30, 144, 255, 0.1) 0%, transparent 50%),
                radial-gradient(circle at 75% 75%, rgba(100, 255, 218, 0.1) 0%, transparent 50%);
        }
        
        .container {
            max-width: 900px;
            margin: 0 auto;
            padding: 20px;
        }
        
        header {
            text-align: center;
            padding: 20px 0;
            margin-bottom: 20px;
            position: relative;
        }
        
        header::after {
            content: '';
            position: absolute;
            bottom: 0;
            left: 50%;
            transform: translateX(-50%);
            width: 200px;
            height: 3px;
            background: linear-gradient(90deg, transparent, var(--accent), transparent);
        }
        
        h1 {
            color: var(--accent);
            font-size: 2.8rem;
            margin-bottom: 10px;
            text-shadow: 0 0 10px rgba(100, 255, 218, 0.3);
            letter-spacing: 2px;
        }
        
        .subtitle {
            font-size: 1.1rem;
            opacity: 0.8;
        }
        
        .enable-control {
            display: flex;
            flex-direction: column;
            align-items: center;
            gap: 15px;
            margin-bottom: 30px;
            background-color: var(--secondary);
            padding: 20px;
            border-radius: 12px;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        .enable-status {
            width: 25px;
            height: 25px;
            border-radius: 50%;
            background-color: var(--warning);
            position: relative;
            transition: background-color 0.3s ease;
        }
        
        .enable-status.enabled {
            background-color: var(--enabled);
            box-shadow: 0 0 15px var(--enabled);
        }
        
        .enable-status::after {
            content: '';
            position: absolute;
            top: -5px;
            left: -5px;
            right: -5px;
            bottom: -5px;
            border-radius: 50%;
            border: 2px solid var(--warning);
            animation: pulse 2s infinite;
            opacity: 0;
        }
        
        .enable-status.enabled::after {
            border-color: var(--enabled);
        }
        
        @keyframes pulse {
            0% { transform: scale(0.8); opacity: 0.5; }
            70% { transform: scale(1.3); opacity: 0; }
            100% { opacity: 0; }
        }
        
        .btn {
            background-color: var(--secondary);
            color: var(--text);
            border: none;
            padding: 12px 30px;
            border-radius: 30px;
            cursor: pointer;
            font-size: 1rem;
            transition: all 0.3s ease;
            text-decoration: none;
            text-align: center;
            position: relative;
            overflow: hidden;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
            font-weight: 500;
        }
        
        .btn:hover {
            background-color: rgba(100, 255, 218, 0.1);
            color: var(--accent);
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(100, 255, 218, 0.3);
        }
        
        .btn.active {
            background-color: var(--accent);
            color: var(--primary);
            font-weight: bold;
        }
        
        .slider-container {
            width: 100%;
            max-width: 600px;
            margin: 20px auto;
            padding: 20px;
            background-color: var(--secondary);
            border-radius: 15px;
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        .slider-group {
            margin-bottom: 25px;
        }
        
        .slider-label {
            display: flex;
            justify-content: space-between;
            margin-bottom: 8px;
            font-size: 1.1rem;
        }
        
        .slider-name {
            color: var(--accent);
        }
        
        .slider-value {
            font-family: 'Courier New', monospace;
            color: var(--highlight);
            font-weight: bold;
            min-width: 40px;
            text-align: right;
        }
        
        .slider {
            -webkit-appearance: none;
            width: 100%;
            height: 10px;
            border-radius: 5px;
            background: var(--primary);
            outline: none;
        }
        
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 22px;
            height: 22px;
            border-radius: 50%;
            background: var(--accent);
            cursor: pointer;
            transition: all 0.2s ease;
        }
        
        .slider::-webkit-slider-thumb:hover {
            background: var(--highlight);
            transform: scale(1.2);
        }
        
        /* Novo estilo para os botões de controle */
        .control-buttons {
            display: flex;
            justify-content: center;
            gap: 15px;
            margin: 30px 0;
            flex-wrap: wrap;
        }
        
        .control-btn {
            background-color: var(--secondary);
            color: var(--text);
            border: none;
            padding: 12px 25px;
            border-radius: 30px;
            cursor: pointer;
            font-size: 1rem;
            transition: all 0.3s ease;
            min-width: 180px;
        }
        
        .control-btn:hover {
            background-color: rgba(100, 255, 218, 0.1);
            color: var(--accent);
            transform: translateY(-3px);
            box-shadow: 0 8px 25px rgba(100, 255, 218, 0.3);
        }
        
        .home-btn {
            border: 2px solid #64ffda;
        }
        
        .start-cycle-btn {
            border: 2px solid #1e90ff;
        }
        
        .stop-cycle-btn {
            border: 2px solid #ff5f56;
        }
        
        @media (max-width: 768px) {
            h1 {
                font-size: 2.2rem;
            }
            
            .btn {
                padding: 10px 20px;
            }
            
            .control-btn {
                min-width: 150px;
                padding: 10px 20px;
            }
        }
        
        @media (max-width: 480px) {
            .container {
                padding: 15px;
            }
            
            .control-buttons {
                flex-direction: column;
                align-items: center;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <header>
            <h1>L.A.B.A.M ROBOTICS PRO</h1>
            <p class="subtitle">Controle Avançado do Braço Robótico</p>
        </header>
        
        <div class="enable-control">
            <div class="enable-status" id="enableStatus"></div>
            <button class="btn" id="enableBtn">
                <i class="fas fa-power-off"></i> Habilitar Controle
            </button>
        </div>
        
        <div class="control-buttons">
            <button class="control-btn home-btn" id="homeBtn">
                <i class="fas fa-home"></i> Posição Inicial
            </button>
            <button class="control-btn start-cycle-btn" id="startCycleBtn">
                <i class="fas fa-sync-alt"></i> Ciclo Sensorizado
            </button>
            <button class="control-btn stop-cycle-btn" id="stopCycleBtn">
                <i class="fas fa-stop"></i> Parar Ciclo
            </button>
        </div>
        
        <div class="slider-container">
            <div class="slider-group">
                <div class="slider-label">
                    <span class="slider-name"><i class="fas fa-arrows-alt-v"></i> Servo Base</span>
                    <span class="slider-value" id="servo1Value">90°</span>
                </div>
                <input type="range" min="0" max="180" value="90" class="slider" id="servo1Slider">
            </div>
            
            <div class="slider-group">
                <div class="slider-label">
                    <span class="slider-name"><i class="fas fa-arrows-alt-v"></i> Servo Horizontal</span>
                    <span class="slider-value" id="servo2Value">90°</span>
                </div>
                <input type="range" min="0" max="90" value="51" class="slider" id="servo2Slider">
            </div>
            
            <div class="slider-group">
                <div class="slider-label">
                    <span class="slider-name"><i class="fas fa-arrows-alt-v"></i> Servo Vertical</span>
                    <span class="slider-value" id="servo3Value">90°</span>
                </div>
                <input type="range" min="0" max="110" value="57" class="slider" id="servo3Slider">
            </div>
            
            <div class="slider-group">
                <div class="slider-label">
                    <span class="slider-name"><i class="fas fa-hand-paper"></i> Servo Garra</span>
                    <span class="slider-value" id="servo4Value">90°</span>
                </div>
                <input type="range" min="0" max="130" value="90" class="slider" id="servo4Slider">
            </div>
        </div>
    </div>

    <script>
        document.getElementById('enableBtn').addEventListener('click', function() {
            const enableStatus = document.getElementById('enableStatus');
            const isEnabled = enableStatus.classList.toggle('enabled');
            
            this.innerHTML = isEnabled 
                ? '<i class="fas fa-power-off"></i> Desabilitar Controle' 
                : '<i class="fas fa-power-off"></i> Habilitar Controle';
            
            const sliders = document.querySelectorAll('.slider');
            sliders.forEach(slider => {
                slider.disabled = !isEnabled;
            });
            
            updateServos('enable=' + isEnabled);
        });
        
        function setupSlider(sliderId, valueId, servoNumber) {
            const slider = document.getElementById(sliderId);
            const valueDisplay = document.getElementById(valueId);
            
            slider.addEventListener('input', function() {
                valueDisplay.textContent = this.value + '°';
                
                updateServos('servo' + servoNumber + '=' + this.value);
            });
        }
        
        setupSlider('servo1Slider', 'servo1Value', 1);
        setupSlider('servo2Slider', 'servo2Value', 2);
        setupSlider('servo3Slider', 'servo3Value', 3);
        setupSlider('servo4Slider', 'servo4Value', 4);
        
        function updateServos(params) {
            fetch('/update?' + params)
                .then(response => response.text())
                .then(data => console.log('Resposta:', data))
                .catch(error => console.error('Erro:', error));
        }
        
        document.getElementById('homeBtn').addEventListener('click', function() {
            updateServos('command=home');
        });
        
        document.getElementById('startCycleBtn').addEventListener('click', function() {
            updateServos('command=start_cycle');
        });
        
        document.getElementById('stopCycleBtn').addEventListener('click', function() {
            updateServos('command=stop_cycle');
        });
    </script>
</body>
</html>
)=====");
}


void varSpeedServos(int servo, double angle, double time) {
    Servo* selectedServo;

    switch (servo) {
        case 1:
            selectedServo = &servo1;
            break;
        case 2:
            selectedServo = &servo2;
            break;
        case 3:
            selectedServo = &servo3;
            break;
        case 4:
            selectedServo = &servo4;
            break;
        default:
            return; // Invalid servo number
    }

    int currentAngle = selectedServo->read();
    int targetAngle = (int)angle;
    int steps = abs(targetAngle - currentAngle);
    
    if (steps == 0) return;

    // Calculate the delay per step in milliseconds
    double delayPerStep = (time * 1000.0) / steps;

    if (currentAngle < targetAngle) {
        for (int i = currentAngle; i <= targetAngle; i++) {
            selectedServo->write(i);
            delay((int)delayPerStep);
        }
    } else {
        for (int i = currentAngle; i >= targetAngle; i--) {
            selectedServo->write(i);
            delay((int)delayPerStep);
        }
    }
}
