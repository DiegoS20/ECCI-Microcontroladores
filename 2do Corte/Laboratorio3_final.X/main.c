/*
 * File:   main.c
 * Author: user
 *
 * Created on 3 de abril de 2020, 11:22 AM
 */


#include <xc.h>
#include <string.h>
#include <stdio.h>

#pragma config OSC = XT
#pragma config PWRT = ON
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
#define _XTAL_FREQ 4000000

#define ENABLE LC3
#define RS     LC4

char first_execution = 1;

void initial_config(void);
void init_LCD(void);
void R_I(char dato); // Rutina de instrucción
void R_D(char dato); // Rutina de datos
void print_message_onLCD(char message[20], char place);
void turn_off_everything(void);
void light_the_bulb(void);
void start_engine(void);
void light_the_bulb_and_start_engine(void);
void serial_transmi_init_config(void);
void transmit(char response[50]);
float get_voltage(unsigned int ac_in);
float get_temp(unsigned int ac_in);

char data_received = 0;
char send_alert = 0;
char alert_sent = 0;
char w_alert_sent;
float volt;
float temp;
char message_alert[50] = "";
void main(void) {
    initial_config();
    serial_transmi_init_config();
    init_LCD();
    print_message_onLCD("CONVERSOR ADC", 128);
    print_message_onLCD("Diego - 84010", 192);
    __delay_ms(3000);
    
    R_I(0X01); // Limpiando pantalla de LCD
    
    char canal = 0;
    while(1) {
        ADCON0 = canal ? 0B01001001 : 0B01000001; // Selecciono canal de entrada de los datos
        __delay_us(20);
        GO_DONE = 1;
        
        while(GO_DONE) {}
        
        unsigned int in = (ADRESH<<8) | ADRESL;
        __delay_ms(50);
        char message[20] = "";
        if (!canal) {
            volt = get_voltage(in);
            sprintf(message, "Voltaje: %0.2fV   ", volt);
        } else {
            temp = get_temp(in);
            sprintf(message, "Temp: %.0fC       ", temp);
        }
        
        if (volt > 3.8) {
            print_message_onLCD("    PELIGRO   ", 128);
            print_message_onLCD("  ALTO VOLTAJE  ", 192);
            light_the_bulb();
            
            send_alert = alert_sent && w_alert_sent == '1' ? 0 : 1;
            w_alert_sent = '1';
            sprintf(message_alert, "PELIGRO: ALTO VOLTAJE\r\n");
        } else if (volt <= 0.29 && temp >= 61) {
            print_message_onLCD("     RIESGO     ", 128);
            print_message_onLCD(" DE DESTRUCCION ", 192);
            start_engine();
            
            send_alert = alert_sent && w_alert_sent == '2'? 0 : 1;
            w_alert_sent = '2';
            sprintf(message_alert, "PELIGRO: RIESGO DE DESTRUCCION\r\n");
        } else if (temp < 5) {
            print_message_onLCD("     LLEGO     ", 128);
            print_message_onLCD("     EL FIN     ", 192);
            light_the_bulb_and_start_engine();
            
            send_alert = (alert_sent && w_alert_sent == '3') ? 0 : 1;
            w_alert_sent = '3';
            sprintf(message_alert, "PELIGRO: LLEGO EL FIN\r\n");
        } else {
            char line = !canal ? 128 : 192;
            print_message_onLCD(message, line);
            turn_off_everything();
            
            send_alert = 0;
            alert_sent = 0;
            w_alert_sent = '0';
        }
        
        if (!first_execution) {
            char _volt[25];
            char _temp[25];
            sprintf(_volt, "Voltaje: %.2fV\r\n", volt);
            sprintf(_temp, "Temperatura: %.0fC\r\n", temp);
            
            if (send_alert) {
                transmit(message_alert);
                transmit(_temp);
                transmit(_volt);
                
                transmit("\n\n");
                alert_sent = 1;
            }
            
            if (data_received) {
                if (data_received == 't') {
                    transmit(_temp);
                } else if (data_received == 'v') {
                    transmit(_volt);
                } else if (data_received == 'x') {
                    transmit(_temp);
                    transmit(_volt);
                } else {
                    transmit("Opcion invalida\r\n");
                }
                data_received = 0;
            }
        } else {
            transmit("Presiona:\r\n");
            transmit("t - Conocer temperatura\r\n");
            transmit("v - Conocer voltaje\r\n");
            transmit("\n");
            first_execution = 0;
        }
        
        __delay_ms(10);
        canal = canal ? 0 : 1;
    }
    return;
}

