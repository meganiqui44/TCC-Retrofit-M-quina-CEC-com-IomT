



#include "EmonLib.h" //INCLUSÃO DE BIBLIOTECA
#include <LiquidCrystal.h>
#define CURRENT_CAL 20 //VALOR DE CALIBRAÇÃO (DEVE SER AJUSTADO EM PARALELO COM UM MULTÍMETRO MEDINDO A CORRENTE DA CARGA)

#define WAIT 500 // Delay

#define automaticRPM 1500
#define TAMANHODOCIRCULO 12
String c;
String rotacaoin;
//TFT_HX8357 tft = TFT_HX8357(); //Chama a biblioteca do display


const int rs = 22, en = 23, d4 = 24, d5 = 25, d6 = 26, d7 = 27;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
float saidaC;











uint32_t runTime = 0;

const int MANUALPINFINO = A0, MANUALPINGROSSO = A1, pinoSensorDeCorrente = A2;//Portas Analógicas

const int ONOFFPIN = 4, MODEPIN = 2, saidaPin = 7, LIGeDESPIN = 18, sensorPin1 = 19, sensorPin2 = 20, sensorPin3 = 21,APP=6; //Portas Digitais
int       ONOFF       , MODE,        saida,        LIGeDES,          APPSELECT; 

int  sensorHallPulses,rotacaopulsos; //Valores calculados
float manualRPM, auxHZ, HZ, RPM, graficAux; //Valores Calculados

double atualTempoDePulso, antigoTempoDePulso;// atualTempoDoDisplay, antigoTempoDoDisplay; //Tempo

int Pulsos,rpmpulso, CORDOCIRCULO, desenhando,  tempoDeAmostragem = 200, rotacaoEsperada; // CORMANUAL, CORAUTOMATICO, Variaveis Auxiliares

EnergyMonitor emon1; //CRIA UMA INSTÂNCIA

void setup(){ //A funcao setup é iniciada automaticamente uma vez ao ligar o arduino
                //Definiçõs iniciais
                pinMode(MODEPIN,INPUT_PULLUP); //Deixa o pino normalmente em nível lógico alto
                pinMode(LIGeDESPIN,INPUT_PULLUP); //Deixa o pino normalmente em nível lógico alto
                pinMode(APP,INPUT); 
                lcd.begin(16, 2);

                randomSeed(analogRead(0));
                Serial.begin(19200); // Tempo de processamnto do Arduino
                

                ChecarModo();
                ChecarSeEstaLigado();

                //Calculos auxiliares
                auxHZ = 0.027 * tempoDeAmostragem; //O valor 0.012 equivale a (4 Pulsos por volta * 3 Sensores / 1000 para converter a unidade de ms para s)

                //Interrupções
                attachInterrupt(digitalPinToInterrupt(sensorPin1), HallPulse, RISING); //Interrupção para contar pulsos do sensor hall
                //attachInterrupt(digitalPinToInterrupt(sensorPin2), HallPulse, RISING); //Interrupção para contar pulsos do sensor hall
                //attachInterrupt(digitalPinToInterrupt(sensorPin3), HallPulse, RISING); //Interrupção para contar pulsos do sensor hall
                attachInterrupt(digitalPinToInterrupt(MODEPIN), ChecarModo, CHANGE); //Interrupção para saber que o modo foi alterado
                attachInterrupt(digitalPinToInterrupt(LIGeDESPIN), ChecarSeEstaLigado, CHANGE);  //Interrupção para saber que o motor foi ligado ou desligado

                //Calibrar Sensor de Corrente
                emon1.current(pinoSensorDeCorrente, CURRENT_CAL); //PASSA PARA A FUNÇÃO OS PARÂMETROS (PINO ANALÓGIO / VALOR DE CALIBRAÇÃO)

 
             }

void HallPulse(){     sensorHallPulses ++;
                      atualTempoDePulso = millis(); //Conta variação de tempo
                     if (atualTempoDePulso - antigoTempoDePulso > tempoDeAmostragem){ 
                                                                                        RPM = (60*sensorHallPulses) / auxHZ; 
                                                                                        Pulsos = sensorHallPulses;
                                                                                      
                                                                                        antigoTempoDePulso = atualTempoDePulso;
                                                                                        sensorHallPulses = 0;
                                                                                        if(RPM>2){
                                                                                          rotacaopulsos=RPM;
                                                                                        }
                       }
                  }


void ChecarModo(){  MODE = digitalRead(MODEPIN); //Ler se o modo é automatico ou manual onde GND = Automatico
                    if (MODE == 0){ 
                                      rotacaoEsperada = automaticRPM;
                                  }                
                 }
                     

void ChecarSeEstaLigado(){    LIGeDES = digitalRead(LIGeDESPIN); // Ler se a bomba esta ligada ou não
                              if (LIGeDES == 0){ 
                                                   ONOFF = 255;
                                                  
                                               }
                                               
                              else             {  
                                                   ONOFF = 0;
                                                  
                                               }
                              analogWrite (ONOFFPIN, ONOFF);

                         }


