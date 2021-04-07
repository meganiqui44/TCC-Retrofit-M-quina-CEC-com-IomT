/*  nossa versao final pura com interpolação de vazao e pressao    COM LENGTH
 *  enviando pressao, vazao e rotacao para aplicativo
 *  enviando rotacao para proteus
 */
#include <ESP8266WiFi.h> // biblioteca esp8266

int rotacao = 0;
float corrente = 0.598;
String i; // string entrada do webserver(armazenar rot)
String h;// armazenar pam
String r="R" ; //string para ser comparado com a enviada pelo esp ROT
String p="P" ; //string para ser comparado com a enviada pelo esp PAM
int rot, pam;
float pressao,vazao;
int  j, yC, xR;
float Q11, Q12, Q21, Q22, interpol;
float vCor[] = {0.045,0.048,0.051,0.059,0.081,0.103,0.105,0.135,0.137,0.145,0.204,0.206,0.217,0.278,0.301,0.375,0.501,0.598,0.840};
float vRot[] = {855,1000,1504,1759,2006,2509,2769};

float mVazao[7][19]={{0,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239,1.239},
                        {0,0,0,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968,1.968},
                        {0,0,0,0,0, 1.994, 2.109941176,3.849058824,3.965,3.965,3.965,3.965,3.965,3.965,3.965,3.965,3.965,3.965,3.965},
                        {0,0,0,0,0,0,0,1.79475,1.9144,2.393,4.782983607,4.864,4.864,4.864,4.864,4.864,4.864,4.864,4.864},
                        {0,0,0,0,0,0,0,0,0.084086957,0.420434783,2.901,2.959268041,3.279742268,5.056917526,5.727,5.727,5.727,5.727,5.727},
                        {0,0,0,0,0,0,0,0,0,0,0,0,0,1.433113924,1.973468354,3.712,5.830269058,7.461,7.461},
                        {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.42555157,1.794717489,4.126,5.330058997,8.334}};

float mPressao[7][19]={{16,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15},
                        {23,23,23,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20,20},
                        {52,52,52,52,52,50,49.64705882,44.35294118,44,44,44,44,44,44,44,44,44,44,44},
                        {74,74,74,74,74,74,74,71,70.8,70,60.32786885,60,60,60,60,60,60,60,60},
                        {96,96,96,96,96,96,96,96,95.88405797,95.42028986,92,91.73195876,90.25773196,82.08247423,79,79,79,79,79},
                        {150,150,150,150,150,150,150,150,150,150,150,150,150,148.0696203,147.3417722,145,131.4394619,121,121},
                        {183,183,183,183,183,183,183,183,183,183,183,183,183,183,182.4843049,180.8251121,178,169.1297935,147}};

WiFiServer server(80);  //porta da rede ex:192.168.1.164

void setup() //configuraçoes iniciais 
{
  Serial.begin(9600);                               // baudrate da porta da rede
  WiFi.disconnect();                                // se desconectado cai aqui
  delay(3000);                                      // tempo ate a proxima tentativa de conexao 
  WiFi.begin("DOUGLAS", "ELETRONIC");               // usuario e senha da rede 
  Serial.println("Conectando à rede");              //display de conexao 
  while ((!(WiFi.status() == WL_CONNECTED))) {      // padronizacao de conexao caso ele encontre a rede ele faz a conexao 
    delay(300);
    Serial.print("...");             // display conectando
  }
  Serial.println("Conectado!");               // display conectado!
  Serial.println((WiFi.localIP().toString()));  // display ip
  server.begin(); // inicia web server

}

float interpolarbi (int rotacao, float corrente, String tipo){ 

    for (int i = 0; i < sizeof(vCor); i++) { //for da corrente
       if (corrente <= vCor[i]){
           yC=i;
           i=sizeof(vCor);
       }
       }
    for (int i = 0; i < sizeof(vRot); i++) { //for da rotacao
       if (rotacao <= vRot[i]){
           xR=i;
           i=sizeof(vRot);
     }
     }

  if(tipo=="PRE"){              // SE tipo = PRE, usa valores de Pressao
    Q11 = mPressao[xR-1][yC-1];
    Q12 = mPressao[xR-1][yC];
    Q21 = mPressao[xR][yC-1];
    Q22 = mPressao[xR][yC];
    }
  if(tipo=="VAZ"){               // SE tipo = VAZ, usa valores de VAZAO
    Q11 = mVazao[xR-1][yC-1];
    Q12 = mVazao[xR-1][yC];
    Q21 = mVazao[xR][yC-1];
    Q22 = mVazao[xR][yC];
    }
  
        // Equação de interpolação
    interpol = ((
                Q11*(vRot[xR]-rotacao)*(vCor[yC]-corrente)+
                Q21*(rotacao-vRot[xR-1])*(vCor[yC]-corrente)+
                Q12*(vRot[xR]-rotacao)*(corrente-vCor[yC-1])+
                Q22*(rotacao-vRot[xR-1])*(corrente-vCor[yC-1]) 
                ) / ((vRot[xR]-vRot[xR-1])*(vCor[yC]-vCor[yC-1])
                ));
    return(interpol);
}

