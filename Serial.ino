#define PINO_RX 13
#define PINO_TX 13
#define PINO_HANDSHAKE_IN 12
#define PINO_HANDSHAKE_OUT 11
#define BAUD_RATE 1
#define HALF_BAUD 1000/(2*BAUD_RATE)
#define WORD_LENGHT 8

#include "Temporizador.h"
#include <stdlib.h>
#include <string.h>

char start;
char dados;
char i;
char leitura;
char handshake;

// Calcula bit de paridade - Par ou impar
bool bitParidade(char dado, char paridade) {
  char contador;
  for (char b = 0; b < WORD_LENGHT; b++) {
    if ((1 << b) & dado)
      contador++;
  }
  if (!((contador + paridade) % 2)) {
    Serial.println(dado);
  } else {
    Serial.print("Paridade errada! Caracter recebido: ");
    Serial.println(dado);
  }
}

// Rotina de interrupcao do timer1
// O que fazer toda vez que 1s passou?
ISR(TIMER1_COMPA_vect) {
  leitura = digitalRead(PINO_RX); // Lê do input
  Serial.print("Entrada = ");
  Serial.println(leitura == HIGH);
  
  if (i++ < (WORD_LENGHT + 1)) {    // Se ainda estiver lendo a palavra
    if (i == WORD_LENGHT + 1) { // Caso seja o bit de paridade
      bitParidade(dados, leitura);
    } else {  // Senão, adiciona o bit aos dados
      dados = dados << 1;
      if (leitura == HIGH)
        dados |= 1;
    }
  } else {          // Já acabou a palavra, então para o temporizador
    start = HIGH;    
    i = 0;
    paraTemporizador();
  }
}

// Executada uma vez quando o Arduino reseta
void setup() {
  start = HIGH;
  handshake = LOW;
  i = 0;
  //desabilita interrupcoes
  noInterrupts();
  // Configura porta serial (Serial Monitor - Ctrl + Shift + M)
  Serial.begin(9600);
  // Inicializa TX ou RX
  pinMode(PINO_RX, INPUT);
  pinMode(PINO_HANDSHAKE_IN, INPUT);
  pinMode(PINO_HANDSHAKE_OUT, OUTPUT);
  // Configura timer
  configuraTemporizador(BAUD_RATE);
  // habilita interrupcoes
  interrupts();
}

// O loop() eh executado continuamente (como um while(true))
void loop ( ) {
  leitura = digitalRead(PINO_HANDSHAKE_IN); // Lê do input
  if (leitura == HIGH && handshake == LOW) {    // Iniciaram o handshake mas ainda não respondi
    handshake = HIGH;
    digitalWrite(PINO_HANDSHAKE_OUT, handshake);  // Seta o pino de resposta pra HIGH
  } else if (leitura == LOW && handshake == HIGH) {  // Finalizaram o handshake 
    handshake = LOW;
    digitalWrite(PINO_HANDSHAKE_OUT, handshake);  // Seta o pino de resposta pra LOW
  }
  if (handshake == HIGH) {
    leitura = digitalRead(PINO_RX);
    if (leitura == LOW && start == HIGH) {   // Enviaram sinal de start bit
      delay(HALF_BAUD);
      start = LOW;
      iniciaTemporizador();
    }
  }
}
