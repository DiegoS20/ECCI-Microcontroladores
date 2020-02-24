/* 
 * File:   function_statements.h
 * Author: user
 *
 * Created on 10 de febrero de 2020, 08:37 PM
 */

#ifndef FUNCTION_STATEMENTS_H
#define	FUNCTION_STATEMENTS_H

void init_config();

char multi(char oper_1, char oper_2);
char division(char oper_1, char oper_2);
char suma(char oper_1, char oper_2);
char resta(char oper_1, char oper_2);
char module(char oper_1, char oper_2);
char and(char oper_1, char oper_2);
char or(char oper_1, char oper_2);
char nand(char oper_1, char oper_2);
char xor(char oper_1, char oper_2);
char operar(char oper, char oper_1, char oper_2);
char no_es_primo(char num);
void visualizar(char SoN[2], char position, char from, char enable);

#endif	/* FUNCTION_STATEMENTS_H */

