/* Dinamômetro Digital - revisão 3.2 - 21/10/2019 -> código final 
 * Universidade São Judas Tadeu 
 * FTCE - Engenharia Eletrônica
 * Código desenvolvido como parte do Projeto de TCC 
 * Alunos:                                R.A.:
 *        Eliana Harume Rodrigues Tamari        201404433
 *        José Leonardo Natalício               817110728
 *        Rafael de Oliveira Silva              201312986
 * Turma: EET6AN-MCA
 ******************************************************************/      
// ========================================================================== //
// ---------------------------- Variáveis Globais --------------------------- // 
byte     idade = 18,                  // variável para delimitar valor mínimo de idade
         peso = 40,                   // variável para delimitar valor mínimo de peso
         altura = 100,                // variável para delimitar valor mínimo de altura
         paciente = 1,                // variável para delimitar valor mínimo de pacientes
         paciente_reg,                // variável auxiliar a struct paciente (registro)
         paciente_idade_reg,          // variável que vai registrar o valor setado da idade do paciente
         paciente_peso_reg,           // variável que vai ragistrar o peso setado do paciente
         paciente_altura_reg,         // variável que vai ragistrar a altura setada do paciente
         paciente_forca_type;         // variável que indica o tipo de dado da força após arredondamento
    
int      indice_linha = 0,            // variavel que indica em que linha irá a seta (começa na linha 0, com valor 0)
         linha_anterior_subida = 0,   // variavel que indica qual linha a seta estava na subida
         linha_anterior_descida = 0,  // variavel que indica qual linha a seta estava na descida
         funcao = 0,                  // variavel que indica qual função foi escolhida
         menu_inicial = 0,            // variavel para indicar escolha de outra função que não seja a novo paciente
         pagina = 0,                  // variavel que indica em qual sub menu esta
         limite_superior = 0,         // variável que indicao limite superior da tela (para seleção)
         limite_inferior = 3,         // variável que indicao limite inferior da tela (para seleção)
         forca_arredondado = 0,       // variável que armazenará temporariamente o valor arredondado da força medida
         endereco_memoria = 0,        // variável que indica a posição de memória para leitura ou escrita da EEPROM interna
         LABEL = 1;                   // variável que imprime o nome das colunas no EXCEL
         
long     intervalo_medicao = 60000,   // variável que indica intervalo de espera da medição de força
         contagemInicial;             // variável que fará a contagem do tempo para fzer a medição de força
         
bool  sobe_flag = false,              // flag que indicará que apertou o botao up quando for verdadeira
      desce_flag = false,             // flag que indicará que apertou o botao down quando for verdadeira
      enter_flag = false,             // flag que indicará que apertou o botao enter quando for verdadeira
      esc_flag = false,               // flag que indicará que apertou o botao esc quando for verdadeira
      loop0 = true,                   // flag para manter a função de leitura de botões em loop, quando necessário
      loop_medicao = true,            // flag para manter a medição de força em loop, quando necessário
      loop_format = true,             // flag para manter a formatação de dados em loop, quando necessário
      forca_flag = false,             // flag para indicar medição de força feita com sucesso
      forca_aux_flag = false,         // flag auxiliar para forca_flag 
      paciente_cadastrado = false,    // flag para indicar quando usuario foi cadastrado
      pac_cad;                        // flag auxiliar para paciente_cadastrado

char sexo = 'N',                      // variável para delimitar um caractér para sexo, caso não tenha nenhum
     sexo_paciente_reg;               // variável que vai ragistrar o sexo do paciente  

float forca_pico,                     // variável que indica valor máximo de força medida naquele instante 
      paciente_forca_reg,             // variável para registro da força em float
      forca_medida;                   // variável que armazena temporariamente valor final da força medida

// ========================================================================== //
// ------------------------ Mapeamento de Hardware -------------------------- //

#define  sobe   25
#define  desce  26
#define  enter  27
#define  esc    14

// ligação do circuito HX711
const int DOUT_PIN = 2;               // saida DT do modulo no terminal D2 do ESP32
const int SCK_PIN = 4;                // saida SCK do modulo no terminal D4 do ESP32

// initialize the library by associating any needed LCD interface pin
// with the esp32 pin number it is connected to
const int rs = 23, en = 22, d4 = 5, d5 = 18, d6 = 19, d7 = 21;

// ========================================================================== //
// ------------------------ Protótipo das Funções --------------------------- //

void menu_principal();                // função para entrar no menu principal
void leitura_botoes();                // função para leitura dos botoes

// ========================================================================== //

// include the library code:
#include <LiquidCrystal.h>
#include <HX711.h>
#include <elapsedMillis.h>
#include <EEPROM.h>

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const long fator_calibracao(35331); // fator de calibração do sensor
HX711 sensor;                       //quando utilizar um comando do sensor, comece pelo nome escrito aqui

elapsedMillis tempoDecorrido;       //declare global if you don't want it reset every time loop

struct cadastro {
                 byte registro;
                 byte idade;
                 byte  peso;
                 byte altura;
                 char sexo;
                 byte forca;
  }patient[20]; // fim da struct de paciente

void setup() {

  pinMode(sobe,INPUT_PULLUP);
  pinMode(desce,INPUT_PULLUP);
  pinMode(enter,INPUT_PULLUP);
  pinMode(esc,INPUT_PULLUP);

  lcd.begin(16, 4);   // set up the LCD's number of columns and rows
  Serial.begin (9600);
  EEPROM.begin(512);

  lcd.setCursor(0,0);
  lcd.print("   USJT 2019");
  lcd.setCursor(0,1);
  lcd.print("   ***TCC***");
  lcd.setCursor(0,2);
  lcd.print("  Dinamometro");
  lcd.setCursor(0,3);
  lcd.print("    Digital");

delay(5000);

  for (endereco_memoria = 6; endereco_memoria <= 177; endereco_memoria+= 9) {
    
      Serial.println("verificando pacientes cadastrados...");
      Serial.print("endereco_memoria = ");
      Serial.println(endereco_memoria);
      pac_cad = EEPROM.get(endereco_memoria, paciente_cadastrado);
      Serial.print("pac_cad = ");
      Serial.println(pac_cad);
      delay(50);
      
      if (pac_cad != true) {
          paciente_cadastrado = false;
          EEPROM.put(endereco_memoria, paciente_cadastrado);
          EEPROM.commit();
          delay(5);
      }
      
      delay(45);
  }

  paciente_cadastrado = false;
  pac_cad = false;
}

void loop() {
  menu_principal(); 
}

