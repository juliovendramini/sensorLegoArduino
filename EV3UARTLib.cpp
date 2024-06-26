#include "EV3UARTLib.h"
// #include <serial->h>

/**
 * Create a mode object
 **/
EV3UARTMode::EV3UARTMode(){
}

/**
 * Create the sensor emulation with hardware serial
 **/
EV3UART::EV3UART(byte type, unsigned long speed){
    this->serialType = HARDWARE_MODE;
    Serial.end();
    this->serial = &Serial;
    this->type = type;
    this->speed = speed;
    status = 0;
    modes = 0;
    views = 0;
    current_mode = 0;
}

/**
 * Create the sensor emulation with the specified RX and TX pins
 **/
EV3UART::EV3UART(byte type, unsigned long speed, uint8_t rx_pin, uint8_t tx_pin){
    this->serialType = SOFTWARE_MODE;
    this->softSerial = new SoftwareSerial(rx_pin, tx_pin);
    this->serial = this->softSerial;
    this->type = type;
    this->speed = speed;
    status = 0;
    modes = 0;
    views = 0;
    current_mode = 0;
}

/**
 * Define the next mode
 **/
void EV3UART::create_mode(String name, boolean view,
                                                                     byte data_type, byte sample_size,
                                                                     byte figures, byte decimals, float min, float max){
    EV3UARTMode *mode = new EV3UARTMode();
    mode->name = name;
    mode->view = view;
    mode->data_type = data_type;
    mode->sample_size = sample_size;
    mode->figures = figures;
    mode->decimals = decimals;
    mode->min = min;
    mode->max = max;
    mode_array[modes] = mode;
    modes++;
    if(view){
        views++;
    }
}

/**
 * Get the status of the connection
 **/
byte EV3UART::get_status(){
    return status;
}

/**
 * Reset the connection
 **/
void EV3UART::reset(){
    for (;;){   
        if(this->serialType == HARDWARE_MODE){
            Serial.begin(2400);
        }else{
            softSerial->begin(2400);
        }
        
        byte b[4];
        b[0] = type;
        send_cmd(CMD_TYPE, b, 1);
        b[0] = modes - 1;
        b[1] = views - 1;
        send_cmd(CMD_MODES, b, 2);
        get_long(speed, b);
        send_cmd(CMD_SPEED, b, 4);
        for (int i = modes - 1; i >= 0; i--){
            EV3UARTMode *mode = get_mode(i);
            // mandando nome
            byte l = mode->name.length();
            byte ll = next_power2(l);
            byte bb[ll + 2];
            for(uint8_t i=0; i < ll + 2; i++){ //se nao limpar tudo antes pode dar problema com o lixo (já aconteceu)
                bb[i] = 0;    
            }
            mode->name.getBytes(&bb[1], ll + 1); // Leave room for null terminator
            byte lll = log2(ll);
            // Send name
            send_cmd(CMD_INFO | (lll << CMD_LLL_SHIFT) | i, bb, ll + 1);
            byte raw[9];

            // mandando info raw
            raw[0] = 0x01;
            byte tempFloat[4];
            memcpy(tempFloat, &(mode->min), sizeof(float));
            //tempFloat = (void* )&(mode->min);
            for (int i = 0; i < 4; i++){
                raw[i + 1] = tempFloat[i];
            }
            memcpy(tempFloat, &(mode->max), sizeof(float));
            for (int i = 0; i < 4; i++){
                raw[i + 5] = tempFloat[i];
            }
            send_cmd(CMD_INFO | (0x03 << CMD_LLL_SHIFT) | i, raw, 9 /*tamanho do envio*/);

            // mandando info scale
            raw[0] = 0x03;
            memcpy(tempFloat, &(mode->min), sizeof(float));
            for (int i = 0; i < 4; i++){
                raw[i + 1] = tempFloat[i];
            }
            memcpy(tempFloat, &(mode->max), sizeof(float));
            for (int i = 0; i < 4; i++){
                raw[i + 5] = tempFloat[i];
            }
            send_cmd(CMD_INFO | (0x03 << CMD_LLL_SHIFT) | i, raw, 9 /*tamanho do envio*/);

            // mandando INFO_FORMAT
            byte bbb[5];
            bbb[0] = 0x80;
            bbb[1] = mode->sample_size;
            bbb[2] = mode->data_type;
            bbb[3] = mode->figures;
            bbb[4] = mode->decimals;
            send_cmd(CMD_INFO | (2 << CMD_LLL_SHIFT) | i, bbb, 5);
        }

        send_byte(BYTE_ACK);
        unsigned long m = millis();
        while (!(serial->available()) && millis() - m < ACK_TIMEOUT){
            ;
        }
        if(serial->available()){
            byte b = serial->read();
            if(b == BYTE_ACK){
                    if(this->serialType == HARDWARE_MODE){
                        Serial.end();
                        Serial.begin(speed);
                    }else{
                        softSerial->end();
                        softSerial->begin(speed);
                    }
                delay(80);
                last_nack = millis();
                break;
            }
        }
    }
}

/**
 * Process incoming messages
 **/
