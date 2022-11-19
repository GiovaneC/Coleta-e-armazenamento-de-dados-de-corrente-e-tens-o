
// Bibliotecas do projeto

#include <LiquidCrystal.h>  // Biblioteca para o LCD
#include <TimeLib.h>        // Biblioteca para funções de tempo
#include <SPI.h>            // Biblioteca para o Ethernet Shield 
#include <SD.h>             // Biblioteca para cartão SD

// Define nome para as portas
#define corrente 0
#define tensao 1
#define troca 9
#define enter 8              

// Inicia uma instância do objeto LiquidCrystal com o nome de lcd para utilizar o LCD
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);


// Inicia uma instância do objeto FILE com o nome de arquivo para manipular arquivos
File arquivo;


// Constantes da resistência medida dos resistores do divisor de tensão (Proporção 10:1)
const int R1 = 21900, R2 = 2180;


// Variaveis
int gravacoes = 0;
int hora = 0, minuto = 0, segundo = 0;
float valorBruto = 0, valorTratado = 0;
float somaTensao = 0, somaCorrente = 0, mediaTensao = 0, mediaCorrente = 0;


// Cabeçalho das funções
void medir(void);
void armazenarMedia(void);
float converter(float, float, float);


void setup() {

  // Coloca as portas dos botões como entrada
  pinMode(troca, INPUT);
  pinMode(enter, INPUT);

  // Inicia o cartão SD
  SD.begin(4);
  // Inicia o LCD
  lcd.begin(16, 2);

  // Espera 1 segundo
  delay(1000);

  // Abre o arquivo Dados_Coletados.csv (comma-separated values) no modo escrita
  arquivo = SD.open("dados.csv", FILE_WRITE);
  
  // Printa o cabeçalho para o arquivo
  arquivo.println("Hora da Leitura,Corrente,Tensao");

  // Fecha o arquivo para salvar no cartão SD
  arquivo.close();


  // ********** Setup da hora do sistema ********** //

  // Horas

  // escreve no LCD "Hora"
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.print(hora);


  // loop para escolher a hora
  while (digitalRead(enter) == LOW)
  {
    // Só sai do loop se apertar enter

    // Se apertou o botão para trocar, aumenta uma hora da variavel
    if (digitalRead(troca) == HIGH)
    {
      hora++;
      // Se passar de 23 horas, volta para 0
      if (hora > 23) {
        hora = 0;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hora: ");
      lcd.print(hora);
      while (digitalRead(troca) == HIGH) {}
    }
    delay(100);
  }

  while (digitalRead(enter) == HIGH) {}


  // Minutos

  // escreve no LCD "Minuto"
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Minuto: ");
  lcd.print(minuto);

  // loop para escolher os minutos
  while (digitalRead(enter) == LOW)
  {
    // Só sai do loop se apertar enter

    // Se apertou o botão para trocar, aumenta um minuto da variavel
    if (digitalRead(troca) == HIGH)
    {
      // Se passar de 59 minutos, volta para 0
      minuto++;
      if (minuto > 59) {
        minuto = 0;
      }
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Minuto: ");
      lcd.print(minuto);
      while (digitalRead(troca) == HIGH) {}
    }
    delay(100);
  }
  while (digitalRead(enter) == HIGH) {}

  // Set a hora do sistema apartir dos dados inseridos, data set manualmente
  setTime(hora, minuto, segundo, 18, 9, 2022);

}

void loop() {

  // Faz 12 medições até chamar a função "armazenaMedia", a qual faz uma média das medições e armazena no cartão SD
  for (int i = 0; i < 12; i++) {
    medir();
    delay(5000);
  }

  // Chama a função armazena média
  armazenaMedia();

}


// Função converte a entrada 0 a 1023 para a resolução do sensor dado pelas variaveis minimo e maximo
float converter(float entrada, float minimo, float maximo)
{
  return entrada * (maximo - minimo) / (1023 - 0) + minimo;
}


// ***** FUNÇÃO MEDIR CORRENTE E TENSÃO ***** //

void medir(void) {


  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Leituras: ");
  lcd.print(gravacoes);

  // Le a corrente através do sensor de efeito hall conectado a porta A0
  valorBruto = analogRead(corrente);
  
  // Faz uma transformação linear do valor lido na resolução 10 bits (2^10 = 1024) para a resolução do sensor
  valorTratado = converter(valorBruto, -30, 30) ;
  somaCorrente += valorTratado;
  

  lcd.setCursor(0, 1);
  lcd.print("A:");
  lcd.print(valorTratado);
  lcd.setCursor(9, 1);
  lcd.print("V:");


  // Le a tensão da bateria através da tensão proveniente do divisor de tensão conectado a porta A1
  valorBruto = analogRead(tensao);
 
  // Faz uma transformação linear do valor lido na resolução 10 bits (2^10 = 1024) para a resolução do divisor de tensão
  // Divisor de tensão será a relação da resistência medida dos dois resistores multiplicado por 5V (referência da porta analógica do arduino)
  valorTratado = converter(valorBruto, 0, 5) ;
  valorTratado = valorTratado * ((R1 + R2) / R2);
  somaTensao += valorTratado;

  lcd.print(valorTratado);

  return;
}

// ***** FUNÇÃO ARMAZENAR A MÉDIA ***** //

void armazenaMedia() {


  // Abre o arquivo novamente para edição
  arquivo = SD.open("dados.csv", FILE_WRITE);

  if (arquivo)
  {
    gravacoes++;
    
    // Na primeira coluna da nova linha de dados, imprime a hora, minuto e segundo da leitura
    arquivo.print(hour());
    arquivo.print(":");
    arquivo.print(minute());

    // printa uma virgula para separar o proximo valor
    arquivo.print(",");

    mediaCorrente = somaCorrente / 12;
    somaCorrente = 0;

    // Armazena o valor no arquivo CSV
    arquivo.print(mediaCorrente);

    // printa uma virgula para separar o proximo valor
    arquivo.print(",");

    mediaTensao = somaTensao / 12;
    somaTensao = 0;

    // Armazena o valor no arquivo CSV
    arquivo.println(mediaTensao);

    
    // Fecha o arquivo para salvar no cartão SD
    arquivo.close();
  }
  
  return;
}