void menu_principal() {

  switch (funcao) { //mensagem na tela
      case 0:
          lcd.clear();
          lcd.setCursor(0,0);   // posionamento do cursor -> obs: sintaxe = lcd.setCursor(coluna, linha)
          lcd.print(">");       // coloca o caracter ">" no inicio da linha para indicar a linha que será selecionada
          lcd.setCursor(2, 0);  // posiciona o cursor um pouco apos a seta
          lcd.print("Novo Paciente");
          lcd.setCursor(0, 1);
          lcd.print("  Medicao");
          lcd.setCursor(0, 2);
          lcd.print("  Historico");
          lcd.setCursor(0, 3);
          lcd.print("  Editar Dados");
          
          pagina = 0;           // o menu principal sera a pagina "zero"
          
          if(loop0 == false) loop0 = true;
      break;
      
      case 1:
          if (pagina == 1) {
            lcd.clear();
            lcd.setCursor(3,0);
            lcd.print("Selecione o");
            lcd.setCursor(4,1);
            lcd.print("Paciente:");
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(7,2);
            lcd.print(paciente);

              if(loop0 == false) loop0 = true;
          }
          else if (pagina == 2) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print(" Defina o sexo");
              lcd.setCursor(0,1);
              lcd.print(" do Paciente:");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(3,2);
              lcd.print("feminino");
              lcd.setCursor(3,3);
              lcd.print("masculino");

                if(loop0 == false) loop0 = true;
          }
          else if (pagina == 3) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print(" Defina a idade");
              lcd.setCursor(0,1);
              lcd.print("  do Paciente:");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(6,2);
              lcd.print(idade);
              lcd.setCursor(6,3);
              lcd.print ("Anos");

                if(loop0 == false) loop0 = true;
          }
          else if (pagina == 4) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print(" Defina o peso");
              lcd.setCursor(0,1);
              lcd.print(" do Paciente:");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(6,2);
              lcd.print(peso);
              lcd.setCursor(6,3);
              lcd.print ("kg");

                if(loop0 == false) loop0 = true;
          }
          else if (pagina == 5) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print(" Defina a altura");
              lcd.setCursor(0,1);
              lcd.print("  do Paciente:");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(6,2);
              lcd.print(altura);
              lcd.setCursor(6,3);
              lcd.print ("cm");

                if(loop0 == false) loop0 = true;
          }
          else if (pagina == 6) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("    Confirmar");
              lcd.setCursor(0,1);
              lcd.print("     Dados?");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(1,2);
              lcd.print("     sim");
              lcd.setCursor(0,3);
              lcd.print("      nao");

                if(loop0 == false) loop0 = true;
          }
      break;
    
      case 2:     
          if (pagina == 1) {
            lcd.clear();
            lcd.setCursor(3,0);
            lcd.print("Selecione o");
            lcd.setCursor(4,1);
            lcd.print("Paciente:");
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(7,2);
            lcd.print(paciente);

              if(loop0 == false) loop0 = true;
          }// end if (pagina == 1)

          else if (pagina == 2) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print(" Inicializando");
              lcd.setCursor(0,1);
              lcd.print("   Sensor...");

              sensor.begin(DOUT_PIN, SCK_PIN);    // inicializacao e definicao dos pinos DT e SCK
              sensor.set_gain();                  // seleciona o ganho e canal do sensor. De acordo com o datasheet,
                                                  // a função a seguir sem parametros seleciona o canal A, com ganho de 128
              sensor.tare();                      // zera a escala
              sensor.set_scale(fator_calibracao); // ajusta a escala para o fator de calibracao
              delay(1000);
              lcd.clear();

                  if (sensor.is_ready()) {
                      lcd.setCursor(0,0);
                      lcd.print("  Pressione o");
                      lcd.setCursor(0,1);
                      lcd.print("  Dinamometro");
                      lcd.setCursor(0,2);
                      lcd.print("     agora!");
                      Serial.println("pressione o dinamometro...");
                      contagemInicial = tempoDecorrido;
                      loop_medicao = true;
                      delay(2000);

                      while (loop_medicao) {

                             if (forca_medida < 5.0 && forca_flag) { // ... se a força de pico é maior que 5 kgf E a flag for verdadeira...
                                      // mostra o valor da forca medida no display
                                      lcd.clear();
                                      delay(500);
                                      lcd.setCursor(0,0);
                                      lcd.print(" Forca Medida =");
                                      lcd.setCursor(4,2);
                                      lcd.print(forca_pico);
                                      lcd.setCursor(8,2);
                                      lcd.print(" kgf");
                                      pagina++;
                                      loop_medicao = false; // sai do loop de medição
                                      forca_flag = false;
                                      forca_aux_flag = false; 
                                      Serial.print("Força medida: ");
                                      Serial.print(forca_pico); 
                                      Serial.println("kgf");
                                      delay(3000);
                              }
                        
                             forca_medida = sensor.get_units(10); // mede a força aplicada no extensômetro
                             delay(500);
                        
                        Serial.println("loop de medição <-");
                        Serial.println("valores dos contadores:");
                        Serial.print("tempoDecorrido = ");
                        Serial.println(tempoDecorrido);
                        Serial.print("força atual = ");
                        Serial.print(forca_medida);
                        Serial.println(" kgf");
                        delay(500);
                            
                              if (forca_medida <= 5.0) {                                      // se a força medida for menor que 5.0 kgf...
                                  if (forca_aux_flag) forca_flag = true;
                                  if (tempoDecorrido > contagemInicial + intervalo_medicao) { // ...dentro de um intervalo de 60 segundos...
                                                                                              // volta para o menu principal 
                                      lcd.clear();
                                      lcd.setCursor(0,0);
                                      lcd.print("Erro: Forca");
                                      lcd.setCursor(0,1);
                                      lcd.print("medida baixa ou");
                                      lcd.setCursor(0,2);
                                      lcd.print("nenhuma forca");
                                      lcd.setCursor(0,3);
                                      lcd.print("medida.");
                                      delay(5000);
                                      Serial.println();
                                      Serial.println("Nenhuma força medida ou força medida abaixo do mínimo esperado.");
                                      loop_medicao = false;                                    // sai do loop de medição
                                      funcao = 0;
                                      menu_inicial = 0;
                                      pagina = 0;
                                      delay(1000);
                                  }
                              }
                              
                              else {                                // se a força medida for maior que 5.0 kgf...
                                  if (forca_medida > forca_pico) {  // se for maior que a força atual...
                                      forca_pico = forca_medida;    // ... atualiza o valor da força atual. 
                                                                    // Se for menor, não faz nada (ou seja, não atualiza)
                                        if (tempoDecorrido < contagemInicial + intervalo_medicao) {
                                            forca_aux_flag = true;
                                        }
                                  }
                              }
                      } // end while (loop_medicao) 
                  } // end if (sensor.is_ready())
                  
          } // end if (pagina == 2)

          else if (pagina == 3) {
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("    Confirmar");
              lcd.setCursor(0,1);
              lcd.print("     Dados?");
              lcd.setCursor(0,2);
              lcd.print(">");
              lcd.setCursor(1,2);
              lcd.print("     sim");
              lcd.setCursor(0,3);
              lcd.print("      nao");

                if(loop0 == false) loop0 = true;
          } // end if (pagina == 3)
      break;

      case 3:
         if (pagina == 1) {
            lcd.clear();
            lcd.setCursor(3,0);
            lcd.print("Selecione o");
            lcd.setCursor(4,1);
            lcd.print("Paciente:");
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(7,2);
            lcd.print(paciente);

              if(loop0 == false) loop0 = true;
          } // end if (pagina == 1)
    break;

    case 4:
        if (pagina == 1){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(">");
            lcd.setCursor(2,0);
            lcd.print("Exportar Dados");
            lcd.setCursor(2,1);
            lcd.print("Editar Dados");

              if(loop0 == false) loop0 = true;
        }
        else if (pagina == 2) {
            lcd.clear();
            lcd.setCursor(3,0);
            lcd.print("Selecione o");
            lcd.setCursor(4,1);
            lcd.print("Paciente:");
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(7,2);
            lcd.print(paciente);
  
              if(loop0 == false) loop0 = true;
        }
        else if (pagina == 3) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(">");
            lcd.setCursor(2,0);
            lcd.print("Apagar Dados");
            lcd.setCursor(2,1);
            lcd.print("Alterar Dados");
            lcd.setCursor(2,2);
            lcd.print("Formatar Dados");

                if(loop0 == false) loop0 = true;
        }
         else if (pagina == 4) {
            lcd.clear();
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(2,0);
            lcd.print("Confirmar?");
            lcd.setCursor(2,2);
            lcd.print("sim");
            lcd.setCursor(2,3);
            lcd.print("nao");
            
                if(loop0 == false) loop0 = true;
        }
        else if (pagina == 5){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("  Escolha qual");
            lcd.setCursor(0,1);
            lcd.print("  dado deseja");
            lcd.setCursor(0,2);
            lcd.print("    alterar:");
            delay(5000);
                    
            lcd.clear();
            lcd.setCursor(4,0);
            lcd.print("Sexo =");
            lcd.setCursor(11,0);
            lcd.print(patient[paciente].sexo);
            lcd.setCursor(3,1);
            lcd.print("Idade =");
            lcd.setCursor(11,1);
            lcd.print(patient[paciente].idade);
            lcd.setCursor(4,2);
            lcd.print("Peso =");
            lcd.setCursor(11,2);
            lcd.print(patient[paciente].peso);
            lcd.setCursor(2,3);
            lcd.print("Altura =");
            lcd.setCursor(11,3);
            lcd.print(patient[paciente].altura);
            lcd.setCursor(0,0);
            lcd.print(">");
            delay(1000);  

                if(loop0 == false) loop0 = true;
        }
        else if (pagina == 6) {
            if(loop0 == false) loop0 = true;
        }
        else if (pagina == 7) {
            if(loop0 == false) loop0 = true;
        }
        else if (pagina == 8) {
            if(loop0 == false) loop0 = true;
        }
        else if (pagina == 9) {
            if(loop0 == false) loop0 = true;
        }
        else if (pagina == 10) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" Todos os dados");
            lcd.setCursor(0,1);
            lcd.print(" serao apagados!");
            lcd.setCursor(0,2);
            lcd.print(">");
            lcd.setCursor(4,2);
            lcd.print("confirmar");
            lcd.setCursor(5,3);
            lcd.print("voltar");
            
                if(loop0 == false) loop0 = true;
        }
        else if (pagina == 11) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" Todos os dados");
            lcd.setCursor(0,1);
            lcd.print(" foram apagados.");
            delay(5000);
            
            funcao = 0;
            menu_inicial = 0;
            pagina = 0;
            endereco_memoria = 0;
        }
    break;
  }
  
  leitura_botoes();
  
} // end void menu_principal()

