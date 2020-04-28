/*
 * File:   main.c
 * Author: user
 *
 * Created on 24 de abril de 2020, 09:54 PM
 */


#include <xc.h>
#include <string.h>
#include <stdlib.h>
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
void init_lcd(void);
void init_serial_com(void);
void R_I(char dato);
void R_D(char dato);
void print_message_onLCD(char message[20], char place, char space_bt);
char get_letter(char code);
char get_rand_num(char from, char to);
char reset_game(void);
char compare_nums(char nums[10], char r[10]);
void clear_vec(int *vec[]);
void win_protocol(void);
void game_won(void);
void loss_protocol(void);
void erase_letter(void);
void clean_screen(void);

/** Global variables */
char first_exec = 1;
char counting_down = 0;
char nums[10];
char ans[10];
char lvl;
int lvl_n[3] = {4, 6, 10};
char attempt = 1;
char next;
char typing;
char e_pressed;

void main(void) {
    initial_config();
    init_lcd();
    init_serial_com();
    
    print_message_onLCD("DIEGO SARAVIA", 128, 0);
    print_message_onLCD("84010", 192, 0);
    __delay_ms(2000);
    clean_screen();
    
    print_message_onLCD("CONCENTRESE", 128, 0);
    print_message_onLCD("Iniciando...", 192, 0);
    __delay_ms(2000);
    clean_screen();
    
    srand(13);
    R_I(128);
    
    while(1) {
        for (char i = 0; i < 4; i++) {
            LATB = ~(1<<i);
            __delay_us(10);
        }
        
        if (!typing) {
            for (char i = 0; i < lvl_n[lvl]; i++) {
                nums[i] = get_rand_num(0, 10);
            }
            print_message_onLCD(nums, 128, 1);
            __delay_ms(2000);
            clean_screen();
            print_message_onLCD("Escribe: ", 128, 0);
            R_I(192);
        }
        
        counting_down = 0;
        typing = 1;
        if (e_pressed) {
            clean_screen();
            typing = 0;
            counting_down = 1;
            if (compare_nums(nums, ans)) {
                win_protocol();
            } else {
                loss_protocol();
            }
            e_pressed = 0;
        }
        
        first_exec = 0;
    }
    return;
}

void initial_config(void) {
    TRISB = 0B11110000; // Pines RB7:RB4 como entradas y RB3:RB0 como salidas
    TRISC = 0B10000000;
    TRISD = 0X00;
    
    // Limpiando basura
    LATB = 0XFF; // Condiciones iniciales para asegurar el cambio de estado
    LATC = 0;
    LATD = 0;
    PORTB = 0XFF; // Condiciones iniciales para asegurar el cambio de estado
    PORTC = 0;
    PORTD = 0;
}

void init_lcd(void) {
    __delay_ms(20);
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

void print_message_onLCD(char message[20], char place, char space_bt) {
    if (place > 0) {
        R_I(place);
    }
    for (char i = 0; i < strlen(message); i++) {
        R_D(message[i]);
        if (space_bt) {
            R_D(' ');
        }
    }
}

void interrupt capture(void) {
    if (INTCONbits.RBIF) {
        char p = PORTB;
        while((PORTB & 0XF0) != 0XF0);
        __delay_us(50); // Antirrebote
        
        if (!counting_down && !first_exec) {
            char m[16];
            sprintf(m, "%c", get_letter(p));
            if (m[0] != 'b' && m[0] != 'e') {
                ans[strlen(ans)] = m[0];
                print_message_onLCD(m, 0, 0);
            } else if(m[0] == 'e') {
                e_pressed = 1;
            } else if(m[0] == 'b') {
                erase_letter();
            }
        }
        
        INTCONbits.RBIF = 0; // Borrando bandera de interrupción
    }
}

char get_letter(char code) {
    switch(code) {
        case 0XEE:
            return '1';
        case 0XDE:
            return '2';
        case 0XBE:
            return '3';
        case 0XED:
            return '4';
        case 0XDD:
            return '5';
        case 0XBD:
            return '6';
        case 0XEB:
            return '7';
        case 0XDB:
            return '8';
        case 0XBB:
            return '9';
        case 0XD7:
            return '0';
        case 0X77:
            return 'e';
        case '~':
            return 'b';
        default:
            break;
    }
}

char get_rand_num(char from, char to) {
    int num = rand() % (to-from+1) + from;
    char r[1];
    sprintf(r, "%i", num);
    return r[0];
}

char reset_game() {
    attempt = 1;
    lvl = 0;
    clear_vec(&ans);
    clear_vec(&nums);
}

void clear_vec(int *vec[]) {
    char vec_len = strlen(vec);
    for (char i = 0; i < vec_len; i++) {
        vec[i] = '\0';
    }
}

char compare_nums(char nums[10], char r[10]) {    
    for(char i = 0; i < strlen(r); i++) {
        if (r[i] != nums[i]) {
            return 0;
        }
    }
    return 1;
}

void win_protocol(void) {
    print_message_onLCD("Ganaste", 128, 0);
    __delay_ms(2000);
    if (attempt < 3) {
        print_message_onLCD("Otra vez...", 128, 0);
        attempt++;
    } else {
        attempt = 1;
        lvl++;
        if (lvl == 3) {
            game_won();
            return;
        } else {
            print_message_onLCD("Sig. nivel", 128, 0);
        }
    }
    clear_vec(&ans);
    __delay_ms(2000);
    clean_screen();
}

void game_won(){
    print_message_onLCD("¡¡Has", 128, 0);
    print_message_onLCD("Ganado!!", 192, 0);
    reset_game();
    __delay_ms(2000);
    R_I(0X01);
    __delay_ms(100);
}

void loss_protocol(void) {
    print_message_onLCD("Perdiste", 128, 0);
    __delay_ms(1000);
    clean_screen();
    
    print_message_onLCD(nums, 128, 0);
    print_message_onLCD(ans, 192, 0);
    __delay_ms(3000);
    clean_screen();
    
    print_message_onLCD("Reiniciando", 128, 0);
    print_message_onLCD("Juego", 192, 0);
    __delay_ms(3000);
    
    reset_game();
    clean_screen();
}

void erase_letter(void) {
    
}

void clean_screen(void) {
    R_I(0X01);
    __delay_ms(100);
}
