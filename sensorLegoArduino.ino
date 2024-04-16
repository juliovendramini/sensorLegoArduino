/*
    Código demonstrativo de como utilizar o arduino nano
    como um sensor personalizado para o lego ev3
    para retornar valores lidos de outros sensores de arduino
    LIMITAÇÔES:
        - Ainda não é possível enviar valores do brick para o arduino
    Criado por Julio Cesar Goldner Vendramini
*/
#include "EV3UARTLib.h" //biblioteca que transforma o arduino em um sensor do Serial do lego EV3, 
                        //código obtido em https://github.com/DexterInd/EV3_Dexter_Industries_Sensors
                        //alterações foram feitas na biblioteca original do link
//mais informações de como o protocolo funciona pode ser encontrado em: 
// https://github.com/pybricks/technical-info/blob/master/uart-protocol.md



//definicoes dos pinos do arduino
#define LED 13
//exemplos com botoes
#define BOTAO1 5
#define BOTAO2 6
#define BOTAO3 7
#define BOTAO4 8

//exemplo de entradas analogicas
#define SENSOR0 3
#define SENSOR1 2
#define SENSOR2 1
#define SENSOR3 0


/*observação sobre os tipos das variaveis:
o tipo uint8_t possui 8 bits 0 até 255
o tipo int8_t possui 8 bits -128 até 127
o tipo uint16_t possui 16 bits indo de 0 até 65.535
o tipo int16_t possui 16 bits indo de -32.768 até 32.767
o que observei é que o brick trabalha sempre com os valores com sinal
*/

//vetor com os valores que serão enviados para o lego. Cada sensor deve ocupar uma posição
//podemos enviar até 32 bytes para o lego, o que daria em valores de 8bits(1 byte), 32 valores diferentes
//para 1 byte, o brick reconhece os valores como -128 até 127
//é possível termos até 8 modos diferentes de configuração do sensor, em cada modo podemos escolher o que enviar e qual tamanho dos bytes.
//quanto mais dado por vez for enviado para o brick, mais processamento consumirá do brick

//define a quantidade de valores diferentes que serão enviado por modo
//a quantidade de sensores deve ser potencia de 2: (2,4,8,16 ou 32)
#define QUANTIDADE_SENSORES_MODO0 4
//vetor que será enviado contendo 1 byte por sensor
uint8_t valoresModo0[QUANTIDADE_SENSORES_MODO0];

//caso sejá necessário enviar valores maiores dos retornos dos sensores, temos que utilizar 2 bytes, que daria algo aproximado entre -32mil e +32mil
#define QUANTIDADE_SENSORES_MODO1 2
//vetor que será enviado contendo 2 bytes por sensor
int16_t valoresModo1[QUANTIDADE_SENSORES_MODO1];

//mais modos como os exemplos acima podem ser criados

//a ideia deste código é coletar todos os valores dos sensores que serão utilizados e jogá-los nesses vetores
//a biblioteca se encarrega de enviar o valores para o brick

//exemplo para atualizar os sensores do modo 0 (1 byte no exemplo)
void atualizaSensoresModo0(){
    uint8_t exemplo[QUANTIDADE_SENSORES_MODO0];
    exemplo[0] = analogRead(SENSOR0)/4; //dividindo por 4 dará 8bits (1 byte)
    exemplo[1] = analogRead(SENSOR1)/4;
    exemplo[2] = analogRead(SENSOR2)/4;
    exemplo[3] = analogRead(SENSOR3)/4;
    //nesse exemplo acima, e como falado sobre os tipos, se enviar o valor, por exemplo 200, no exemplo[1], teriamos um valor negativo sendo apresentado no brick
    //para isso nao acontecer, teriamos que trabalhar, em 8 bits, com valores indo até 127, no caso poderiamos dividir por 2 novamente
    for(uint8_t i=0; i < QUANTIDADE_SENSORES_MODO0; i++){
        exemplo[i] = exemplo[i]/2;    
    }
    /*poderia fazer qualquer coisa aqui com os valores
                ....

                ...


                ...

    */
    for(uint8_t i=0; i < QUANTIDADE_SENSORES_MODO0; i++){
        valoresModo0[i] = exemplo[i];
    }
}

//exemplo para atualizar os sensores do modo 1 (2 bytes no exemplo)
void atualizaSensoresModo1(){
    int16_t exemplo[QUANTIDADE_SENSORES_MODO1];
    exemplo[0] = analogRead(SENSOR0);
    exemplo[1] = analogRead(SENSOR1);
    exemplo[2] = analogRead(SENSOR2);
    exemplo[3] = analogRead(SENSOR3);
    //aqui, o brick aceita valore até 32.575... então nao teremos problemas, só que enviamos mais bytes pra ele, 
    //deixando a comunicação mais lenta e consumindo mais processamento
    
    /*poderia fazer qualquer coisa aqui com os valores
                ....

                ...


                ...

    */
    for(uint8_t i=0; i < QUANTIDADE_SENSORES_MODO1; i++){
        valoresModo1[i] = exemplo[i];
    }
}


