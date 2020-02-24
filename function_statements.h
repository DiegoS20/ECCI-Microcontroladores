/* 
 * File:   function_statements.h
 * Author: user
 *
 * Created on 10 de febrero de 2020, 08:37 PM
 */

#ifndef FUNCTION_STATEMENTS_H
#define	FUNCTION_STATEMENTS_H

void init_config();

int multi(char oper_1, char oper_2);
int division(char oper_1, char oper_2);
int suma(char oper_1, char oper_2);
int resta(char oper_1, char oper_2);
int module(char oper_1, char oper_2);
int and(char oper_1, char oper_2);
int or(char oper_1, char oper_2);
int nand(char oper_1, char oper_2);
int xor(char oper_1, char oper_2);
int operar(char oper, char oper_1, char oper_2);
void visualizar(char SoN[2], char position, int from, int enable);

#endif	/* FUNCTION_STATEMENTS_H */

