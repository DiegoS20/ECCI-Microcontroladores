/*
 * File:   main.c
 * Author: user
 *
 * Created on 3 de mayo de 2020, 03:05 PM
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#pragma config OSC = XT
#pragma config PWRT = ON
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
#define _XTAL_FREQ 4000000

#define ENABLE LC3
#define RS     LC4

void initial_config(void);
void init_LCD(void);
void serial_transmi_init_config(void);
void PWM_config(void);
void R_I(char dato); // Rutina de instrucción
void R_D(char dato); // Rutina de datos
void print_message_onLCD(char message[20], char place);
void transmit(char response[50]);
void PWM1(unsigned int ciclo_util);
void PWM2(unsigned int ciclo_util);

char cu1 = 75, cu2 = 25;
char cu[3];
char cu_changed = 0;
char cu_flag;

void main(void) {
    initial_config();
    serial_transmi_init_config();
    PWM_config();
    init_LCD();
    
    transmit("Senial 1: o \n\r");
    transmit("Senial 2: t \n\r");
    transmit("Ambas seniales: b \n\r");
    transmit("\n\r");
    
    PWM1(cu1);
    PWM2(cu2);
    print_message_onLCD("Freq:29.4kHz", 128);
    
    while(1) {
        if (cu_changed) {
            if (100 < atoi(cu)) {
                transmit("El valor de la senial debe estar entre 0 y 100\n\r");
            } else {
                switch(cu_flag) {
                    case 'o':
                        cu1 = atoi(cu);
                        break;
                    case 't':
                        cu2 = atoi(cu);
                        break;
                    case 'b':
                        cu1 = atoi(cu);
                        cu2 = atoi(cu);
                        break;
                    default:
                        transmit("La opcion no es valida\n\r");
                        break;
                }
                PWM1(cu1);
                PWM2(cu2);
            }
            for (char i = 0; i < 3; i++) {
                cu[i] = '\0';
            }
            cu_changed = 0;
        }
        char f1_m[50];
        sprintf(f1_m, "CU1:%i%% ", cu1);
        print_message_onLCD(f1_m, 192);
        char f2_m[50];
        sprintf(f2_m, "CU2:%i%%  ", cu2);
        print_message_onLCD(f2_m, 0);
    }
    return;
}

void initial_config(void) {
    /*
     * Bit 7 como salida para RX (transmisión serial)
     * Bit 6 como entrada para TX (transmisión serial)
     * Bit 1 y 2 para PWM2 y PWM1 respectivamente
     */
    TRISC = 0B10000000;
    TRISD = 0X00;
    
    /* Limpiando basura */
    LATC = 0;
    LATD = 0;
    PORTC = 0;
    PORTD = 0;
}

void serial_transmi_init_config(void) {
    INTCON = 0B11000000;
    RCONbits.IPEN = 0;
    SPBRG = 25;
    PIE1 = 0B00100000;
    PIR1 = 0B00010000;
    IPR1 = 0B00100000;
    TXSTA = 0B00100110;
    RCSTA = 0B10010000;
}

void PWM_config(void) {
    /**
     * Módulo CCP1 y CCP2 como PWM
     */
    CCP1CON = 0B00011100;
    CCP2CON = 0B00011100;
    
    /**
     * Estableciendo PWM a 29.4KHz
     */
    PR2 = 0X21;
    
    /**
     * Estableciendo un ciclo útil inicial de un 50%
     */
    CCPR1L = 0B01010011;
    CCPR2L = 0B01010011;
    
    /**
     * TIMER 2 CONTROL REGISTER
     * 
     * Timer 2 encendido con prescaler de 1 (1:1)
     */
    T2CON = 0B00000100;
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

void interrupt capture(void) {
    /*
     * Se pregunta si se produjo interrupción por recepción de datos
     */
    if (PIR1bits.RCIF) {
        char data_received = RCREG;
        if (data_received != 'o' && data_received != 't' && data_received != 'b') {
            if (!isdigit(data_received)) {
                transmit("Solo puedes enviar numeros\n\r");
            } else {
                cu[strlen(cu)] = data_received;
            }
        } else {
            cu_changed = 1;
            cu_flag = data_received;
        }
        
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

void PWM1(unsigned int ciclo_util) {
    /**
     Tiempo en alto
     Cuánto del periodo PWM la señal se mantiene en uno lógico
     El valor calculado se asignará a los registros CCPxL y CCPxCON
     */
    unsigned int t_alto = (136 * ciclo_util) / 100;
    
    /*
     Cargando los 8 bits de mayor peso del duty (ciclo útil)
     al registro CCPR1L (El fabricante así lo estipula)
     */
    CCPR1L = t_alto>>2;
    
    /*
     Los desplazo 4 posiciones para poner los dos bits de menor peso en
     las posiciones 5:4
     */
    t_alto = (t_alto<<4)&0x0030;
    
    /*
     Salvando la configuración del CCPxCON y borrando los dos bits
     correspondientes a la asignació del ciclo útil (5:4)
     */
    CCP1CON &= 0XCF; 
    
    /*
     Cargo los dos bits de menor peso del duty
     */
    CCP1CON |= t_alto;
}

void PWM2(unsigned int ciclo_util) {
    unsigned int t_alto = (136 * ciclo_util) / 100;
    CCPR2L = t_alto>>2;
    t_alto = (t_alto<<4)&0x0030;
    CCP2CON &= 0XCF;
    CCP2CON |= t_alto;
}
