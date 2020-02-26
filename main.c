/*
 * File:   main.c
 * Author: user
 *
 * Created on 10 de febrero de 2020, 08:09 PM
 */


#include <xc.h>
#include <math.h>
#include "function_statements.h"
#include <ctype.h>

#pragma config OSC = XT
#pragma config PWRT = ON
#pragma config BOR = OFF
#pragma config WDT = OFF
#pragma config LVP = OFF
#define _XTAL_FREQ 4000000
//Enables
#define ENABLE1 LA3
#define ENABLE2 LA5

char NUMERO[16][3] = {
    0XF8, 0X88, 0XF8, // 0
    0X40, 0XF8, 0XF8, // 1
    0XB8, 0XA8, 0XE8, // 2
    0XA8, 0XA8, 0XF8, // 3
    0XE0, 0X20, 0XF8, // 4
    0XE8, 0XA8, 0XB8, // 5
    0XF8, 0XA8, 0XB8, // 6
    0X80, 0X80, 0XF8, // 7
    0XF8, 0XA8, 0XF8, // 8
    0XE0, 0XA0, 0XF8, // 9
    0XF8, 0XA0, 0XF8, // A
    0XF8, 0X28, 0X38, // B
    0XF8, 0X88, 0X88, // C
    0X38, 0X28, 0XF8, // D
    0XF8, 0XA8, 0XA8, // E
    0XF8, 0XA0, 0XA0  // F
};

char SIMBOLO[11][3] = {
    0X20, 0X70, 0X20, // +
    0X20, 0X20, 0X20, // -
    0X50, 0X20, 0X50, // x
    0X10, 0X20, 0X40, // /
    0X70, 0X20, 0X70, // M
    0X20, 0X40, 0X20, // AND
    0X40, 0X20, 0X40, // v
    0X50, 0X60, 0X50, // NAND
    0X60, 0X50, 0X60, // XOR
    0X80, 0XF8, 0X80, // T
    0X50, 0X50, 0X50, // =
};

void main(void) {
    char oper_1;
    char oper_2;
    char oper;
    char resultado;
    
    init_config();
    while(1) {
        oper_1 = PORTB>>4;
        oper_2 = PORTB&0X0F;
        oper = PORTC&0X0F;
        // Mostrando las operaciones
        // Mostrando el resultado
        for (char i = 0; i < 200; i++) {
            switch (oper) {
                case 1:
                    resultado = oper_1 < oper_2 ? operar(oper, oper_2, oper_1) : operar(oper, oper_1, oper_2);
                    char x = 0;
                    if (oper_1 < oper_2) {
                        visualizar('s', 1, x, 1);
                        x += 3;
                    }
                    
                    visualizar('n', resultado, x, 1);
                    break;
                default:
                    resultado = operar(oper, oper_1, oper_2);
                    visualizar('n', resultado, 0, 1);
            }
        }
    }
}

void init_config(void) {
    ADCON1 = 0X06; // Convirtiendo puerto A como puerto de entrada digital
    // Configurando puerto A
    TRISA = 0B00000000; // Salidas para los enables
    LATA = 0;
    PORTA = 0;
    // Configurando puerto B
    TRISB = 0B11111111; // Entrada de los operadores
    LATB = 0;
    PORTB = 0B11111111;
    // Configurando puerto C
    TRISC = 0B11111111; // Entrada del operador
    LATC = 0;
    PORTC = 0;
    // Configurando puerto D
    TRISD = 0B00000000; // Salida para las matrices de leds
    LATD = 0;
    PORTD = 0;
    // Configurando puerto E
    TRISE = 0B00000000; // Salidas para los decos
    LATE = 0;
    PORTE = 0;
}

/**
 * Visualiza un n�mero o s�mbolo
 * @param SoN Especif�ca si quiere mostrar un S�mbolo(s) o N�mero(n)
 * @param position Especifica el n�mero o s�mbolo que quiere mostrar
 * @param from Desde que columna lo quiere mostrar (empezando desde 0)
 * @param enable Espeficia el enamble a activar
 */
void visualizar(char SoN, char position, char from, char enable) {
    for (char x = 0; x < 3; x++) {
        if (enable == 1) {
            ENABLE1 = 1;
            ENABLE2 = 0;
        } else {
            ENABLE1 = 0;
            ENABLE2 = 1;
        }
        LATD = tolower(SoN) == 's' ? SIMBOLO[position][x] : NUMERO[position][x];
        LATE = x + from;
        __delay_ms(5);
    }
}

void see_full_operation(char oper_1, char oper_2, char oper) {
    visualizar('n', oper_1, 0, 1);
    visualizar('s', oper, 4, 1);
    visualizar('n', oper_2, 0, 2);
    visualizar('s', 10, 4, 2);
    __delay_ms(5);
}

char operar(char oper, char oper_1, char oper_2) {
    switch (oper) {
        case 0:
            return suma(oper_1, oper_2);
            break;
        case 1:
            return resta(oper_1, oper_2);
            break;
        case 2:
            return multi(oper_1, oper_2);
            break;
        case 3:
            return division(oper_1, oper_2);
            break;
        case 4:
            return module(oper_1, oper_2);
            break;
        case 5:
            return and(oper_1, oper_2);
            break;
        case 6:
            return or(oper_1, oper_2);
            break;
        case 7:
            return nand(oper_1, oper_2);
            break;
        case 8:
            return xor(oper_1, oper_2);
            break;
        case 9:
            return no_es_primo(oper_1);
            break;
        case 10:
            return oper_1 == 2;
            break;
        default:
            break;
    }
}

char suma(char oper_1, char oper_2) {
    return oper_1 + oper_2;
}

char resta(char oper_1, char oper_2) {
    char result = oper_1 - oper_2;
    /*if (oper_2 > oper_1) {
        result *= -1;
    }*/
    return oper_1 - oper_2;
}

char multi(char oper_1, char oper_2) {
    return oper_1 * oper_2;
}

char division(char oper_1, char oper_2) {
    char result = oper_1 / oper_2;
    return result;
}

char module(char oper_1, char oper_2) {
    return oper_1 % oper_2;
}

char and(char oper_1, char oper_2) {
    return oper_1 & oper_2;
}

char or(char oper_1, char oper_2) {
    return oper_1 | oper_2;
}

char nand(char oper_1, char oper_2) {
    return ~(and(oper_1, oper_2));
}

char xor(char oper_1, char oper_2) {
    return oper_1 ^ oper_2;
}

char no_es_primo(char num) {
    if (!module(num, 2)) {
        return 0;
    }
    return 1;
}
