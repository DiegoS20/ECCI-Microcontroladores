/*
 * File:   main.c
 * Author: user
 *
 * Created on 24 de abril de 2020, 09:54 PM
 */


#include <xc.h>

#pragma config OSC = XT
#pragma config PWRT = ON
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
#define _XTAL_FREQ 4000000

#define ENABLE LC3
#define RS     LC4

void initial_config(void);
void init_lcd(void);
void init_serial_com(void);
void R_I(char dato);
void R_D(char dato);

void main(void) {
    initial_config();
    init_lcd();
    init_serial_com();
    
    while(1) {
        
    }
    return;
}

void initial_config(void) {
    TRISA = 0X03;
    TRISB = 0B11110000; // Pines RB7:RB4 como entradas y RB3:RB0 como salidas
    TRISC = 0B10000000;
    TRISD = 0X00; 
    
    // Configuración para registros del ADC
    ADCON0 = 0B01000001;
    ADCON1 = 0B10000100;
    
    // Limpiando basura
    LATA = 0;
    LATB = 0XFF; // Condiciones iniciales para asegurar el cambio de estado
    LATC = 0;
    LATD = 0;
    PORTA = 0;
    PORTB = 0XFF; // Condiciones iniciales para asegurar el cambio de estado
    PORTC = 0;
    PORTD = 0;
}

void init_lcd(void) {
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

void init_serial_com(void) {
    INTCON = 0B11001000;
    INTCON2 = 0B00000001;
    RCONbits.IPEN = 0;
    SPBRG = 25;
    PIE1 = 0B00100000;
    PIR1 = 0B00010000;
    IPR1 = 0B00100000;
    TXSTA = 0B00100110;
    RCSTA = 0B10010000;
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