void leitura_botoes() {

  switch (funcao) { //determina os limites de cada pag
    case 0:
        limite_superior = 0;
        limite_inferior = 3;
        indice_linha = 0;
    break;
    
    case 1:
        if (pagina == 1) { // pagina 1: escolha o paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 2) { // pagina 2: escolha o sexo do paciente
            limite_superior = 2;
            limite_inferior = 3;
            indice_linha = 2; 
        }
        else if (pagina == 3) { // pagina 3: escolha a idade do paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 4) { // pagina 4: escolha o peso do paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 5) { // pagina 5: escolha a altura do paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 6) { // pagina 6: confirme suas escolhas
            limite_superior = 2;
            limite_inferior = 3;
            indice_linha = 2;
        } 
    break;
    
    case 2:
        if (pagina == 1) { // pagina 1: escolha o paciente para medir a força
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 3) { // pagina 3: confirme a medição
            limite_superior = 2;
            limite_inferior = 3;
            indice_linha = 2;
        } 
    break;

    case 3:
        if (pagina == 1) { // pagina 1: escolha o paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
    break;

    case 4:
        if (pagina == 1) { // pagina 1: escolha a opção desejada
            limite_superior = 0;
            limite_inferior = 1;
            indice_linha = 0;
        }
        else if (pagina == 2) { // pagina 2: escolha a opção desejada
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
        }
        else if (pagina == 3) { // pagina 3: escolha a opção desejada
            limite_superior = 0;
            limite_inferior = 2;
            indice_linha = 0;
        }
         else if (pagina == 4) { // pagina 4: escolha a opção desejada
            limite_superior = 2;
            limite_inferior = 3;
            indice_linha = 2;
        }
        else if (pagina == 5) { // pagina 5: escolha a opção desejada
            limite_superior = 0;
            limite_inferior = 3;
            indice_linha = 0;
        }
        else if (pagina == 6) { // pagina 6: alterar dados o paciente
            if (indice_linha == 0) { // escolha o sexo do paciente para alterar
                limite_superior = 2;
                limite_inferior = 3;
                indice_linha = 2; 
            }
            else if (indice_linha == 1) { // escolha a idade do paciente para alterar
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;
            }    
        }
        else if (pagina == 7) { // pagina 7: alterar dados o paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2; 
        }
        else if (pagina == 8) { // pagina 8: alterar dados o paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;         
        }
        else if (pagina == 9) { // pagina 9: alterar dados o paciente
            limite_superior = 2;
            limite_inferior = 2;
            indice_linha = 2;         
        }
        else if (pagina == 10) { // pagina 10: confirmação formatar dados
            limite_superior = 2;
            limite_inferior = 3;
            indice_linha = 2;         
        }
        
    break;
  }

  Serial.println();
  Serial.println("dados salvos");
  Serial.print("limimte_superior = ");
  Serial.println(limite_superior); // variavel anterior = paciente_idade_reg
  Serial.print("limimte_inferior = ");
  Serial.println(limite_inferior); // variavel anterior = paciente_peso_reg
  Serial.print("indice_linha = ");
  Serial.println(indice_linha); // variavel anterior = sexo_paciente_reg
  Serial.print("loop0 = ");
  Serial.println(loop0);
  Serial.print("pagina = ");
  Serial.println(pagina);
  Serial.print("funcao = ");
  Serial.println(funcao);
  Serial.print("menu_inicial = ");
  Serial.println(menu_inicial);
  Serial.print("paciente_cadastrado = ");
  Serial.println(paciente_cadastrado);
  Serial.println();

while(loop0) {

  if(!digitalRead(desce))  desce_flag = true; // se a leitura digital do botão desce for diferente de "1" seta desce_flag
  delay(21);                                  // verificando quando o botão deixar de ser pressionado
  if(digitalRead(desce) && desce_flag) {      // se a variavel sobe E o flag estiverem em "1"...

      desce_flag = false;
      
      switch (funcao) {// mudar a variavel
    
        case 1:
            if (pagina == 1) { // pagina 1: escolha o paciente
                paciente--;
                if (paciente <= 1) paciente = 1;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
            else if (pagina == 3) { // pagina 3: escolha a idade do paciente
                idade--;
                if (idade <= 18) idade = 18;
                
                lcd.setCursor(6,2);
                lcd.print(idade);
            }
            else if (pagina == 4) { // pagina 4: escolha o peso do paciente
                peso--;
                if (peso <= 40) peso = 40; 
                lcd.setCursor(6,2);
                lcd.print(peso);
            }
            else if (pagina == 5) { // pagina 5: escolha a altura do paciente
                altura--;
                if (altura <= 100) altura = 100; 
                lcd.setCursor(6,2);
                lcd.print(altura);
            }
        break;


        case 2:
            if (pagina == 1) { // pagina 1: escolha o paciente para medir a força
                paciente--;
                if (paciente <= 1) paciente = 1;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
        break;

        case 3:
            if (pagina == 1) { // pagina 1: escolha o paciente
                paciente--;
                if (paciente <= 1) paciente = 1;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
                
               
            }
        break;

        case 4:
            if (pagina == 2) { // pagina 1: escolha o paciente
                paciente--;
                if (paciente <= 1) paciente = 1;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
            else if (pagina == 7) {
              if (indice_linha == 2) {
                idade--;
                if (idade <= 18) idade = 18;  
                lcd.setCursor(6,2);
                lcd.print(idade);
                }
            }
            else if (pagina == 8) {
              if (indice_linha == 2) {
                peso--;
                if (peso <= 40) peso = 40;   
                lcd.setCursor(6,2);
                lcd.print(peso);
                }    
            }
            else if (pagina == 9) { // pagina 9: escolha a altura do paciente
                altura--;
                if (altura <= 100) altura = 100; 
                lcd.setCursor(6,2);
                lcd.print(altura);
            }
              
        break;
      } // end switch (funcao)

      indice_linha++;                             // incrementa a variavel que indicara a linha a ser escrita
      menu_inicial = indice_linha;
      if(indice_linha >= limite_inferior) {       // se a variavel indicar um valor maior que a ultima linha...
        indice_linha = limite_inferior;           // mantem o valor para a ultima linha
        menu_inicial = limite_inferior;
        }

      linha_anterior_descida = indice_linha - 1;  // atribui a variavel o valor da linha que estava a seta
      lcd.setCursor(0,linha_anterior_descida);    // posiciona o cursor para a posicao que estava escrita a seta
      lcd.print(" ");                             // escreve um espaço no lugar, apagando a seta
      delay(50);
          
      lcd.setCursor(0,indice_linha);              // posiciona o cursor para a posicao que sera escrita a seta
      lcd.print(">");                             // escreve a seta
      delay(50);

   } // endif (digitalRead(desce) && desce_flag)

  if(!digitalRead(sobe))  sobe_flag = true;
  delay(21); 
  if(digitalRead(sobe) && sobe_flag) {

      sobe_flag = false;

      switch (funcao) {
    
        case 1:
            if (pagina == 1) {
                paciente++;
                if (paciente >= 20) paciente = 20;

                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
            else if (pagina == 3) { // pagina 3: escolha a idade do paciente
                idade++;
                if (idade >= 100) idade = 100;
                
                lcd.setCursor(6,2);
                lcd.print(idade);
            }
            else if (pagina == 4) { // pagina 4: escolha o peso do paciente
                peso++;
                if (peso >= 200) peso = 200;
                
                lcd.setCursor(6,2);
                lcd.print(peso);
            }
            else if (pagina == 5) { // pagina 5: escolha a altura do paciente
                altura++;
                if (altura >= 200) altura = 200; 
                lcd.setCursor(6,2);
                lcd.print(altura);
            }
        break;

        case 2:
            if (pagina == 1) { // pagina 1: escolha o paciente para medir a força
                paciente++;
                if (paciente >= 20) paciente = 20;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
        break;

        case 3:
            if (pagina == 1) { // pagina 1: escolha o paciente
                paciente++;
                if (paciente >= 20) paciente = 20;
                
                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);
            }
        break;

        case 4:
            if (pagina == 2) { // pagina 1: escolha o paciente
                paciente++;
                if (paciente >= 20) paciente = 20;

                lcd.clear();
                delay(3);
                lcd.setCursor(3,0);
                lcd.print("Selecione o");
                lcd.setCursor(4,1);
                lcd.print("Paciente:");
                lcd.setCursor(7,2);
                lcd.print(paciente);       
               
            }
            else if (pagina == 7) {
              if (indice_linha == 2) {
                idade++;
                if (idade >= 100) idade = 100;
                
                lcd.setCursor(6,2);
                lcd.print(idade);
                }
            }
            else if (pagina == 8) {
              if (indice_linha == 2) {
                peso++;
                if (peso >= 200) peso = 200;   
                lcd.setCursor(6,2);
                lcd.print(peso);
                }    
            }
            else if (pagina == 9) { // pagina 9: escolha a altura do paciente
                altura++;
                if (altura >= 200) altura = 200; 
                lcd.setCursor(6,2);
                lcd.print(altura);
            }
            
        break;
        
      } // end switch (funcao)
        
      indice_linha--;                           // incrementa a variavel que indicara a linha a ser escrita
      menu_inicial = indice_linha;
      if(indice_linha <= limite_superior) {     // se a variavel indicar um valor menor que a primeira linha...
          indice_linha = limite_superior;       // mantem o valor para a primeira linha
          menu_inicial = limite_superior;
        }

      linha_anterior_subida = indice_linha + 1; // atribui a variavel o valor da linha que estava a seta
      lcd.setCursor(0,linha_anterior_subida);   // posiciona o cursor para a posicao que estava escrita a seta
      lcd.print(" ");                           // escreve um espaço no lugar, apagando a seta
      delay(50);
      
      lcd.setCursor(0,indice_linha);            // posiciona o cursor para a posicao que sera escrita a seta
      lcd.print(">");                           // escreve a seta
      delay(50);     
      
   } // endif (digitalRead(sobe) && sobe_flag)

   if(!digitalRead(enter))  enter_flag = true;
   delay(21); 
   if(digitalRead(enter) && enter_flag) {

    enter_flag = false;

    switch (funcao) { //depois de apertar o enter

        case 0:

                switch (menu_inicial) {
                    case 0:
                        funcao++;
                    break;
                
                    case 1:
                        menu_inicial = 2;
                        funcao = menu_inicial;
                    break;
                
                    case 2:
                        menu_inicial = 3;
                        funcao = menu_inicial;
                    break;
                
                    case 3:
                        menu_inicial = 4;
                        funcao = menu_inicial;
                    break;
            }
            
        break;
    
        case 1:
            if (pagina == 1) { // pagina 1: escolha o paciente
                if (indice_linha == 2){ // confirmação
                    paciente_reg = paciente;
                    Serial.print("paciente_reg = ");
                    Serial.println(paciente_reg);
                    patient[paciente].registro = paciente_reg;
                    
                    switch (paciente_reg) {
                      case 1:
                      endereco_memoria = 0;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 2:
                      endereco_memoria = 9;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 3:
                      endereco_memoria = 18;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 4:
                      endereco_memoria = 27;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 5:
                      endereco_memoria = 36;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 6:
                      endereco_memoria = 45;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 7:
                      endereco_memoria = 54;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 8:
                      endereco_memoria = 63;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 9:
                      endereco_memoria = 72;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 10:
                      endereco_memoria = 81;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 11:
                      endereco_memoria = 90;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 12:
                      endereco_memoria = 99;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 13:
                      endereco_memoria = 108;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 14:
                      endereco_memoria = 117;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 15:
                      endereco_memoria = 126;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 16:
                      endereco_memoria = 135;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 17:
                      endereco_memoria = 144;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 18:
                      endereco_memoria = 153;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 19:
                      endereco_memoria = 162;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 20:
                      endereco_memoria = 171;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;
                      
                    }
                    
                    EEPROM.put(endereco_memoria,patient[paciente].registro);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("paciente escolhido = ");
                    Serial.println(paciente_reg);
                }
            }
            else if (pagina == 2) { // pagina 2: escolha o sexo do paciente
                if (indice_linha == 2) { // seta na linha 2: confirmação sexo feminino 
                    sexo_paciente_reg = 'F';
                    patient[paciente].sexo = sexo_paciente_reg;
                    endereco_memoria++;
                    Serial.print("endereco de memoria (sexo) = ");
                    Serial.println(endereco_memoria);
                     
                    EEPROM.put(endereco_memoria,patient[paciente].sexo);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("sexo do paciente = ");
                    Serial.println(EEPROM.get(endereco_memoria, sexo_paciente_reg));
                }
                else if (indice_linha == 3) { // seta na linha 3: confirmação sexo masculino 
                    sexo_paciente_reg = 'M';
                    patient[paciente].sexo = sexo_paciente_reg;
                    endereco_memoria++;
                    Serial.print("endereco de memoria (sexo) = ");
                    Serial.println(endereco_memoria);
                    
                    EEPROM.put(endereco_memoria,patient[paciente].sexo);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("sexo do paciente = ");
                    Serial.println(EEPROM.get(endereco_memoria, sexo_paciente_reg));  
                }
                
            }
            else if (pagina == 3) { // pagina 3: escolha a idade do paciente
                if (indice_linha == 2){ // confirmação
                    paciente_idade_reg = idade; 
                    patient[paciente].idade = paciente_idade_reg;
                    endereco_memoria++;
                    Serial.print("endereco de memoria (idade) = ");
                    Serial.println(endereco_memoria);
                    
                    EEPROM.put(endereco_memoria,patient[paciente].idade);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("idade do paciente = ");
                    Serial.print(EEPROM.get(endereco_memoria, paciente_idade_reg));
                    Serial.print(" anos");
                }
            }
            else if (pagina == 4) { // pagina 4: escolha o peso do paciente
                if (indice_linha == 2){ // confirmação
                    paciente_peso_reg = peso;
                    patient[paciente].peso = paciente_peso_reg;
                    endereco_memoria++;
                    Serial.print("endereco de memoria (peso) = ");
                    Serial.println(endereco_memoria);
                    
                    EEPROM.put(endereco_memoria,patient[paciente].peso);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("peso do paciente = ");
                    Serial.print(EEPROM.get(endereco_memoria, paciente_peso_reg));
                    Serial.println(" kg");
                }
            }
            else if (pagina == 5) { // pagina 5: escolha a altura do paciente
                if (indice_linha == 2){ // confirmação
                    paciente_altura_reg = altura;
                    patient[paciente].altura = paciente_altura_reg;
                    endereco_memoria++;
                    Serial.print("endereco de memoria (altura) = ");
                    Serial.println(endereco_memoria);
                    
                    EEPROM.put(endereco_memoria, patient[paciente].altura);
                    EEPROM.commit();
                    delay(5);
                    
                    Serial.println();
                    Serial.print("altura do paciente = ");
                    Serial.print(EEPROM.get(endereco_memoria, paciente_altura_reg));
                    Serial.println(" cm");
                }
            }
            else if (pagina == 6) { // pagina 6: confirme suas escolhas
                if (indice_linha == 2) { // seta na linha 2: confirmação 
                  
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print(" Cadastro do");
                    lcd.setCursor(0,1);
                    lcd.print(" Paciente"); //os tigrões e a tigresa orgulhosamente apresentam: 
                    lcd.setCursor(10,1);    //o dinamômetro digital do maroto!
                    lcd.print(paciente_reg);
                    lcd.setCursor(0,2);
                    lcd.print("  Concluido!");

                    paciente_cadastrado = true; // seta flag para indicar que o usuario foi cadastrado com sucesso
                    endereco_memoria += 2; // pula um endereço de memoria, que está reservado para armazenamento do valor da força medida
                    Serial.print("endereco de memoria (paciente_cadastrado) = ");
                    Serial.println(endereco_memoria);
                    
                    EEPROM.put(endereco_memoria, paciente_cadastrado);
                    EEPROM.commit();
                    delay(5);

                    Serial.println();
                    Serial.print("paciente_cadastrado = ");
                    Serial.print(EEPROM.get(endereco_memoria, paciente_cadastrado));
                    
                    endereco_memoria = 0; // zera endereço de memoria para reutilização
                    delay(3000);
                    
                    Serial.println();
                    Serial.println("todos os dados foram salvos.");
                    Serial.print("paciente = ");
                    Serial.println(patient[paciente].registro);
                    Serial.print("sexo do paciente = ");
                    Serial.println(patient[paciente].sexo);
                    Serial.print("idade do paciente = ");
                    Serial.print(patient[paciente].idade);
                    Serial.println(" anos");
                    Serial.print("peso do paciente = ");
                    Serial.print(patient[paciente].peso);
                    Serial.println(" kg");
                    Serial.print("altura do paciente = ");
                    Serial.print(patient[paciente].altura);
                    Serial.print(" cm"); 
                   

                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    paciente = 1;
                    idade = 18;
                    peso = 40;
                    altura = 100;
                    sexo_paciente_reg = 'N';
                    paciente_idade_reg = 0;
                    paciente_peso_reg = 0;
                    paciente_altura_reg = 0;
                    paciente_cadastrado = false;
                }
                else if (indice_linha == 3) { // seta na linha 3: negação
                   
                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    paciente = 1;
                    idade = 18;
                    peso = 40;
                    sexo_paciente_reg = 'N';
                    paciente_idade_reg = 0;
                    paciente_peso_reg = 0;
                    paciente_altura_reg = 0;
                    paciente_cadastrado = false;
                    lcd.clear();
                    
                    Serial.println();
                    Serial.println("nenhum dado foi salvo.");
                    Serial.print("paciente = ");
                    Serial.println(patient[paciente].registro);
                    Serial.print("sexo do paciente = ");
                    Serial.println(patient[paciente].sexo);
                    Serial.print("idade do paciente = ");
                    Serial.print(patient[paciente].idade);
                    Serial.println(" anos");
                    Serial.print("peso do paciente = ");
                    Serial.print(patient[paciente].peso);
                    Serial.println(" kg");
                    Serial.print("altura do paciente = ");
                    Serial.print(patient[paciente].altura);
                    Serial.print(" cm"); 
                     
                
                }
                
            }
        break;
    
        case 2:
            if (pagina == 1) { // pagina 1: escolha o paciente para medir a força
                if (indice_linha == 2){ // confirmação
                    paciente_reg = paciente;
                    patient[paciente].registro = paciente_reg;

                      switch (paciente_reg) {
                      case 1:
                      endereco_memoria = 5;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 2:
                      endereco_memoria = 14;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 3:
                      endereco_memoria = 23;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 4:
                      endereco_memoria = 32;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 5:
                      endereco_memoria = 41;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 6:
                      endereco_memoria = 50;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 7:
                      endereco_memoria = 59;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 8:
                      endereco_memoria = 68;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 9:
                      endereco_memoria = 77;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 10:
                      endereco_memoria = 86;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 11:
                      endereco_memoria = 95;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 12:
                      endereco_memoria = 104;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 13:
                      endereco_memoria = 113;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 14:
                      endereco_memoria = 122;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 15:
                      endereco_memoria = 131;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 16:
                      endereco_memoria = 140;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 17:
                      endereco_memoria = 149;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 18:
                      endereco_memoria = 158;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 19:
                      endereco_memoria = 167;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 20:
                      endereco_memoria = 176;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;
                      
                    }
                    
                    Serial.println();
                    Serial.print("paciente escolhido = ");
                    Serial.println(patient[paciente].registro);
                }
            }
            else if (pagina == 3) { // pagina 5: confirme sua medição
                if (indice_linha == 2) { // seta na linha 2: confirmação
                    paciente_forca_reg = forca_pico;
                    forca_arredondado = arredondar(paciente_forca_reg);
                    Serial.print("forca_arredondado [int] = ");
                    Serial.println(forca_arredondado);
                    Serial.print("tamanho da variavel forca_arredondado [int] na EEPROM = ");
                    Serial.print(sizeof(forca_arredondado));
                    Serial.println(" bytes");
                    
                    patient[paciente].forca = byte(forca_arredondado);
                    Serial.print("patient[paciente].forca [char] = ");
                    Serial.println(patient[paciente].forca);
                    Serial.print("tamanho da variavel patient[paciente].forca [char] na EEPROM = ");
                    Serial.print(sizeof(patient[paciente].forca));
                    Serial.println(" bytes");
                    
                    EEPROM.put(endereco_memoria, patient[paciente].forca);
                    EEPROM.commit();
                    delay(5);
                    Serial.print("endereco de memoria atual = ");
                    Serial.println(endereco_memoria);
                    
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("    Medicao");
                    lcd.setCursor(0,1);
                    lcd.print("    Salva!");
                    delay(2000);
                    
                    Serial.println();
                    Serial.println("medição de força salvo.");
                    Serial.print("paciente = ");
                    Serial.println(patient[paciente].registro);
                    Serial.print("sexo do paciente = ");
                    Serial.println(patient[paciente].sexo);
                    Serial.print("idade do paciente = ");
                    Serial.print(patient[paciente].idade);
                    Serial.println(" anos");
                    Serial.print("peso do paciente = ");
                    Serial.print(patient[paciente].peso);
                    Serial.println("kg");
                    Serial.print("Força medida: ");
                    Serial.print(patient[paciente].forca); 
                    Serial.println("kgf");

                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    endereco_memoria = 0;
                    forca_medida = 0.0;
                    forca_pico = 0.0;
                    paciente = 1;
                    paciente_forca_reg = 0.0;
                    
                }
                else if (indice_linha == 3) { // seta na linha 3: negação
                   
                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    endereco_memoria = 0;
                    forca_medida = 0.0;
                    forca_pico = 0.0;
                    paciente = 1;
                    paciente_forca_reg = 0.0;
                    
                    lcd.clear();
                    
                    Serial.println();
                    Serial.println("nenhum dado foi salvo.");
                    Serial.print("paciente = ");
                    Serial.println(paciente_reg);
                    Serial.print("sexo do paciente = ");
                    Serial.println(sexo_paciente_reg);
                    Serial.print("idade do paciente = ");
                    Serial.print(paciente_idade_reg);
                    Serial.println(" anos");
                    Serial.print("peso do paciente = ");
                    Serial.print(paciente_peso_reg);
                    Serial.print("kg");
                    Serial.print("Força medida: ");
                    Serial.print(patient[paciente].forca); 
                    Serial.println("kgf");  
                }
                
            }
        break;

        case 3:
            if (pagina == 1) { // pagina 1: escolha o paciente
                if (indice_linha == 2){ // confirmação
                    paciente_reg = paciente;
                    Serial.print("paciente_reg = ");
                    Serial.println(paciente_reg);
                    patient[paciente].registro = paciente_reg;

                    switch (paciente_reg) {
                      case 1:
                      endereco_memoria = 0;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 2:
                      endereco_memoria = 9;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 3:
                      endereco_memoria = 18;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 4:
                      endereco_memoria = 27;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 5:
                      endereco_memoria = 36;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 6:
                      endereco_memoria = 45;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 7:
                      endereco_memoria = 54;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 8:
                      endereco_memoria = 63;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 9:
                      endereco_memoria = 72;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 10:
                      endereco_memoria = 81;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 11:
                      endereco_memoria = 90;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 12:
                      endereco_memoria = 99;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 13:
                      endereco_memoria = 108;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 14:
                      endereco_memoria = 117;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 15:
                      endereco_memoria = 126;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 16:
                      endereco_memoria = 135;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 17:
                      endereco_memoria = 144;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 18:
                      endereco_memoria = 153;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 19:
                      endereco_memoria = 162;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 20:
                      endereco_memoria = 171;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;
                      
                    }
                    
                    endereco_memoria += 6; // pula 6 endereços de memoria, para pegar flag de usuario cadastrado
                    pac_cad = EEPROM.get(endereco_memoria, paciente_cadastrado);
                    Serial.print("pac_cad = ");
                    Serial.println(pac_cad);
                    delay(100);
                    
                      if (!pac_cad) { // se a flag estiver em '0' (false)...
                          Serial.println("paciente não cadastrado, zerar variaveis");
                          patient[paciente].sexo = 'N';
                          patient[paciente].idade = 0;
                          patient[paciente].peso = 0;
                          patient[paciente].altura = 0;
                          patient[paciente].forca = 0;
                          delay(100);
                      }
                      else { // se não (flag está em '1' (true)), carregar valores guardados na EEPROM
                          Serial.println("paciente cadastrado, mostrar variavies salvas");
                          endereco_memoria -= 6; // volta para endereço de memoria anterior (registro)
                          endereco_memoria++; // vai para proximo endereço de memoria (sexo)
                          Serial.print("endereco de memoria atual (sexo) = ");
                          Serial.println(endereco_memoria);
                          
                          patient[paciente].sexo = EEPROM.get(endereco_memoria, sexo_paciente_reg);
                          endereco_memoria++; // vai para proximo endereço de memoria (idade)
                          
                          Serial.print("endereco de memoria atual (idade) = ");
                          Serial.println(endereco_memoria);
                          patient[paciente].idade = EEPROM.get(endereco_memoria,paciente_idade_reg);
                          endereco_memoria++; // vai para proximo endereço de memoria (peso)
                          
                          Serial.print("endereco de memoria atual (peso) = ");
                          Serial.println(endereco_memoria);
                          patient[paciente].peso = EEPROM.get(endereco_memoria,paciente_peso_reg);
                          endereco_memoria++; // vai para proximo endereço de memoria (altura)
                          
                          Serial.print("endereco de memoria atual (altura) = ");
                          Serial.println(endereco_memoria);                    
                          patient[paciente].altura = EEPROM.get(endereco_memoria,paciente_altura_reg);
                          endereco_memoria++; // vai para proximo endereço de memoria (força)
                          
                          Serial.print("endereco de memoria atual (forca) = ");
                          Serial.println(endereco_memoria);
                          patient[paciente].forca = EEPROM.get(endereco_memoria,paciente_forca_type);
                      }

                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("-> Paciente ");
                    lcd.setCursor(12,0);
                    lcd.print(patient[paciente].registro);
                    lcd.setCursor(14,0);
                    lcd.print("<-");
                    lcd.setCursor(0,1);
                    lcd.print("Sexo");
                    lcd.setCursor(6,1);
                    lcd.print(patient[paciente].sexo);  
                    lcd.setCursor(0,2);
                    lcd.print("Idade");
                    lcd.setCursor(6,2);
                    lcd.print(patient[paciente].idade);
                    lcd.setCursor(10,2);
                    lcd.print("anos");
                    lcd.setCursor(0,3);
                    lcd.print("Peso");
                    lcd.setCursor(6,3);
                    lcd.print(patient[paciente].peso);
                    lcd.setCursor(10,3);
                    lcd.print("kg");
                    delay(5000);
                    lcd.clear();
                    
                    lcd.setCursor(0,0);
                    lcd.print("-> Paciente ");
                    lcd.setCursor(12,0);
                    lcd.print(paciente_reg);
                    lcd.setCursor(14,0);
                    lcd.print("<-");
                    lcd.setCursor(0,2);
                    lcd.print("Altura");
                    lcd.setCursor(7,2);
                    lcd.print(patient[paciente].altura);
                    lcd.setCursor(13,2);
                    lcd.print("cm");
                    lcd.setCursor(0,3);
                    lcd.print("Forca");
                    lcd.setCursor(7,3);
                    lcd.print(patient[paciente].forca);
                    lcd.setCursor(13,3);
                    lcd.print("kgf");
                    delay(5000);
                    lcd.clear();
                    
                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    endereco_memoria = 0;
                    
                    Serial.println();
                    Serial.print("paciente escolhido = ");
                    Serial.println(patient[paciente].registro);
                    Serial.println(patient[paciente].sexo);
                    Serial.println(patient[paciente].idade);
                    Serial.println(patient[paciente].peso);
                    Serial.println(patient[paciente].altura);
                    Serial.println(patient[paciente].forca);
                    Serial.print("endereco de memoria atual = ");
                    Serial.println(endereco_memoria);
                }
            }
        break;

        case 4:
            if (pagina == 1) {
                if (indice_linha == 0) { // confirmação importar dados
                   lcd.clear();
                   lcd.setCursor(0,1);
                   lcd.print("   Exportando");
                   lcd.setCursor(0,2);
                   lcd.print("    Dados");
                    
                  delay(8000);
                  Serial.println("CLEARDATA");
                  Serial.println("LABEL,Hora,Paciente,Sexo,Idade,Peso,Altura,Forca");
                  
                  endereco_memoria = 0;
                  
                  for(paciente_reg = 1; paciente_reg <=20; paciente_reg++){
                    
                    Serial.print("DATA,TIME,"); 
                    patient[paciente].registro = EEPROM.get(endereco_memoria,patient[paciente].registro);
                    Serial.print(patient[paciente].registro);
                     
                    endereco_memoria++;
                    Serial.print(",");
                    patient[paciente].sexo = EEPROM.get(endereco_memoria,patient[paciente].sexo);
                    Serial.print(patient[paciente].sexo);
                      
                    endereco_memoria++;
                    Serial.print(","); 
                    patient[paciente].idade = EEPROM.get(endereco_memoria,patient[paciente].idade);
                    Serial.print (patient[paciente].idade); 
                    
                    endereco_memoria++;
                    Serial.print(","); 
                    patient[paciente].peso = EEPROM.get(endereco_memoria,patient[paciente].peso);
                    Serial.print (patient[paciente].peso); 
                    
                    endereco_memoria++;
                    Serial.print(","); 
                    patient[paciente].altura = EEPROM.get(endereco_memoria,patient[paciente].altura);
                    Serial.print (patient[paciente].altura);
                    
                    endereco_memoria++;
                    Serial.print(","); 
                    patient[paciente].forca = EEPROM.get(endereco_memoria,patient[paciente].forca);
                    Serial.print (patient[paciente].forca);  
                    
                    endereco_memoria +=4;
                    Serial.println();
                    delay(1000);
                   
                  }//fim do for
                   
                    lcd.clear();
                    lcd.setCursor(0,1);
                    lcd.print(" Transferencia");
                    lcd.setCursor(0,2);
                    lcd.print("   Concluida!");
                    delay(5000);


                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                }
            }
            else if (pagina == 2) { // pagina 2: escolha o paciente
                if (indice_linha == 2){ // confirmação
                    paciente_reg = paciente;
                    patient[paciente].registro = paciente_reg;
                    
                    switch (paciente_reg) {
                      case 1:
                      endereco_memoria = 0;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 2:
                      endereco_memoria = 9;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 3:
                      endereco_memoria = 18;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 4:
                      endereco_memoria = 27;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 5:
                      endereco_memoria = 36;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 6:
                      endereco_memoria = 45;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 7:
                      endereco_memoria = 54;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 8:
                      endereco_memoria = 63;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 9:
                      endereco_memoria = 72;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 10:
                      endereco_memoria = 81;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 11:
                      endereco_memoria = 90;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 12:
                      endereco_memoria = 99;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 13:
                      endereco_memoria = 108;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 14:
                      endereco_memoria = 117;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 15:
                      endereco_memoria = 126;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 16:
                      endereco_memoria = 135;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 17:
                      endereco_memoria = 144;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 18:
                      endereco_memoria = 153;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 19:
                      endereco_memoria = 162;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;

                      case 20:
                      endereco_memoria = 171;
                      Serial.print("endereco de memoria = ");
                      Serial.println(endereco_memoria);
                      break;
                      
                    }
                    
                    Serial.println();
                    Serial.print("paciente escolhido = ");
                    Serial.println(EEPROM.get(endereco_memoria, paciente_reg)); // pega o valor armazenado de registro do paciente);

                    endereco_memoria += 6; // pula 6 endereços de memoria, para pegar flag de usuario cadastrado
                     Serial.print("endereco de memoria = ");
                     Serial.println(endereco_memoria);
                    pac_cad = EEPROM.get(endereco_memoria, paciente_cadastrado);
                    Serial.print("pac_cad = ");
                    Serial.println(pac_cad);
                    delay(100);
                    
                      if (!pac_cad) { // se a flag estiver em '0' (false), atribuir caracter 'N' para a variável patient[paciente].sexo
                          Serial.println("paciente não cadastrado, sexo = N");
                          patient[paciente].sexo = 'N';
                          delay(100);
                      }
                   
                    endereco_memoria = endereco_memoria -6; // volta para endereco de memoria anterior 
                    Serial.println(endereco_memoria);
                    endereco_memoria++; // vai para proximo endereço de memoria (sexo)
                    Serial.print("endereco de memoria atual (sexo) = ");
                    Serial.println(endereco_memoria);
                          
                    patient[paciente].sexo = EEPROM.get(endereco_memoria, sexo_paciente_reg);
                    endereco_memoria++; // vai para proximo endereço de memoria (idade)
                          
                    Serial.print("endereco de memoria atual (idade) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].idade = EEPROM.get(endereco_memoria,paciente_idade_reg);
                    endereco_memoria++; // vai para proximo endereço de memoria (peso)
                          
                    Serial.print("endereco de memoria atual (peso) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].peso = EEPROM.get(endereco_memoria,paciente_peso_reg);
                    endereco_memoria++; // vai para proximo endereço de memoria (altura)
                          
                    Serial.print("endereco de memoria atual (altura) = ");
                    Serial.println(endereco_memoria);                    
                    patient[paciente].altura = EEPROM.get(endereco_memoria,paciente_altura_reg);
                    endereco_memoria++; // vai para proximo endereço de memoria (força)
                          
                    Serial.print("endereco de memoria atual (forca) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].forca = EEPROM.get(endereco_memoria,paciente_forca_type);
                    endereco_memoria = endereco_memoria -5;
                }
            }
            else if (pagina == 3) { // pagina 3: apagar dados | editar dados | formatar dados
                if (indice_linha == 0){ // confirmação apagar dados
                    paciente_reg = paciente;
                    patient[paciente].registro = paciente_reg;
                    Serial.println();
                    Serial.print("paciente escolhido para apagar dados = ");
                    Serial.println(paciente_reg);
                }
                else if (indice_linha == 1) { // confirmação editar dados
                  pagina++; 
                }
                else if (indice_linha == 2) { // confirmação formatar dados
                  pagina = 9; 
                }
            }
            else if (pagina == 4) { // pagina 4: apagar dados do paciente escolhido
                if (indice_linha == 2){ // confirmação apagar dados

                    Serial.print("endereco de memoria atual (registro) = ");
                    Serial.println(endereco_memoria);
                    EEPROM.put(endereco_memoria, patient[paciente].registro);
                    EEPROM.commit();
                    delay(5);
                    
                    patient[paciente].registro = EEPROM.get(endereco_memoria, paciente_reg); // pega o valor armazenado de registro do paciente
                    endereco_memoria++; // vai para proximo endereço de memoria (sexo)
                    

                    Serial.print("endereco de memoria atual (sexo) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].sexo = 'N';
                    EEPROM.put(endereco_memoria, patient[paciente].sexo);
                    EEPROM.commit();
                    delay(5);
                    endereco_memoria++;

                    Serial.print("endereco de memoria atual (idade) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].idade = 0;
                    EEPROM.put(endereco_memoria, patient[paciente].idade);
                    EEPROM.commit();
                    delay(5);
                    endereco_memoria++;

                    Serial.print("endereco de memoria atual (peso) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].peso = 0;
                    EEPROM.put(endereco_memoria, patient[paciente].peso);
                    EEPROM.commit();
                    delay(5);
                    endereco_memoria++;

                    Serial.print("endereco de memoria atual (altura) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].altura = 0;
                    EEPROM.put(endereco_memoria, patient[paciente].altura);
                    EEPROM.commit();
                    delay(5);
                    endereco_memoria++;

                    Serial.print("endereco de memoria atual (forca) = ");
                    Serial.println(endereco_memoria);
                    patient[paciente].forca = 0;
                    EEPROM.put(endereco_memoria, patient[paciente].forca);
                    EEPROM.commit();
                    delay(5);
                    endereco_memoria++;

                    Serial.print("endereco de memoria atual (paciente_cadastrado) = ");
                    Serial.println(endereco_memoria);
                    paciente_cadastrado = false;
                    EEPROM.put(endereco_memoria, paciente_cadastrado);
                    EEPROM.commit();
                    delay(5);
                    
                    lcd.clear();
                    lcd.setCursor(4,0);
                    lcd.print("Excluido");
                    lcd.setCursor(6,1);
                    lcd.print("com");
                    lcd.setCursor(4,2);
                    lcd.print("Sucesso.");
                    delay(3000);
                    
                    Serial.println("Todos os dados foram apagados.");
                    Serial.println(patient[paciente].registro);
                    Serial.println(patient[paciente].sexo);
                    Serial.println(patient[paciente].idade);
                    Serial.println(patient[paciente].peso);
                    Serial.println(patient[paciente].altura);
                    Serial.println(patient[paciente].forca);

                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    endereco_memoria = 0;
                }
                else if(indice_linha == 3){ // não apagar dados
                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                    endereco_memoria = 0;
                }
            }
            else if(pagina == 5){
                if(indice_linha == 0){
                      lcd.clear();
                      lcd.setCursor(0,0);
                      lcd.print(" Defina o sexo");
                      lcd.setCursor(0,1);
                      lcd.print(" do Paciente:");
                      lcd.setCursor(0,2);
                      lcd.print(">");
                      lcd.setCursor(3,2);
                      lcd.print("Feminino");
                      lcd.setCursor(3,3);
                      lcd.print("Masculino");
                      delay(1000);
                }
                      
                   else if (indice_linha == 1){
                     lcd.clear();
                     lcd.setCursor(0,0);
                     lcd.print(" Defina a idade");
                     lcd.setCursor(0,1);
                     lcd.print("  do Paciente:");
                     lcd.setCursor(0,2);
                     lcd.print(">");
                     lcd.setCursor(6,2);
                     lcd.print(idade);
                     lcd.setCursor(6,3);
                     lcd.print ("anos");
                     pagina++;
                    delay(1000);
                    }

                    else if (indice_linha == 2){
                     lcd.clear();
                     lcd.setCursor(0,0);
                     lcd.print(" Defina o peso");
                     lcd.setCursor(0,1);
                     lcd.print(" do Paciente:");
                     lcd.setCursor(0,2);
                     lcd.print(">");
                     lcd.setCursor(6,2);
                     lcd.print(peso);
                     lcd.setCursor(6,3);
                     lcd.print ("kg");
                     pagina = pagina + 2;
                     delay(1000);
                     }

                    else if (indice_linha == 3){
                      lcd.clear();
                      lcd.setCursor(0,0);
                      lcd.print(" Defina a altura");
                      lcd.setCursor(0,1);
                      lcd.print("  do Paciente:");
                      lcd.setCursor(0,2);
                      lcd.print(">");
                      lcd.setCursor(6,2);
                      lcd.print(altura);
                      lcd.setCursor(6,3);
                      lcd.print ("cm");
                      pagina = pagina + 3;
                      delay(1000);
                    }
                 }
             else if (pagina == 6) {
                if (indice_linha == 2) { // seta na linha 2: confirmação sexo feminino 
                    sexo_paciente_reg = 'F';
                    patient[paciente].sexo = sexo_paciente_reg;
                    endereco_memoria++;
                    Serial.print("endereco_memoria (sexo) = ");
                    Serial.println(endereco_memoria);
                    EEPROM.put(endereco_memoria, patient[paciente].sexo);
                    EEPROM.commit();
                    delay(5);
                    
                    lcd.clear();
                    lcd.setCursor(3,0);
                    lcd.print("Dado alterado");
                    lcd.setCursor(3,1);
                    lcd.print ("para = ");
                    lcd.setCursor(10,1);
                    lcd.print (patient[paciente].sexo);
                    delay(5000);
                    
                    Serial.println();
                    Serial.print("sexo do paciente = ");
                    Serial.println(patient[paciente].sexo);

                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                }
                else if (indice_linha == 3) { // seta na linha 3: confirmação sexo masculino 
                    sexo_paciente_reg = 'M';
                    patient[paciente].sexo = sexo_paciente_reg;
                    endereco_memoria++;
                    Serial.print("endereco_memoria (sexo) = ");
                    Serial.println(endereco_memoria);
                    EEPROM.put(endereco_memoria, patient[paciente].sexo);
                    EEPROM.commit();
                    delay(5);
                    
                    lcd.clear();
                    lcd.setCursor(3,0);
                    lcd.print("Dado alterado");
                    lcd.setCursor(3,1);
                    lcd.print ("para = ");
                    lcd.setCursor(10,1);
                    lcd.print (patient[paciente].sexo);
                    delay(5000);
                    
                    Serial.println();
                    Serial.print("sexo do paciente = ");
                    Serial.println(patient[paciente].sexo);

                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                }
             }
              else if (pagina == 7) {
                if (indice_linha == 2){ // confirmação
                    paciente_idade_reg = idade; 
                    patient[paciente].idade = paciente_idade_reg;
                    endereco_memoria += 2;
                    Serial.print("endereco_memoria (idade) = ");
                    Serial.println(endereco_memoria);
                    EEPROM.put(endereco_memoria, patient[paciente].idade);
                    EEPROM.commit();
                    delay(5);
                    
                    lcd.clear();
                    lcd.setCursor(3,0);
                    lcd.print("Dado alterado");
                    lcd.setCursor(3,1);
                    lcd.print ("para = ");
                    lcd.setCursor(10,1);
                    lcd.print (patient[paciente].idade);
                    delay(5000);
                    
                    Serial.println();
                    Serial.print("idade do paciente = ");
                    Serial.print(patient[paciente].idade);
                    Serial.print(" anos");
                    
                    funcao = 0;
                    menu_inicial = 0;
                    pagina = 0;
                }
               }
                else if (pagina == 8) {
                    if (indice_linha == 2){ // confirmação
                        paciente_peso_reg = peso; 
                        patient[paciente].peso = paciente_peso_reg;
                        endereco_memoria += 3;
                        Serial.print("endereco_memoria (peso) = ");
                        Serial.println(endereco_memoria);
                        EEPROM.put(endereco_memoria, patient[paciente].peso);
                        EEPROM.commit();
                        delay(5);
                    
                        lcd.clear();
                        lcd.setCursor(3,0);
                        lcd.print("Dado alterado");
                        lcd.setCursor(3,1);
                        lcd.print ("para = ");
                        lcd.setCursor(10,1);
                        lcd.print (patient[paciente].peso);
                        delay(5000);
                        
                        Serial.println();
                        Serial.print("peso do paciente = ");
                        Serial.print(patient[paciente].peso);
                        Serial.print(" kg");
    
                        funcao = 0;
                        menu_inicial = 0;
                        pagina = 0;
                    }
                }
                else if (pagina == 9) {
                    if (indice_linha == 2){ // confirmação
                        paciente_altura_reg = altura; 
                        patient[paciente].altura = paciente_altura_reg;
                        endereco_memoria += 4;
                        Serial.print("endereco_memoria (altura) = ");
                        Serial.println(endereco_memoria);
                        EEPROM.put(endereco_memoria, patient[paciente].altura);
                        EEPROM.commit();
                        delay(5);
                        
                        lcd.clear();
                        lcd.setCursor(3,0);
                        lcd.print("Dado alterado");
                        lcd.setCursor(3,1);
                        lcd.print ("para = ");
                        lcd.setCursor(10,1);
                        lcd.print (patient[paciente].altura);
                        delay(5000);
                        
                        Serial.println();
                        Serial.print("altura do paciente = ");
                        Serial.print(patient[paciente].altura);
                        Serial.print(" cm");
    
                        funcao = 0;
                        menu_inicial = 0;
                        pagina = 0;
                    }
               }
               else if (pagina == 10) { // pagina 10: confirmação formatar dados
                    if (indice_linha == 2){ // confirmação: apague tudo

                        int loading_col = 0;
                        int loading_lin = 2;
                        lcd.clear();
                        lcd.setCursor(0,0);
                        lcd.print("Nao desligue o");
                        lcd.setCursor(0,1);
                        lcd.print("Dispositivo");
                        lcd.setCursor(0,2);
                        lcd.print("antes do termino");
                        lcd.setCursor(0,3);
                        lcd.print("da operacao.");
                        delay(4000);
                        
                        lcd.clear();
                        lcd.setCursor(0,0);
                        lcd.print("Apagando dados.");
                        lcd.setCursor(0,1);
                        lcd.print("Aguarde...");
                        delay(1000);

                              for (endereco_memoria = 0; endereco_memoria <= 180; endereco_memoria ++) {
                                  EEPROM.put(endereco_memoria, 0);
                                  EEPROM.commit();
                                  delay(5);

                                  lcd.setCursor(loading_col,loading_lin);
                                  lcd.print("*");
                                  loading_col++;

                                    if (loading_lin == 3 && loading_col == 16){
                                        lcd.setCursor(0,2);
                                        lcd.print("                ");
                                        lcd.setCursor(0,3);
                                        lcd.print("                ");
                                        loading_col = 0;
                                        loading_lin = 2;
                                        delay(95);
                                    }
                                    
                                    if (loading_col >= 16) {
                                      loading_lin++;
                                      loading_col = 0;
                                    }
                                
                              }
                     delay(100);   
                    }
                    else if (indice_linha == 3){ // negação: volta ao menu inicial
                        lcd.clear();
                        funcao = 0;
                        menu_inicial = 0;
                        pagina = 0;
                        delay(100);
                    }
               }
             
        break;
    } // end switch (funcao)

    loop0 = false;
    pagina++;
      
    } // end if enter && enter_flag

    if(!digitalRead(esc))  esc_flag = true;
    delay(21);
    if(digitalRead(esc) && esc_flag) {
      
    esc_flag = false;
    loop0 = false;
    funcao = 0;
    menu_inicial = 0;
    pagina = 0;
    paciente = 1;
    lcd.clear();
    
    } // end if esc e esc_flag 
         
 } // end while(loop0)
   
} // end void leitura_botoes()

//algoritmo para arredondamento de valor float
int arredondar( float valor ) {

    int resultado, sobra;                                                     // variáveis usadas pela função
    
    resultado = valor * 100;            // 1 - multiplicar valor da força medida por 100, guardar em 'resultado'
    Serial.print("resultado = ");
    Serial.println(resultado);
    
    sobra = resultado % 100;            // 2 - dividir o valor da força medida por 100, guarda o resto da divisão em 'sobra'
    Serial.print("sobra = ");
    Serial.println(sobra);

      Serial.print("sobra e resultado final = ");
      if (sobra <= 49) {                // 3.1 - se o valor da sobra for menor ou igual a 49 (ou seja, quando o valor fracionario for 0,49 ou menos)
          sobra = 0;                    // 3.1.1 - zera o resto, e não altera o resultado
          Serial.println(sobra);
          
      }
      else {                            // 3.2 - se o valor da sobra for maior que 49 (ou seja, quando o valor fracionario for 0,50 ou mais) 
          resultado = resultado + 100;  // 3.2.1 - soma 100 ao resultado (ou seja, 1 no valor final)
          sobra = 0;                    // 3.2.2 - zera o resto
          Serial.println(sobra);
          Serial.print("resultado calculado = ");
          Serial.println(resultado);
      }

      resultado /= 100;                 // 4 - para voltar ao valor "normal", dividir o resultado por 100, e guardar nele mesmo
      Serial.print("resultado final = ");
      Serial.println(resultado);
      Serial.println();
      
      return resultado;                 // 5 - retornar ao codigo-fonte com o valor arredondado
}// end int arredondar()

/* O Serial.print ao longo do código serve como forma acompanhar o código e como auxilio para futuras alterações
 * Utilizamos o Serial.print também para mandar os valores armazenados na memória para uma planilha no excel 
 * A Biblioteca EEPROM está desatualizada e não recomendamos o uso futuramente (procurar pela lib "Preferences").
 */