void loop() //funcao principal, loop infinito
{

  WiFiClient client = server.available();   // recebe status de conexao 
  if (!client) {                            // se  client não conectado  
    return;                                 // caso problema na rede reiniciar esp
  }
  while (!client.available()) {             // cliente disponivel no caso aplicativo conectado a rede
    delay(1); 
  }

                String correntein ="";
                while (Serial.available()) {
                char correnteinc = Serial.read(); // Lê byte do buffer serial;
                correntein +=correnteinc;
                
                } 
               
                 int correnteint = correntein.toDouble();
                 Serial.println(correnteint);


 
  i = (client.readStringUntil('\r'));       // recebe string inteira ex: HTTP/1.1GET /PAM/400&
  i.remove(0,5);                            // depende da linha de baixo  
  i.remove(i.length() - 9, 9);              // remove os caracteres 8 primeiros caracteres ex: PAM/400&
 
  client.println("HTTP/1.1 200 OK");        // padronizaçao http
  client.println("Content-Type: text/html");// padronizaçao http
  client.println("Connection: close");      // padronizaçao http
  client.println("Refresh: 1");             // regarrega a pagina a cada 1ms
  client.println();                         // padronizaçao http
  
  client.print("rotacao");  
  
  if(String(rotacao).length()<4){
       client.print(0);
                      }
  if(String(rotacao).length()<3){
       client.print(0);
                      }
  if(String(rotacao).length()<2){
       client.print(0);
                      } 
  client.print(rotacao); //envio da rotacao para webserver
  
  
  client.print("vazao");  
  if(String(vazao).length()<4){
       client.print(0);
                      }
  if(String(vazao).length()<3){
       client.print(0);
                      }
  if(String(vazao).length()<2){
       client.print(0);
                      }
  client.print(vazao);    // envio vazao para webserver

  
  client.print("pressao");  
  if(String(pressao).length()<4){
       client.print(0);
                      }
  if(String(pressao).length()<3){
       client.print(0);
                      }
  if(String(pressao).length()<2){
       client.print(0);
                      } 
  client.print(pressao); // envio pressao para webserver


if( i[0]== p[0])
{
  client.println("HTTP/1.1 200 OK");        // padronizaçao http
  client.println("Content-Type: text/html");// padronizaçao http
  client.println("Connection: close");      // padronizaçao http
  client.println("Refresh: 1");             // regarrega a pagina a cada 1ms
  client.println();                         // padronizaçao http

  client.print("rotacao");  
  if(String(rotacao).length()<4){
       client.print(0);
                      }
  if(String(rotacao).length()<3){
       client.print(0);
                      }
  if(String(rotacao).length()<2){
       client.print(0);
                      } 
  client.print(rotacao); //envio da rotacao para webserver
  
  
  client.print("vazao");  
  if(String(vazao).length()<4){
       client.print(0);
                      }
  if(String(vazao).length()<3){
       client.print(0);
                      }
  if(String(vazao).length()<2){
       client.print(0);
                      }
  client.print(vazao);    // envio vazao para webserver

  
  client.print("pressao");  
  if(String(pressao).length()<4){
       client.print(0);
                      }
  if(String(pressao).length()<3){
       client.print(0);
                      }
  if(String(pressao).length()<2){
       client.print(0);
                      } 
  client.print(pressao); // envio pressao para webserver
  
  i.remove(0,4);//  depende da linha de baixo  
  i.remove(i.length() - 10, 9); // remove os caracteres 8 primeiros caracteres ex: PAM/400&
  pam =i.toInt(); //s recebe o valor convertido para inteiro
  Serial.print(pam); // display serial
  }
  
if( i[0]== r[0])
{

  client.println("HTTP/1.1 200 OK");        // padronizaçao http
  client.println("Content-Type: text/html");// padronizaçao http
  client.println("Connection: close");      // padronizaçao http
  client.println("Refresh: 1");             // regarrega a pagina a cada 1ms
  client.println();                         // padronizaçao http
    
  client.print("rotacao");  
  if(String(rotacao).length()<4){
       client.print(0);
                      }
  if(String(rotacao).length()<3){
       client.print(0);
                      }
  if(String(rotacao).length()<2){
       client.print(0);
                      } 
  client.print(rotacao); //envio da rotacao para webserver
  
  
  client.print("vazao");  
  if(String(vazao).length()<4){
       client.print(0);
                      }
  if(String(vazao).length()<3){
       client.print(0);
                      }
  if(String(vazao).length()<2){
       client.print(0);
                      }
  client.print(vazao);    // envio vazao para webserver

  
  client.print("pressao");  
  if(String(pressao).length()<4){
       client.print(0);
                      }
  if(String(pressao).length()<3){
       client.print(0);
                      }
  if(String(pressao).length()<2){
       client.print(0);
                      } 
  client.print(pressao); // envio pressao para webserver

  i.remove(0,4);                // depende da linha de baixo  
  i.remove(i.length() - 10, 9); // remove os caracteres 8 primeiros caracteres ex: PAM/400&
  rot =i.toInt();               // recebe o valor convertido para inteiro
  rotacao=rot;
  
 }
      Serial.println(rotacao);           
     
     vazao = interpolarbi(rotacao, corrente,"VAZ");
     
     pressao = interpolarbi(rotacao, corrente,"PRE")+ pam;
   
 }
