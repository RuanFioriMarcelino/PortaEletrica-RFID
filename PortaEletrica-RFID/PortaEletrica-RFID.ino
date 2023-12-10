#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
LiquidCrystal_I2C lcd(0x27,16,2);
#include <SPI.h>
#include <MFRC522.h>  //Biblioteca para o RFID
#define SS_PIN 5      //Define o pino SS do RFID
#define RST_PIN 2     //Define o pino RST do RFID
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.


const int relay = 26; //Variavel do Relé
bool autorizado = 0;

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);

const char* html = R"(
  <!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Poppins&display=swap');

* {
    margin: 0;
    padding: 0;
    box-sizing: border-box;
}

body {
    font-family: 'Poppins';
    min-height: 98vh;
    background-color: rgb(132, 132, 185);
}

header {
    padding-top: 8px;
    box-shadow: 0px 12px 14px 0px rgba(0,0,0,0.15);
    background: radial-gradient(white, rgb(231, 231, 231));
}

.logo {
    width: 100%;
    display: flex;
    flex-direction: column;
    align-items: center;
}


.logo img {
    width: 80px;
}

nav ul{
    display: flex;
    justify-content: center;
    list-style: none;
    background-color: white;
    width: 100%;
}



.container {
    background-color: rgb(132, 132, 185);
    display: flex;
    align-items: center;
    flex-direction: column;
    padding: 24px 0;
}

.container a {
    color: white;
    text-decoration: none;
    padding: 24px;
    margin: 12px;
    border: 2px solid white;
    border-radius: 16px;
    background-color: rgb(115, 115, 168);
    transition: all 0.2s;
}
.container a:hover {
    background-color: rgb(107, 107, 156);
}
footer {
    position:absolute;
    bottom: 0;
    width: 100%;
    display: flex;
    justify-content: center;
    background-color: white;
}
    </style>


    <script defer>
        let fechadura = document.getElementById('fechadura')
    let count = 1


    function abrir() {

        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/", true);
        xhr.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");
        if (count == 0) {
            
            count = 1;
            
        } else if (count == 1) {
            
            count = 0;
        }
        xhr.onreadystatechange = function() {
            if (xhr.readyState == 4 && xhr.status == 200) {
                document.getElementById("resp").innerHTML = xhr.responseText;
    }
}
xhr.send("count=" + count);
}
    </script>
    <title>Fechadura</title>
</head>
<body>
    <header>
        <div class="logo">
            <img src="https://banner2.cleanpng.com/20180624/rzu/kisspng-computer-icons-computer-security-closed-circuit-te-5b2fd95ab45c71.2262348015298624907388.jpg" alt="">
            <h1>Security Lock</h1>
        </div>
    </header>
    <div class="container">
            <a href="#" id="fechadura" onclick='abrir()'>Fechar Porta</a>
            <p id="resp"></p>
    </div>

    <footer>
        <h3>Feito Por Bruno Vieira, Ruan</h3>
    </footer>
</body>
</html>
)";

const char* ssid = "SATC IOT";
const char* password = "IOT2023@";

void setup() 
{
  Serial.begin(115200);   //Inicia a comunicação serial 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  
    
  server.begin();
  
  digitalWrite(relay,HIGH );
  lcd.init();                      // inicializa o lcd 
  lcd.backlight();
  
  SPI.begin();            // Inicias o SPI
  mfrc522.PCD_Init();     // Inicializa o MFRC522
  Serial.println("Aproxime o cartão para leitura ");
  Serial.println();

  pinMode(relay, OUTPUT); //Define o relé como saída
  digitalWrite(relay,HIGH );
  lcd.setCursor(0,0);
  lcd.print("   Bem  Vindo");//mensagem no display

  server.on("/", HTTP_ANY, [](AsyncWebServerRequest * request) {
      if (request->method() == HTTP_GET) {
        request->send(200,"text/html", html);
      } else if (request -> method() == HTTP_POST) {
        if (request -> hasParam("count", true)) {
          String count = request -> getParam("count", true)->value();
          int count1 = count.toInt();
          Serial.println(count1);
          digitalWrite(relay, count1);
        }
      }
    });

  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if ( ! mfrc522.PICC_IsNewCardPresent()) //Verificando se um novo cartão RFID 
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())  //Ler os dados do cartão RFID que foi detectado anteriormente
  {
    return;
  }

  Serial.println("UID tag :");  //Mostra UID da tag na serial 
  String content= "";         //
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) //Percorre cada byte do UID do cartão RFID
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");            //Imprime o valor hexadecimal desse byte na porta serial
     Serial.print(mfrc522.uid.uidByte[i], HEX);                           //
     content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));  //Preenche com os bytes do UID
     content.concat(String(mfrc522.uid.uidByte[i], HEX));                 //
  }
  content.toUpperCase();   //Converte a string em letras maiúsculas 
  
 if ((content.substring(1) == "0A C0 8B B1")||(content.substring(1) == "71 DF 55 6A")){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("     Acesso");
    lcd.setCursor(0,1);
    lcd.print("   Autorizado");
    digitalWrite(relay,LOW );
    delay(2500);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   Bem  Vindo");
    digitalWrite(relay,HIGH );
  } 
  else {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("     Acesso");
    lcd.setCursor(0,1);
    lcd.print("     Negado");
    delay(2500);
    lcd.clear();
    lcd.print("   Bem  Vindo");
  }
}