void EV3UART::heart_beat(){
    byte checksum;
    byte mode;
    byte tamanho;
    byte modo;
    byte temp;
    if(millis() - last_nack > HEARTBEAT_PERIOD){
        reset();
    }
    if(serial->available()){
        byte b = serial->read();
        if(b == BYTE_NACK){
            last_nack = millis();
        }
        else{
            if(b == CMD_SELECT){
                checksum = 0xff ^ b;
                mode = read_byte();
                checksum ^= mode;
                Serial.println(mode);
                if(checksum == read_byte()){
                    if(mode < modes)
                        current_mode = mode;
                }
            }
            else if((b & 0b11000000) == CMD_TYPE){ // modo envio de dados MESSAGE_CMD
                tamanho = b & 0b00111000;
                tamanho = tamanho >> 3;
                tamanho = this->next_power2(tamanho);
                modo = b & 0b00000111;
                if(modo == CMD_EXT_MODE){ // é para ler os proximos dados
                    temp = read_byte();
                    checksum = read_byte();
                    b = read_byte();
                    if((b & 0b11000000) == MESSAGE_DATA)
                    {
                        tamanho = b & 0b00111000;
                        tamanho = tamanho >> 3;
                        tamanho = this->next_power2(tamanho);
                        if(tamanho == this->sample_size_return)
                        {
                            modo = b & 0b00000111;
                            if(modo == this->current_mode)
                            {
                                // leio os dados
                                for (uint8_t k = 0; k < tamanho; k++)
                                {
                                    this->valoresRetorno[k] = read_byte();
                                }
                                read_byte(); // checksum
                            }
                        }
                    }
                }
            }
        }
    }
}

/**
 * Get the mode object for a specific mode.
 **/
EV3UARTMode *EV3UART::get_mode(byte mode){
    return mode_array[mode];
}

/**
 * Send a single byte t the EV3, as data.
 **/
void EV3UART::send_data8(byte b){
    byte bb[1];
    bb[0] = b;

    send_cmd(CMD_DATA | current_mode, bb, 1);
}

/**
 * Send a single int16_t to the EV3
 **/
void EV3UART::send_data16(int16_t s){
    byte bb[2];
    bb[0] = s & 0xff;
    bb[1] = s >> 8;
    send_cmd(CMD_DATA | (1 << CMD_LLL_SHIFT) | current_mode, bb, 2);
}

/**
 * Send a multiples byte t the EV3, as data.
 **/
void EV3UART::send_data8(byte *b, uint8_t len){

    send_cmd(CMD_DATA | (log2(len) << CMD_LLL_SHIFT) | current_mode, b, len);
}

/**
 * Send an array of int16_ts to the EV3 as data.
 * len must be a power of 2
 **/
void EV3UART::send_data16(int16_t *s, uint8_t len){
    byte bb[len * 2];
    for (int i = 0; i < len; i++){
        bb[2 * i] = s[i] & 0xff;
        bb[2 * i + 1] = s[i] >> 8;
    }
    send_cmd(CMD_DATA | (log2(len * 2) << CMD_LLL_SHIFT) | current_mode, bb, len * 2);
}

/**
 * Send a long to the EV3 as data
 **/
void EV3UART::send_data32(long l){
    byte bb[4];
    for (int i = 0; i < 4; i++){
        bb[i] = (l >> (i * 8)) && 0xff;
    }
    send_cmd(CMD_DATA | (2 << CMD_LLL_SHIFT) | current_mode, bb, 4);
}

/**
 * Send a float to the EV3 as data
 **/
void EV3UART::send_dataf(float f){
    union Data
    {
        unsigned long l;
        float f;
    } data;
    data.f = f;
    send_data32(data.l);
}

/**
 * Send a command to the    EV3
 **/
void EV3UART::send_cmd(byte cmd, byte *data, byte len){
    byte checksum = 0xff ^ cmd;
    serial->write(cmd);
    for (int i = 0; i < len; i++){
        checksum ^= data[i];
        serial->write(data[i]);
    }
    serial->write(checksum);
}

/**
 * Write a single byte to the EV3
 **/
void EV3UART::send_byte(byte b){
    serial->write(b);
}

/**
 * Utility method to copy a long into a byte array
 **/
void EV3UART::get_long(unsigned long l, byte *bb){
    for (int i = 0; i < 4; i++){
        bb[i] = (l >> (i * 8)) & 0xff;
    }
}

/**
 * Utility method to return a small power of 2
 **/
int EV3UART::log2(int val){
    switch (val){
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 8:
        return 3;
    case 16:
        return 4;
    case 32:
        return 5;
    default:
        return 0;
    }
}

/**
 * Utility method to return the next power of 2 (up to 32)
 **/
int EV3UART::next_power2(int val){
    if(val == 1 || val == 2)
        return val;
    else if(val <= 4)
        return 4;
    else if(val <= 8)
        return 8;
    else if(val <= 16)
        return 16;
    else if(val <= 32)
        return 32;
    else
        return 0;
}

/**
 * Utility method to read a byte synchronously
 **/
byte EV3UART::read_byte(){
    while (!serial->available())
        ;
    return serial->read();
}

/**
 * Get the current mode
 **/
byte EV3UART::get_current_mode(){
    return current_mode;
}