void initial_config(void) {
    TRISA = 0X03;
    TRISB = 0X00; // Bombillo y Motor
    TRISC = 0B10000000;
    TRISD = 0X00; 
    
    // Configuración para registros del ADC
    ADCON0 = 0B01000001;
    ADCON1 = 0B10000100;
    
    // Limpiando basura
    LATA = 0;
    LATB = 0;
    LATC = 0;
    LATD = 0;
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    PORTD = 0;
}

void serial_transmi_init_config(void) {
    /* 
     * Configuración USART de interrupciones
     * Ver el datasheet para saber qué configura c/pin 
     * 
     * Habilitando interrupciones globales y de periféricos
    */
    INTCON = 0B11000000;
    
    RCONbits.IPEN = 0; // Desabilitando prioridad de las interrupciones
    SPBRG = 25; // Permite trabajar transmisión de datos a 9600 bits/seg. (bouds)
    
    /*
     * Habilitar interrupción por recepción de datos (PIE1bits.RCIE
     * Deshabilitar interrupción transmisión de datos (PIE1bits.TXIE)
     */
    PIE1 = 0B00100000;
    
    PIR1 = 0B00010000; // Borrar bandera interrupción por recepción de datos (PIR1bits.RCIF = 0)
    IPR1 = 0B00100000; // Repeción de datos con alta prioridad (IPR1bits.RCIP = 1)
    
    /*
     * 8 bits de transmisión
     * Habilitar transmisión de datos
     * Selecciono modo asíncrono y alta velocidad
     * Registro de cambio de transmisión: vacío
     */
    TXSTA = 0B00100110;
    
    /*
     * Habilitar puerto serial
     * 8 bits de recepción
     * Recepción contínua
     */
    RCSTA = 0B10010000;
}

void init_LCD(void) {
    __delay_ms(20);
    // Inicializando LCD
    R_I(0X30); // Función set (datasheet)
    __delay_ms(5);
    R_I(0X30);
    R_I(0X3C);
    R_I(0X0C);
    R_I(0X01);
    __delay_ms(20);
}

void R_I(char dato) {
    ENABLE = 0;
    RS = 0; // Instrucción
    LATD = dato;
    ENABLE = 1;
    __delay_us(100);
    ENABLE = 0;
}

void R_D(char dato) {
    ENABLE = 0;
    RS = 1; // Dato
    LATD = dato;
    ENABLE = 1;
    __delay_us(100);
    ENABLE = 0;
}

void print_message_onLCD(char message[20], char place) {
    if (place > 0) {
        R_I(place);
    }
    for (char i = 0; i < strlen(message); i++) {
        R_D(message[i]);
    }
}

float get_voltage(unsigned int ac_in) {
    return (ac_in * 5.0 / 1023);
}
float get_temp(unsigned int ac_in) {
    float r = ac_in * 500.0 / 1023;
    if (ac_in <= 10) {
        r += 0.3;
    }
    return (r - 0.0099999);
}

void turn_off_everything(void) {
    LATB = 0;
}

void light_the_bulb(void) {
    LATB = 1;
}

void start_engine(void) {
    LATB = 2;
}

void light_the_bulb_and_start_engine(void){
    LATB = 3;
}

void interrupt capture(void) {
    /*
     * Se pregunta si se produjo interrupción por recepción de datos
     */
    if (PIR1bits.RCIF) {
        data_received = RCREG;
        PIR1bits.RCIF = 0;
    }
}

void transmit(char response[50]) {
    for (char i = 0; i < strlen(response); i++) {
        /*
         * Esperando a que termine de transmitir el dato anterior
         */
        while(!TXSTAbits.TRMT);
        TXREG = response[i];
    }
}
