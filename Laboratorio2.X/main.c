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

void initial_config(void);
void init_LCD(void);
void R_I(char dato); // Rutina de instrucción
void R_D(char dato); // Rutina de datos
void print_message_onLCD(char message[20]);
void turn_off_everything(void);
void light_the_bulb(void);
void start_engine(void);
void light_the_bulb_and_start_engine(void);
float get_voltage(unsigned int ac_in);
float get_temp(unsigned int ac_in);

void main(void) {
    initial_config();
    init_LCD();
    R_I(128);
    print_message_onLCD("CONVERSOR ADC");
    
    R_I(192);
    print_message_onLCD("Diego - 84010");
    __delay_ms(3000);
    
    R_I(0X01); // Limpiando pantalla de LCD
    
    char canal = 0;
    float volt;
    float temp;
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
            R_I(128);
            sprintf(message, "Voltaje: %0.2fV   ", volt);
        } else {
            temp = get_temp(in);
            R_I(192);
            sprintf(message, "Temp: %.0fC       ", temp);
        }
        
        if (volt > 3.8) {
            R_I(128);
            print_message_onLCD("    PELIGRO   ");
            R_I(192);
            print_message_onLCD("  ALTO VOLTAJE  ");
            light_the_bulb();
        } else if (volt <= 0.29 && temp >= 61) {
            R_I(128);
            print_message_onLCD("     RIESGO     ");
            R_I(192);
            print_message_onLCD(" DE DESTRUCCION ");
            start_engine();
        } else if (temp < 5) {
            R_I(128);
            print_message_onLCD("     LLEGO     ");
            R_I(192);
            print_message_onLCD("     EL FIN     ");
            light_the_bulb_and_start_engine();
        } else {
            print_message_onLCD(message);
            turn_off_everything();
        }
        
        __delay_ms(10);
        canal = canal ? 0 : 1;
    }
    return;
}

void initial_config(void) {
    TRISA = 0X03; // Los pines 0 y 1, canales para conversión, se inyectan las señales de los potenciometros
    // TRISB = 0X00; // Muestra los 8 bits de menor peso de la salida del conversor del canal 0, se conectan los leds para visualizar usando las resistencias de 330Ohm por led
    TRISB = 0X00; // Bombillo y Motor
    TRISC = 0X00;
    TRISD = 0X00; // Bus de datos para los pines de la LCD
    // TRISE = 0X00;
    
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

void print_message_onLCD(char message[20]) {
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