//existem duas formas de criar o sensor:
//usando a porta serial padrão (da pra atingir velocidade maiores)
//configuracao do sensor (numero de identificacao pro brick, velocidade serial)
//no modo serial padrão, nunca utilize Serial.print no seu código, pois são os mesmo pinos para comunicar com o brick
//isso fará perder o sincronimos e seu sensor não funcionará
//88 no parametro é a identificação do sensor pro EV3, o de cor por exemplo é o 29. Estamos usando um numero qualquer aqui
//EV3UART sensor(88, 38400); //é possivel atingir até 115200 de velocidade na serial
//ou usando uma porta SoftwareSerial, usando velocidade até 38400
//dessa forma é possivel usar Serial.print para debug
#define PINO_RX 10
#define PINO_TX 11
EV3UART sensor(88, 38400, PINO_RX, PINO_TX);
bool modoDebug = false;
void setup() {
    //inicializacao da porta Serial (só faça isso se for usar o modo com software serial no sensor)
    //Serial.begin(38400); //faça isso apenas se for usar para debug
    //inicialização dos pinos do arduino
    pinMode(LED, OUTPUT);
    pinMode(BOTAO1,INPUT_PULLUP);
    pinMode(BOTAO2,INPUT_PULLUP);
    pinMode(BOTAO3,INPUT_PULLUP);
    pinMode(BOTAO3,INPUT_PULLUP);
    digitalWrite(LED,HIGH); //acendo o led para analisar reacao do modulo e sabe se ligou, por exemplo

    //criacao dos modos dos sensores
    //ordem dos parametros(nome do modo, se o valor será visualizado(coloque nomes bem pequenos), tamanho do dado (1 byte, 2 bytes),quantidade de valores enviados, 
                        // quantidade de digitos, casas decimais, valor minimo do sensor, valor maximo do sensor)
	//ATENCAO! Nao utilize numeros para o nome do sensor, pode dar problema, e não sei o motivo. Mas em algumas situações ele nao sincronização
	//com o brick. Prefira utilizar somente letras maiusculas e traço (-)
    sensor.create_mode("MODO-UM", true, DATA8, QUANTIDADE_SENSORES_MODO0, 3, 0, 0, 255);
    sensor.create_mode("MODO-DOIS", true, DATA16, QUANTIDADE_SENSORES_MODO1, 5, 0,0,65535);
    
    delay(200);
    digitalWrite(LED,LOW);
    sensor.reset(); //entra em modo de sincronização com o brick
}


#define TEMPO_ESPERA_ENVIO_BRICK 20 //nesse caso ele aguarda 20ms pra enviar entre cada envio, isso equivale a uma taxa de atualização de 50Hz
unsigned long momentoUltimaLeitura = 0;
#define TEMPO_ESPERA_ATUALIZACAO_DADOS_SENSOR1 200 //faça dessa forma no lugar de utilizar delays para esperar
unsigned long momentoUltimaLeituraSensor1 = 0; //exemplo de variavel para esperar um tempo parar leitura do sensor1
uint8_t data = 0;

//no loop o codigo deve:
//verificar se a sincronização com o brick continua
//ler os valores de todos os sensores que devem ser atualizados
//atualizar o vetor que será enviado pelo arduino para o brick
//não coloque delays no seu código isso fará o loop parar e quebrará o sincronismo com o brick
void loop() {
    sensor.heart_beat(); //aqui verifica a sincronização com o brick e se teve alguma mudança de modo que o brick solicitou pro sensor
    if (millis() - momentoUltimaLeitura > TEMPO_ESPERA_ENVIO_BRICK) { //tempo em milissegundos (dessa forma conseguimos enviar os dados 
                                                //em um intervalo de tempo sem usar delay, se precisar esperar tempo
                                                //em outros locais, faça da mesma forma)
        momentoUltimaLeitura = millis(); //atualizamos o tempo que entrou no if antes para garantir que o tempo de leitura dos sensores
                                         //não atrapalhe a contagem de tempo do TEMPO_ESPERA_ENVIO_BRICK
        if(sensor.get_current_mode() == 0 ){ //caso esteja no modo 0 atualizo os sensores que devem ser enviados nesse modos 
            atualizaSensoresModo0(); //pode-se utilizar uma função onde todos os dadosdos sensores são atualizados
            sensor.send_data8(valoresModo0,QUANTIDADE_SENSORES_MODO0); //e envia usando o método do modo 0 (neste caso variaveis de 1 byte)
        }else if(sensor.get_current_mode() == 1 ){ //caso esteja no modo 1 atualizo os sensores que devem ser enviados nesse modos 
            atualizaSensoresModo1(); //pode-se utilizar uma função onde todos os dadosdos sensores são atualizados
            sensor.send_data16(valoresModo1,QUANTIDADE_SENSORES_MODO1); //e envia usando o método do modo 1 (neste caso variaveis de 2 bytes)
        }
	    
    }
}
