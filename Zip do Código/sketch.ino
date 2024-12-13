#include <WiFi.h>
#include <HTTPClient.h>

// Apaguei LED azul, não estava sendo utilizado!!!

#define led_verde 2 // Pino utilizado para controle do led verde
#define led_vermelho 40 // Pino utilizado para controle do led vermelho
#define led_amarelo 9 // Pino utilizado para controle do led azul

const int pinoBotao = 18;  // o número do pino do botão
int estadoBotao = 0;  // variável para ler o estado do botão

const int pinoLdr = 4;  // o número do pino do LDR
int luminosidade=600; // váriavel para determinar quando está claro ou escuro
// luminosidade <=600: Escuro, luminosidade > 600: Claro

// Quando estiver escuro
int intervaloAmareloEscuro = 1000; // número do intervalo para piscar o LED Amarelo

//Quando estiver calro
unsigned long intervaloVerde = 3000; // número do intervalo para ligar o LED Verde
unsigned long intervaloAmareloClaro = 2000; // número do intervalo para ligar o LED Amarelo
unsigned long intervaloVermelho = 5000; // número do intervalo para ligar o LED Vermelho

// Estados do semáforo no modo claro
int estadoSemaforo = 0; // 0: Verde, 1: Amarelo, 2: Vermelho

bool botaoPressionadoNoVermelho = false;
unsigned long tempoBotaoPressionado = 0; // Variável para contar o tempo após o botão for pressionado


unsigned long cronometro; // variável para armazenar o tempo passado 


void setup() {
  Serial.begin(9600); // Configuração para debug por interface serial entre ESP e computador com baud rate de 9600

  // Configuração inicial dos pinos para controle dos leds como OUTPUTs (saídas) do ESP32
  pinMode(led_verde, OUTPUT);
  pinMode(led_vermelho, OUTPUT);
  pinMode(led_amarelo, OUTPUT);

  // Inicialização das entradas
  pinMode(pinoBotao, INPUT); // Iniciar botão como INPUT

  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);

  cronometro = millis(); // atribuindo a variavel cronometro o valor 0

  // Inicializando a luz verde como próxima do semáforo do modo claro
  estadoSemaforo = 0;


  // WiFi.begin("Wokwi-GUEST", ""); // Conexão à rede WiFi aberta com SSID Wokwi-GUEST

  // while (WiFi.status() != WL_CONNECT_FAILED) {
  //   delay(100);
  //   Serial.print(".");
  // }
  // Serial.println("Conectado ao WiFi com sucesso!"); // Considerando que saiu do loop acima, o ESP32 agora está conectado ao WiFi (outra opção é colocar este comando dentro do if abaixo)

  // if(WiFi.status() == WL_CONNECTED){ // Se o ESP32 estiver conectado à Internet
  //   HTTPClient http;

  //   String serverPath = "http://www.google.com.br/"; // Endpoint da requisição HTTP

  //   http.begin(serverPath.c_str());

  //   int codigoRespostaHttp = http.GET(); // Código do Resultado da Requisição HTTP

  //   if (codigoRespostaHttp>0) {
  //     Serial.print("HTTP Response code: ");
  //     Serial.println(codigoRespostaHttp);
  //     String payload = http.getString();
  //     Serial.println(payload);
  //     }
  //   else {
  //     Serial.print("Error code: ");
  //     Serial.println(codigoRespostaHttp);
  //     }
  //     http.end();
  //   }

  // else {
  //   Serial.println("WiFi Disconnected");
  // }
}

// Função para desligar todos os LEDs
// Assim é possivel evitar conflitos entre condições e LED's acesos!
void desligaTodos() {
  digitalWrite(led_verde, LOW);
  digitalWrite(led_vermelho, LOW);
  digitalWrite(led_amarelo, LOW);
}

void loop() {
  // Verifica estado do botão
  estadoBotao = digitalRead(pinoBotao);
  if (estadoBotao == HIGH) {
    Serial.println("Botão pressionado!");
  } else {
    Serial.println("Botão não pressionado!");
  }

  int estadoLdr = analogRead(pinoLdr); 
  Serial.print("Leitura LDR: ");
  Serial.println(estadoLdr);

  if (estadoLdr <= luminosidade) {
    // Modo Escuro
    desligaTodos();
    // Pisca o LED amarelo a cada segundo
    if (millis() - cronometro >= intervaloAmareloEscuro) {
      cronometro = millis(); 
      // Se estiver apagado acende, se estiver aceso, apaga
      int estadoAmarelo = digitalRead(led_amarelo);
      digitalWrite(led_amarelo, !estadoAmarelo);
    }
  } else {
    // Modo Claro
    switch (estadoSemaforo) {
      case 0:
        if(digitalRead(led_verde) == LOW){
          desligaTodos(); // Desligar LED's e evitar conflitos
          digitalWrite(led_verde, HIGH); // Ascendendo o LED Verde
          cronometro = millis(); // Atualizar valor da contagem do tempo
        }

        // Condição para concluir o intervalo do LED Verde
        if (millis() - cronometro >= intervaloVerde) {
          // Próximo estado será o Amarelo, ou seja, 1
          estadoSemaforo = 1;
          desligaTodos();
          cronometro = millis(); // Atualizar valor da contagem do tempo
        }
        break;

      case 1:
        if(digitalRead(led_amarelo) == LOW){
          desligaTodos(); // Desligando o LED anterior, Verde
          digitalWrite(led_amarelo, HIGH); // Ascendendo o LED Amarelo
          cronometro = millis(); // Atualizar valor da contagem do tempo
        }

        if (millis() - cronometro >= intervaloAmareloClaro) {
          estadoSemaforo = 2;
          desligaTodos();
          cronometro = millis(); // Atualizar valor da contagem do tempo
        }
        break;

      case 2: 
        if(digitalRead(led_vermelho) == LOW){
          desligaTodos(); //Desligando LED anterior, Amarelo
          digitalWrite(led_vermelho, HIGH); // Ascendendo o LED Vermelho
          cronometro = millis(); // Atualizar valor da contagem do tempo
        }

        // Condição para verificar se o botão foi pressionado
        if (estadoBotao == HIGH && botaoPressionadoNoVermelho == false) {
          botaoPressionadoNoVermelho = true;
          tempoBotaoPressionado = millis();
        }
        
        // Condição concluir o intervalo após o click do botão
        if (botaoPressionadoNoVermelho && (millis() - tempoBotaoPressionado >= 1000)) {
          desligaTodos();
          digitalWrite(led_verde, HIGH);
          estadoSemaforo = 0; 
          cronometro = millis();
          botaoPressionadoNoVermelho = false;
        } else {
          // Caso o botão não tenha sido pressionado no vermelho, seguir o fluxo normal
          if (!botaoPressionadoNoVermelho && (millis() - cronometro >= intervaloVermelho)) {
            // Volta para o LED Verde
            estadoSemaforo = 0;
            desligaTodos();
            cronometro = millis();
          }
        }
        break;
    }
  }
}