void loop(){     //LerCorrente


APPSELECT = digitalRead(APP);

if(APPSELECT == HIGH){

  



                 emon1.calcVI(20,100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)
                 double currentDraw = emon1.Irms; //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO

                 
                 //Serial.println(currentDraw); //IMPRIME NA SERIAL O VALOR DE CORRENTE MEDIDA
                 //Serial.print(" ");
                 graficAux = saida*20;
                 //Serial.println(graficAux); //IMPRIME O TEXTO NA SERIAL
              
                 
                 //Atualizar o valor do potênciometro
                  manualRPM  = (((analogRead(MANUALPINGROSSO)* 6)+ analogRead(MANUALPINFINO))/7)*3; // Define uma sensibilidade de 6 para 1 entre os potenciometros
                 
                 if (MODE == 1){    rotacaoEsperada = manualRPM;
                               }
                 else if(MODE ==0){
                  rotacaoEsperada = automaticRPM;
                 }

                 if (ONOFF == 0){
                  RPM=0;
                  saida =0;
                 }
                 
                 //Controle de Rotação
                 saida = (rotacaoEsperada / 3 /4 /2);
                 if (rotacaoEsperada > 2500){
                 saida =  saida-6;
                 }
                 else if(rotacaoEsperada > 1800){
                  saida = saida -5;
                 }
                 else if(rotacaoEsperada > 1200){
                  saida = saida -5;
                 }
                 else if(rotacaoEsperada > 610){
                  saida = saida -6;
                 }
                 else if(rotacaoEsperada > 150){
                  saida = saida -7;
                 }
                 else {
                  saida = 0;
                  manualRPM = 150;
                 }
              //  Serial.print(saida); // variando entre 0 a 127
                 analogWrite(saidaPin, saida);
                
                /* lcd.setCursor(0, 1);                                             
                 lcd.print( manualRPM/1.268181);    */             
                 if(rotacaopulsos<1000){
                    lcd.setCursor(3, 0);
                    lcd.print(" "); 
                    }  
                 lcd.setCursor(7, 0);
                 lcd.print("rpm"); 
                 lcd.setCursor(0, 0);                                          
                 lcd.print(rotacaopulsos); 


                 
                 saidaC = currentDraw*1000;
                
                 lcd.setCursor(0, 1);                                             
                 lcd.print(saidaC);
                 lcd.setCursor(7, 1);                                             
                 lcd.print("mA"); 
                  

                 
                 Serial.println(rotacaopulsos);  
                 Serial.println(currentDraw);                                         
             }
  if(APPSELECT == LOW){




                String rotacaoin ="";
                while (Serial.available()) {
                 char rotacaoinc = Serial.read(); // Lê byte do buffer serial;
                 rotacaoin +=rotacaoinc;
                
                } 
               
                 int rotacaoint= rotacaoin.toInt();
               
    
                 emon1.calcVI(20,100); //FUNÇÃO DE CÁLCULO (20 SEMICICLOS / TEMPO LIMITE PARA FAZER A MEDIÇÃO)
                 double currentDraw = emon1.Irms; //VARIÁVEL RECEBE O VALOR DE CORRENTE RMS OBTIDO
                
                 graficAux = saida*20;
              
                manualRPM  = rotacaoint*1.2681; 
                 
                 if (MODE == 1){    rotacaoEsperada = manualRPM;
                               }
                 else if(MODE ==0){
                  rotacaoEsperada = automaticRPM;
                 }

                 if (ONOFF == 0){
                  RPM=0;
                  saida =0;
                 }
                 
                 //Controle de Rotação
                 saida = (rotacaoEsperada / 3 /4 /2);
                 if (rotacaoEsperada > 2500){
                 saida =  saida-6;
                 }
                 else if(rotacaoEsperada > 1800){
                  saida = saida -5;
                 }
                 else if(rotacaoEsperada > 1200){
                  saida = saida -5;
                 }
                 else if(rotacaoEsperada > 610){
                  saida = saida -6;
                 }
                 else if(rotacaoEsperada > 150){
                  saida = saida -7;
                 }
                 else {
                  saida = 0;
                  manualRPM = 150;
                 }
   
                 
              //  Serial.print(saida); // variando entre 0 a 127
                 analogWrite(saidaPin, saida);
                
                /* lcd.setCursor(0, 1);                                             
                 lcd.print( manualRPM/1.268181);    */             
                 if(rotacaopulsos<1000){
                    lcd.setCursor(3, 0);
                    lcd.print(" "); 
                    }  
                 lcd.setCursor(7, 0);
                 lcd.print("rpm"); 
                 lcd.setCursor(0, 0);                                          
                 lcd.print(rotacaopulsos); 


                 
                 saidaC = currentDraw*1000;
                
                 lcd.setCursor(0, 1);                                             
                 lcd.print(saidaC);
                 lcd.setCursor(7, 1);                                             
                 lcd.print("mA"); 

                  
                 Serial.print(rotacaopulsos); 
                 Serial.println(currentDraw); 
                                                           
             }

}       
  
