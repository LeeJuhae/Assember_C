/*
* ��  �� : �� ���α׷��� SIC/XE �ӽ��� ���� ������ Assembler ���α׷��� ���η�ƾ����,
* �Էµ� ������ �ڵ� ��, ��ɾ �ش��ϴ� OPCODE�� ã�� ����Ѵ�.
*/

/*
* ���α׷��� ����� �����Ѵ�.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "my_assembler_20160311.h"

/*token_unit���� nixbpe�� �����Ҷ� ����� ��ũ�� ���*/
#define n_flag 32
#define i_flag 16
#define x_flag 8
#define b_flag 4
#define p_flag 2
#define e_flag 1
/* --------------------------------------------------------------------------------
* ���� : ����ڷ� ���� ����� ������ �޾Ƽ� ��ɾ��� OPCODE�� ã�� ����Ѵ�.
* �Ű� : ���� ����, ����� ����
* ��ȯ : ���� = 0, ���� = < 0
* ���� : ���� ����� ���α׷��� ����Ʈ ������ �����ϴ� ��ƾ�� ������ �ʾҴ�.
*		   ���� �߰������� �������� �ʴ´�.
* ---------------------------------------------------------------------------------
*/
int main(int args, char *arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler : ���α׷� �ʱ�ȭ�� ���� �߽��ϴ�.\n");
		return -1;
	}
	
	if (assem_pass1() < 0) {
		printf("assem_pass1 : �н� 1 �������� �����Ͽ����ϴ�.\n");
		return -1;
	}
	//make_opcode_output("output_20160311");
	//make_opcode_output(NULL);

	//make_symtab_output(NULL);
	make_symtab_output("symtab_20160311");
	
	if(assem_pass2() < 0 ){
		printf(" assem_pass2: �н�2 �������� �����Ͽ����ϴ�.  \n") ;
		return -1 ;
	}

	make_objectcode_output("output_20160311") ;
	//make_objectcode_output(NULL);
	return 0;
}

/* ----------------------------------------------------------------------------------
* ���� : ���α׷� �ʱ�ȭ�� ���� �ڷᱸ�� ���� �� ������ �д� �Լ��̴�.
* �Ű� : ����
* ��ȯ : �������� = 0 , ���� �߻� = -1
* ���� : ������ ��ɾ� ���̺��� ���ο� �������� �ʰ� ������ �����ϰ� �ϱ�
*		   ���ؼ� ���� ������ �����Ͽ� ���α׷� �ʱ�ȭ�� ���� ������ �о� �� �� �ֵ���
*		   �����Ͽ���.
* ----------------------------------------------------------------------------------
*/
int init_my_assembler(void)
{
	int result;

	if ((result = init_inst_file("inst.data")) < 0) {
		return -1;
	}
	if ((result = init_input_file("input.txt")) < 0) {
		return -1;
	}
	return result;
}

/* ----------------------------------------------------------------------------------
* ���� : �ӽ��� ���� ��� �ڵ��� ������ �о� ���� ��� ���̺�(inst_table)��
*        �����ϴ� �Լ��̴�.
* �Ű� : ���� ��� ����
* ��ȯ : �������� = 0 , ���� < 0
* ���� : ���� ������� ������ �����Ӱ� �����Ѵ�. ���ô� ������ ����.
*
*	===============================================================================
*		   | �̸� | ���� | ���� �ڵ� | ���۷����� ���� | NULL|
*	===============================================================================
*
* ----------------------------------------------------------------------------------
*/
int init_inst_file(char *inst_file)
{
	FILE * file;
	int errno;
	file = fopen(inst_file, "rt");
	if (file == NULL)
		errno = -1;
	else {
		errno = 0;
		inst_index = 0;
		while (!feof(file)) {
			inst_table[inst_index] = (inst *)calloc(1,sizeof(inst));
			inst_table[inst_index]->mnemo = (char*)calloc(10,sizeof(char));
			inst_table[inst_index]->format = (char*)calloc(3, sizeof(char));
			inst_table[inst_index]->opcode = (char*)calloc(2, sizeof(char));
			
			/*inst data������ ������ inst_table�� �Է�*/
			fscanf(file,"%s %s %s %d", inst_table[inst_index]->mnemo,inst_table[inst_index]->format,inst_table[inst_index]->opcode, &(inst_table[inst_index]->opcount));
			inst_index++;
		}
	}
	return errno;
}
/* ----------------------------------------------------------------------------------
* ���� : ����� �� �ҽ��ڵ带 �о� �ҽ��ڵ� ���̺�(input_data)�� �����ϴ� �Լ��̴�.
* �Ű� : ������� �ҽ����ϸ�
* ��ȯ : �������� = 0 , ���� < 0
* ���� : ���δ����� �����Ѵ�.
*
* ----------------------------------------------------------------------------------
*/
int init_input_file(char *input_file)
{
	FILE * file;
	int errno;
	file = fopen(input_file, "rt");
	if (file == NULL)
		errno = -1;
	else {
		errno = 0;
		line_num = 0;
		while (!feof(file)) {
			input_data[line_num] = (char *)calloc(80,sizeof(char));
			
			/*input �ؽ�Ʈ ���Ͽ��� ���پ� input_data�� �־���*/
			fgets(input_data[line_num], sizeof(input_data[line_num])*20, file);
			line_num++;
		}
	}
	return errno;
}
/* ----------------------------------------------------------------------------------
* ���� : �ҽ� �ڵ带 �о�� �Ľ��� ���ϴ� ���ڿ�(str)���� tab�� ������ ��������
		 ��ū������ �м��ϰ� ��ū ���̺��� �ۼ��ϴ� �Լ��̴�. 
		 str���� tab�� ���������� tabcount�� ������Ų��.
*        �н� 1�� ���� ȣ��ȴ�.
* �Ű� : �Ľ��� ���ϴ� ���ڿ�
* ��ȯ : �������� = 0 , ���� < 0
* ���� : my_assembler ���α׷������� ���δ����� ��ū �� ������Ʈ ������ �ϰ� �ִ�.
* ----------------------------------------------------------------------------------
*/
int token_parsing(char *str)
{
	int tabcount = 0;
	int errno;
	token_table[token_line] = (token *)calloc(1, sizeof(token));
	token_table[token_line]-> label = (char*)calloc(10, sizeof(char));
	token_table[token_line]-> operator = (char*)calloc(30, sizeof(char));
	token_table[token_line]->comment = (char*)calloc(40, sizeof(char));
	
	for (int i = 0, j = 0; ; i++) {
		if (str[i] == '\n') {
			if (token_table[token_line]->isOperator == 0) {
				token_table[token_line]->isOperator = -1;
			}
			token_line++;
			break;
		}
		else if (str[i] == '\t') {
			tabcount++;
			j = 0;
		}
		else if (str[i] == 0) { // ������ ���κ� �϶� for�� Ż��
			break;
		}
		else {
			/*tab�� 0�� ������ ��  str[i]�� label�� �Է�*/
			if (tabcount == 0) {
				token_table[token_line]->label[j] = str[i];
				j++;
			}
			/*tab�� 1�� ������ ��  str[i]�� operator�� �Է�*/
			else if (tabcount == 1) {
				token_table[token_line]->operator[j] = str[i];
				token_table[token_line]->isOperator = search_opcode(token_table[token_line]->operator);
				j++;
			}
			/*tab�� 2�� ������ �� str[i]�� operand�� �Է�*/
			else if (tabcount == 2) {
				//	token_table[token_line]->isOperator = search_opcode(token_table[token_line]->operator);
					/*token�� operator�� ��ɾ� �� ���*/
				if (token_table[token_line]->isOperator >= 0) {

					int inst_opcount = inst_table[token_table[token_line]->isOperator]->opcount; // operator�� operand������ ����.
					for (int k = 0; k < inst_opcount; k++) { // inst.data�� �����ִ� operator�� operand�� ������ŭ loop
						token_table[token_line]->operand[k] = (char*)calloc(40, sizeof(char));
						for (;; i++, j++) {
							if (str[i] == '\t') {
								tabcount++;
								/*operand�� ������ 2�ε� input���Ͽ� ���� operand�� 1���� ��� ex)CLEAR A , CLEAR X*/
								if (tabcount == 4)
									tabcount--;
								break;
							}
							else if (str[i] == '\n') {
								i--;
								break;
							}
							else if (str[i] == ',') {
								/* operand ������ 1�϶� x�������� ����ϴ� ��� x�� operand[2]�� �־���. ex) +LDCH	BUFFER,X */
								if ((inst_opcount == 1) && ((str[i + 1] == 'X') || (str[i + 1] == 'x'))) {
									token_table[token_line]->operand[2] = (char*)calloc(40, sizeof(char));
									token_table[token_line]->operand[2][0] = str[i + 1];
								}
								j = 0;
								i++;
								break;
							}
							token_table[token_line]->operand[k][j] = str[i];
						}
					}
				}
				/*token�� operator�� ��ɾ �ƴ� ���*/
				else {
					token_table[token_line]->operand[0] = (char*)calloc(40, sizeof(char));

					for (j = 0; ; j++, i++) {
						if (str[i] == '\t') {
							tabcount++;
							break;
						}
						else if (str[i] == '\n') {
							i--;
							break;
						}
						else if (str[i] == 0) { // ������ ���� ������ ��
							i--;
							break;
						}
						token_table[token_line]->operand[0][j] = str[i];
					}
				}
				j = 0;
			}
			/*tab�� 3�� ������ �� str[i]�� comment�� �Է�*/
			else if (tabcount == 3) {
				token_table[token_line]->comment[j] = str[i];
				j++;
			}
			else
				errno = -1;
		}
	}
	return errno;
}
/* ----------------------------------------------------------------------------------
* ���� : �Է� ���ڿ��� ���� �ڵ������� �˻��ϴ� �Լ��̴�.
* �Ű� : ��ū ������ ���е� ���ڿ�
* ��ȯ : �������� = ���� ���̺� �ε���, ���� < 0
* ----------------------------------------------------------------------------------
*/
int search_opcode(char *str)
{
	int res;
	if (str[0] == '+')
		str = str++;
	for (int k = 0; k < inst_index; k++) {
		res = strcmp(str, inst_table[k]->mnemo);
		/*��ū ������ ���е� ���ڿ�(str)�� inst.data ������ ��ɾ��� ���Ͽ� �ϳ��� ������*/
		if (res == 0) {
			return k;
		}
	}
	return -1;
}
/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �н�1������ �����ϴ� �Լ��̴�.
*		   �н�1������..
*		   1. ���α׷� �ҽ��� ��ĵ�Ͽ� �ش��ϴ� ��ū������ �и��Ͽ� ���α׷� ���κ� ��ū
*		   ���̺��� �����Ѵ�.
*
* �Ű� : ����
* ��ȯ : ���� ���� = 0 , ���� = < 0
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	int errno;
	int x = 0;
	for (int i = 0; i < line_num; i++) {
		/* input_data�� ���ڿ��� ���پ� �Է� �޾Ƽ� token_parsing()�� ȣ���Ͽ� token_unit�� ����*/
		errno = token_parsing(input_data[i]);
		/*token_parsing()�� ���ϰ��� -1�϶� */
		if (errno < 0)
			return errno;
		/*���ͷ� ����ü�� which_section�� addr�� ��� -1�� �ʱ�ȭ��Ŵ*/
		lit_table[i].which_section = -1;
		lit_table[i].addr = -1;
	}
	for (int i = 0; i < line_num; i++) {
		/*���� ����ü�� �����̸��� ���ǹ�ȣ�� ��������*/
		if (i == 0)
			strcpy(section_table[section_num].sect_name, token_table[i]->label);
		else if (!strcmp(token_table[i]->operator,"CSECT")) {
			section_num++;
			strcpy(section_table[section_num].sect_name, token_table[i]->label);
		}
		token_table[i]->which_section = section_num;

		/*���ͷ� ����ü�� ���ͷ��� ��� ������ ���ͷ����� ������*/
		if ((token_table[i]->operand[0] != NULL) && (token_table[i]->operand[0][0] == '=')) {
			strcpy(lit_table[literal_line].literal, token_table[i]->operand[0]);
			lit_table[literal_line].which_section = token_table[i]->which_section;
			literal_line++;
		}
	}
	for (int i = 0; i < literal_line; i++) {
		for (int j = i+1; j < literal_line; j++) {
			/*���ͷ� ����ü�� �ߺ��Ǵ� ���� ������ ���� ����*/
			if ((!strcmp(lit_table[i].literal, lit_table[j].literal)) && (lit_table[i].which_section == lit_table[j].which_section)) {
				strcpy(lit_table[j].literal,"");
				lit_table[j].addr = -1;
				lit_table[j].which_section = -1;
				x = 1;
			}
		}
	}
	/*���ͷ� ���������� literal_line������ 1 ���ҽ�Ŵ*/
	if (x == 1)
		literal_line--;
	
	assign_memory_loc();
	return 0;
}
/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ ��ɾ� ���� OPCODE�� ��ϵ� ǥ(���� 4��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*        ���� ���� 4�������� ���̴� �Լ��̹Ƿ� ������ ������Ʈ������ ������ �ʴ´�.
* -----------------------------------------------------------------------------------
*/

void make_opcode_output(char *filename)
{
	FILE * file;
	int isOperator = 0;
	token_line = 0;
	if (filename == NULL)
	{
		file = stdout;
	}
	else {
		file = fopen(filename, "wt");
	}
	for (int i = 0; i < line_num; i++, token_line++) {
		isOperator = search_opcode(token_table[token_line]->operator);

		/*token�� label�� ""�� �ƴ� ���*/
		if (strcmp(token_table[i]->label, "") != 0)
			fprintf(file, "%s\t", token_table[i]->label);
		/*token�� label�� ""�� ���*/
		else
			fprintf(file, "\t");

		/*token�� operator�� ""�� �ƴ� ���*/
		if (strcmp(token_table[i]->operator, "") != 0)
			fprintf(file, "%s\t", token_table[i]->operator);
		/*token�� operator��  ""�� ���*/
		else
			fprintf(file, "\t");

		/*token�� operator�� ��ɾ� �� ��� (���� ���̺� �ε��� >=0)*/
		if (isOperator >= 0) {

			/*��ɾ��� operand�� 0���� ���*/
			if (inst_table[isOperator]->opcount == 0)
				fprintf(file, "\t");

			/*��ɾ��� operand�� 1�� �̻��� ���*/
			for (int j = 0; j < inst_table[isOperator]->opcount; j++) {

				/*operand�� ""�� �ƴҰ��*/
				if (strcmp(token_table[i]->operand[j], "") != 0) {
					/*operand�� 2�� �̻��϶�, ','�� ����ֱ� ���� �ڵ� ex) COMPR A,S*/
					if ((0 < j) && (j <= (inst_table[isOperator]->opcount) - 1))
						fprintf(file, ",");

					fprintf(file, "%s", token_table[i]->operand[j]);

					/*token_table[i]->operand[j]�� ������ operand�϶�*/
					if (j == inst_table[isOperator]->opcount - 1) {
						/*token_talbe[i]->operand[2]�� ������ ��*/
						if (token_table[i]->operand[2] != NULL) {
							fprintf(file, ",%s", token_table[i]->operand[2]);
						}
						fprintf(file, "\t");
					}
				}
				/*operand�� ""�� ���*/
				else {
					fprintf(file, "\t");
					break;
				}
			}
		}
		/* token�� operator�� ��ɾ �ƴ� ���(���� ���̺� �ε��� = -1)*/
		else {
			if (token_table[token_line]->operand[0] != NULL)
				fprintf(file, "%s\t", token_table[token_line]->operand[0]);
			else
				fprintf(file, "\t");
		}

		/*token�� operator�� ��ɾ� �� ��� (���� ���̺� �ε��� >=0)*/
		if (isOperator >= 0) {
			fprintf(file, "%s\n", inst_table[isOperator]->opcode);
		}
		/* token�� operator�� ��ɾ �ƴ� ���(���� ���̺� �ε��� = -1)*/
		else
			if (i < line_num - 2)
				fprintf(file, "\n");
	}
	fclose(file);
}

/* ----------------------------------------------------------------------------------
* ���� : �޸𸮸� �Ҵ����ִ� �Լ��̴�.
* �Ű� : ����
* ��ȯ : ����
* ���� :
* -----------------------------------------------------------------------------------
*/
void assign_memory_loc() {
	char * equ_operator;// EQU ���þ �������� operator(+,-,*,/)�� �����ϱ����� ����� char�� ������
	char * equ_operand[10];//EQU ���þ �������� operand�� �����ϱ����� ����� char�� ������ �迭
	int equ_operand_value[10];
	equ_operator = (char *)calloc(9, sizeof(char));
	for(int i = 0 ; i < 10 ; i++)
		equ_operand[i] = (char *)calloc(10, sizeof(char));
	int p = 0;//equ_operand ������ �迭�� ���ȣ�� ������.
	section_num = 0;//section num�� 0���� �ʱ�ȭ����.

	for (int i = 0; i <= line_num; i++) {
		if (i == 0) {
			locctr = 0;
		}
		else if (!strcmp(token_table[i - 1]->operator,"CSECT")) { //CSECT�� ������ �ٷ� ���������� �ش��ϴ� ������ ���̸� ���Ǳ���ü�� ��������.
			section_table[section_num].sect_length = locctr;
			section_num++;
			for (int j = 0; j < literal_line; j++) {
				/*LTORG ���� ���ͷ� ���Ǵ°�� assem_pass1()���� �̸� �־�� lit_table�� literal���� ������ ���ͷ��� �ּҰ��� �Ҵ�����.*/
				if ((token_table[i - 2]->which_section == lit_table[j].which_section) && (lit_table[j].addr == -1)) {
					lit_table[j].addr = locctr;
					section_table[lit_table[j].which_section].sect_length = locctr;
				}
			}
			locctr = 0;
			token_table[i-1]->itsloc = locctr;
		}
		/*���þ� END�� �������� �ּ��Ҵ�*/
		else if ((!strcmp(token_table[i - 1]->operator,"END"))&&(i== line_num)) {
			for (int j = 0; j < literal_line; j++) {
				if ((token_table[i - 1]->which_section == lit_table[j].which_section) && (lit_table[j].addr == -1)) {
					if (lit_table[j].literal[1] == 'C') {
						for (int k = 3; ; k++) {
							if (lit_table[j].literal[k] == '\'')
								break;
							locctr++;
						}
					}
					else if (lit_table[j].literal[1] == 'X') {
						for (int k = 3; ; k++) {
							if (lit_table[j].literal[k] == '\'')
								break;
							if (k % 2 == 0)
								locctr++;
						}
					}
					lit_table[j].addr = locctr;
				section_table[lit_table[j].which_section].sect_length = locctr;
				}
			}
		}
		else {
			if (search_opcode(token_table[i-1]->operator) >= 0) {//operator�� ��ɾ��� ���
				if (!strcmp(inst_table[search_opcode(token_table[i-1]->operator)]->format, "1"))
					locctr++;
				else if (!strcmp(inst_table[search_opcode(token_table[i-1]->operator)]->format, "2"))
					locctr += 2;
				else
					locctr += 3;
				if (token_table[i-1]->operator[0] == '+')//4�����϶�
					locctr++;
			}
			else {//operator�� ��ɾ �ƴ� ���
				  /*operator�ڸ��� RESW�� ��������*/
				if (!strcmp(token_table[i - 1]->operator,"RESW")) 
					locctr += 3 * atoi(token_table[i - 1]->operand[0]);
				/*operator�ڸ��� RESB�� ��������*/
				else if (!strcmp(token_table[i - 1]->operator,"RESB")) 
					locctr += atoi(token_table[i - 1]->operand[0]);
				/*operator�ڸ��� LTORG�� ��������*/
				else if (!strcmp(token_table[i - 1]->operator, "LTORG")) {
					for (int j = 0;j < i-1 ; j++) {
						if (lit_table[j].which_section == token_table[i - 1]->which_section) {
							lit_table[j].addr = locctr;
							if (lit_table[j].literal[1] == 'C') {
								for (int k = 3; ; k++) {
									if (lit_table[j].literal[k] == '\'')
										break;
									locctr++;
								}
							}
							else if (lit_table[j].literal[1] == 'X') {
								for (int k = 3; ; k++) {
									if (lit_table[j].literal[k] == '\'')
										break;
									if (k % 2 == 0)
										locctr++;
								}
							}
						}
					}
				}
				else if (!strcmp(token_table[i - 1]->operator,"EQU")) {
					/*EQU�� �������� operand�� *�϶�*/
					if (!strcmp(token_table[i - 1]->operand[0], "*")) {
						token_table[i - 1]->itsloc = locctr;
					
					}
					else {
						token_table[i - 1]->itsloc = 0;
						for (int k = 0, m = 0, q= 0; k<strlen(token_table[i - 1]->operand[0]); k++) {
							if ((token_table[i - 1]->operand[0][k] == '+') || (token_table[i - 1]->operand[0][k] == '-') || (token_table[i - 1]->operand[0][k] == '*') || (token_table[i - 1]->operand[0][k] == '/')) {
								equ_operator[m] = token_table[i - 1]->operand[0][k];
								p++;
								q = 0;
								m++;
							}
							else {
								equ_operand[p][q] = token_table[i - 1]->operand[0][k];
								q++;
							}
						}
						p++;
						for (int k = 0, m = 0; k <token_line; k++) {
							if (!strcmp(equ_operand[m],"")) 
								break;
							if (!strcmp(equ_operand[m], token_table[k]->label)) {
								equ_operand_value[m] = token_table[k]->itsloc;
								m++;
								k = -1;
							}
						}
						if(equ_operator[0] == '+')
							token_table[i - 1]->itsloc = equ_operand_value[0] + equ_operand_value[1];
						else if (equ_operator[0] == '-')
							token_table[i - 1]->itsloc = equ_operand_value[0] - equ_operand_value[1];
						else if (equ_operator[0] == '*')
							token_table[i - 1]->itsloc = equ_operand_value[0] * equ_operand_value[1];
						else if (equ_operator[0] == '/')
							token_table[i - 1]->itsloc = equ_operand_value[0] / equ_operand_value[1];
					}
				}
				/*operator�ڸ��� BYTE�� ��������*/
				else if (!strcmp(token_table[i - 1]->operator,"BYTE")) {
					if (token_table[i - 1]->operand[0][0] == 'C') {
						for (int k = 2; ; k++) {
							if (token_table[i - 1]->operand[0][k] == '\'')
								break;
							locctr++;
						}
					}
					else if (token_table[i - 1]->operand[0][0] == 'X') {
						for (int k = 2; ; k++) {
							if (token_table[i - 1]->operand[0][k] == '\'')
								break;
							if (k % 2 == 0)
								locctr++;
						}
					}
				}
				/*operator�ڸ��� WORD�� ��������*/	
				else if (!strcmp(token_table[i - 1]->operator,"WORD"))
					locctr +=3;
			}
			if (strcmp(token_table[i - 1]->operator,"EQU")) {
				token_table[i]->itsloc = locctr;
				if (i == line_num - 1)
					section_table[section_num].sect_length = locctr;
			}
		}
	}
	/*symbol table ����*/
	for(int i = 0 ; i < line_num ; i++)
		if (strcmp(token_table[i]->label, "")&& strcmp(token_table[i]->label, ".")) {
			strcpy(sym_table[symbol_line].symbol, token_table[i]->label);
			sym_table[symbol_line].addr = token_table[i]->itsloc;
			sym_table[symbol_line].sect_num = token_table[i]->which_section;
			symbol_line++;
		}
}
/* ----------------------------------------------------------------------------------
* ���� : object code�� 10���� ���¸� 16������ ������ִ� �Լ��̴�.
* �Ű� : object code�� 10���� ����(int int_obcode), �ش��ϴ� line(int line) 
* ��ȯ : ����
* ���� : 
* -----------------------------------------------------------------------------------
*/
void make_obcode(int int_obcode, int line) {
	int i;
	/*4�����϶�*/
	if (token_table[line]->operator[0] == '+')
		i =  7;
	/*1,2,3���� �϶�*/
	else 
		i = 2 * (inst_table[token_table[line]->isOperator]->format[0] - '0') - 1;
	for (; i >= 0; i--) {
		if ((int_obcode & 15) >= 10) 
			token_table[line]->obcode[i] = (int_obcode & 15) + 55;
		else if (((int_obcode & 15) <= 9) && ((int_obcode & 15) >= 0))
			token_table[line]->obcode[i] = (int_obcode & 15) + '0';
		int_obcode = int_obcode >> 4;
	}
}

void make_symtab_output(char *filename) {
	FILE * file;
	if (filename == NULL)
		file = stdout;
	else 
		file = fopen(filename, "wt");
	for (int i = 0; i < symbol_line; i++) 
		fprintf(file, "%s\t\t%X\n", sym_table[i].symbol, sym_table[i].addr);
	if (file != stdout)
		fclose(file);
}

/* ----------------------------------------------------------------------------------
* ���� : ����� �ڵ带 ���� �ڵ�� �ٲٱ� ���� �н�2 ������ �����ϴ� �Լ��̴�.
*		   �н� 2������ ���α׷��� ����� �ٲٴ� �۾��� ���� ������ ����ȴ�.
*		   ������ ���� �۾��� ����Ǿ� ����.
*		   1. ������ �ش� ����� ��ɾ ����� �ٲٴ� �۾��� �����Ѵ�.
* �Ű� : ����
* ��ȯ : �������� = 0, �����߻� = < 0
* ���� :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
	int x, y=0, ta, pc;//x,y : object code������������ ��Ʈ�����  ����� ����, ta : Ÿ���ּ�, pc : pc�� 
	char  *str;
	char *str2;
	str = (char *)calloc(3, sizeof(char));
	str2 = (char *)calloc(10, sizeof(char));
	char * word_operator;// EQU ���þ �������� operator(+,-,*,/)�� �����ϱ����� ����� char�� ������
	char * word_operand[10];//EQU ���þ �������� operand�� �����ϱ����� ����� char�� ������ �迭

	word_operator = (char *)calloc(9, sizeof(char));
	for (int i = 0; i < 10; i++)
		word_operand[i] = (char *)calloc(10, sizeof(char));
	int p = 0;

	for (int i = 0; i < line_num; i++) {
		token_table[i]->obcode = (char *)calloc(10, sizeof(char));
		/*��ɾ� �϶�*/
		if (token_table[i]->isOperator >= 0) {
			x = y = 0;
			ta = pc = 0;
			token_table[i]->nixbpe = 0;
			/*��ɾ��� opcode�� object code�� �־���*/
			for (int j = 0; j < 2; j++) {
				if ((inst_table[token_table[i]->isOperator]->opcode[j] >= '0') && (inst_table[token_table[i]->isOperator]->opcode[j] <= '9'))
					x = (x | (inst_table[token_table[i]->isOperator]->opcode[j] - '0'));
				else
					x = (x | (inst_table[token_table[i]->isOperator]->opcode[j] - 55));
				if (token_table[i]->operator[0] == '+') { //4�����϶�
					if (j == 0)
						x = x << 4;
					else
						x = x << 24;
				}
				else {
					if (j == 0)
						x = x << 4;
					else
						x = x << 8 * ((inst_table[token_table[i]->isOperator]->format[0]) - '0' - 1);
				}
			}
			y = x;
			x = 0;
			/*��ɾ 3 �Ǵ� 4���� �϶�*/
			if (!strcmp(inst_table[token_table[i]->isOperator]->format, "3/4")) {
				/*��ɾ��� operator�� ������ ��*/
				if (token_table[i]->operand[0]!= NULL){
					/*nixbpe �� n�� i�� ������*/
					if (token_table[i]->operand[0][0] == '#')
						token_table[i]->nixbpe |= i_flag;
					else if (token_table[i]->operand[0][0] == '@')
						token_table[i]->nixbpe |= n_flag;
					else {
						token_table[i]->nixbpe |= n_flag;
						token_table[i]->nixbpe |= i_flag;
					}
					/*nixbpe �� x ������*/
					if (inst_table[token_table[i]->isOperator]->opcount == 1) {
						if (token_table[i]->operand[2] != NULL) {
							token_table[i]->nixbpe |= x_flag;
						}
					}
					/*4�����϶�, b = 0, p = 0, e = 1�� ������*/
					if (token_table[i]->operator[0] == '+') {
						token_table[i]->nixbpe |= e_flag;
					}
					/*3�����϶�, b�� p���� ����*/
					else {
						for (int j = 0; j < MAX_LINES; j++) {
							if (token_table[i]->operand[0][0] == '@') {
								strcpy(str2, token_table[i]->operand[0]);
								str2 = str2++;
							}
							else
								strcpy(str2, token_table[i]->operand[0]);
		
							if (!strcmp(str2, sym_table[j].symbol)) {
								ta = sym_table[j].addr;
								pc = token_table[i + 1]->itsloc;
								break;
							}
						}
						if (token_table[i]->operand[0][0] != '#') {
							if ((ta - pc >= -2048) && (ta - pc <= 2047))
								token_table[i]->nixbpe |= p_flag;
							else
								token_table[i]->nixbpe |= b_flag;
						}
					}
					x = token_table[i]->nixbpe;
					if (token_table[i]->operator[0] == '+')
						x = x << 20;
					else
						x = x << 12;
					y |= x;

					/*disp ����*/
					if (token_table[i]->operand[0][0] == '#') {
						str = token_table[i]->operand[0];
						str = str++;
						y |= atoi(str);
					}
					else if (token_table[i]->operand[0][0] == '=') {
						for (int j = 0; j < literal_line; j++) {
							if ((!strcmp(lit_table[j].literal, token_table[i]->operand[0])) && (lit_table[j].which_section == token_table[i]->which_section)) {
								ta = lit_table[j].addr;
								pc = token_table[i + 1]->itsloc;
								//break;
							}	
						}
					}
					if (token_table[i]->nixbpe & p_flag) {
						if (ta >= pc) 
							y |= ta - pc;
						else
							y |= ((ta - pc) & 0x00000FFF);
					}
					else if (token_table[i]->nixbpe & b_flag)
						;
				}
				else {
					if (!strcmp(token_table[i]->operator,"RSUB")) {
						token_table[i]->nixbpe |= n_flag;
						token_table[i]->nixbpe |= i_flag;
						x = token_table[i]->nixbpe << 12;
						y |= x;
					}
				}
			}
			/*��ɾ 2���� �϶�*/
			else if (!strcmp(inst_table[token_table[i]->isOperator]->format, "2")) {
				for (int j = 0; j < 2; j++) {
					if (token_table[i]->operand[j] != NULL) {
						/*�������� ��ȣ�� ��Ʈ����*/
						if (!strcmp(token_table[i]->operand[j], "A"))
							x |= 0;
						else if (!strcmp(token_table[i]->operand[j], "X"))
							x |= 1;
						else if (!strcmp(token_table[i]->operand[j], "L"))
							x |= 2;
						else if (!strcmp(token_table[i]->operand[j], "B"))
							x |= 3;
						else if (!strcmp(token_table[i]->operand[j], "S"))
							x |= 4;
						else if (!strcmp(token_table[i]->operand[j], "T"))
							x |= 5;
						else if (!strcmp(token_table[i]->operand[j], "F"))
							x |= 6;
						else if (!strcmp(token_table[i]->operand[j], "PC"))
							x |= 8;
						else if (!strcmp(token_table[i]->operand[j], "SW"))
							x |= 9;
					}
					if (j == 0)
						x = x << 4;
				}
				y |= x;
			}		
			make_obcode(y, i);
		}
		/*��ɾ �ƴ� ��*/
		else {
			/*operator�� BYTE�϶�*/
			if (!strcmp(token_table[i]->operator,"BYTE")) {
				if (token_table[i]->operand[0][0] == 'C') {
					for (int k = 2, m = 2; ; k++,m--) {
						if (token_table[i]->operand[0][k] == '\'')
							break;
						x = token_table[i]->operand[0][k];
						x = x << 8 * m;
					}
					y |= x;
				}
				else if (token_table[i]->operand[0][0] == 'X') {
					for (int k = 2, m = 0; ; k++, m++) {
						if (token_table[i]->operand[0][k] == '\'')
							break;
						token_table[i]->obcode[m] = token_table[i]->operand[0][k];
					}
				}
			}
			/*operator�� WORD�϶�*/
			else if (!strcmp(token_table[i]->operator,"WORD")) {
				strcpy(token_table[i]->obcode,"000000");
			}
		}
	}
}

/* ----------------------------------------------------------------------------------
* ���� : �Էµ� ���ڿ��� �̸��� ���� ���Ͽ� ���α׷��� ����� �����ϴ� �Լ��̴�.
*        ���⼭ ��µǴ� ������ object code (������Ʈ 1��) �̴�.
* �Ű� : ������ ������Ʈ ���ϸ�
* ��ȯ : ����
* ���� : ���� ���ڷ� NULL���� ���´ٸ� ���α׷��� ����� ǥ��������� ������
*        ȭ�鿡 ������ش�.
*
* -----------------------------------------------------------------------------------
*/
void make_objectcode_output(char *file_name)
{
	FILE * file;
	char *ptr[10];

	char *str;
	str = (char *)calloc(20, sizeof(char));

	char *str2;
	char num = 15;

	char *str4;
	str4 = (char *)calloc(20, sizeof(char));

	int length = 0;

	if (file_name == NULL)
		file = stdout;
	else
		file = fopen(file_name, "wt");

	/*text_table ����ü ����*/
	for (int i = 0, j = 0; i < line_num; i++) {
		text_table[i].object_code = (char *)calloc(20, sizeof(char));
		str2 = (char *)calloc(20, sizeof(char));
		if (!strcmp(token_table[i]->operator,"LTORG")|| !strcmp(token_table[i]->operator,"END")) {
			text_table[i].section_number = lit_table[j].which_section;
			text_table[i].location = lit_table[j].addr;
			for (int k = 3; k <= strlen(lit_table[j].literal) - 2; k++) {
				str2[k - 3] = lit_table[j].literal[k];
			}
			if (lit_table[j].literal[1] == 'C') {
				for (int k = 0, m = 0; k < strlen(str2); k++) {
					text_table[i].object_code[m+1] = str2[k] & num;
					if (((str2[k] & num) >= 0) && ((str2[k] & num) <= 9)) 
						text_table[i].object_code[m+1] += '0';
					else
						text_table[i].object_code[m + 1] += 55;
					str2[k] = str2[k] >> 4;

					text_table[i].object_code[m] = str2[k] & num;
					if (((str2[k] & num) >= 0) && ((str2[k] & num) <= 9)) 
						text_table[i].object_code[m] += '0';
					else
						text_table[i].object_code[m] += 55;
					m += 2;
				}
			}
			else
				strcpy(text_table[i].object_code, str2);
			str2 = NULL;
			j++;
		}
		else {
		text_table[i].section_number = token_table[i]->which_section;
		text_table[i].location = token_table[i]->itsloc;
		strcpy(text_table[i].object_code, token_table[i]->obcode);
		}
	}
	for (int i = 0; i <= section_num; i++) {
		int count = 0;
		int start = 0;
		int end = 0;

		/*Header ���*/
		fprintf(file, "H%s\t%06X%06X\n", section_table[i].sect_name, 0, section_table[i].sect_length);

		/*Define record ���*/
		for (int j = 0; j < line_num; j++) {
			if ((!strcmp(token_table[j]->operator,"EXTDEF")) && (token_table[j]->which_section == i)) {
				fprintf(file, "D");
				ptr[0] = strtok(token_table[j]->operand[0], ",");
				for (int k = 1; k < 10; k++) {
					fprintf(file, "%s", ptr[k - 1]);
					for (int m = 0; m < symbol_line; m++) {
						if (!strcmp(ptr[k - 1], sym_table[m].symbol))
							fprintf(file, "%06X", sym_table[m].addr);
					}
					ptr[k] = strtok(NULL, ",");
					if (ptr[k] == NULL)
						break;
				}
				fprintf(file, "\n");
			}
		}

		/*Refer record ���*/
		fprintf(file, "R");
		for (int j = 0; j < line_num; j++) {
			if ((!strcmp(token_table[j]->operator,"EXTREF")) && (token_table[j]->which_section == i)) {
				for (int k = 0; k <= strlen(token_table[j]->operand[0]); k++) {
					if (token_table[j]->operand[0][k] != ',')
						fprintf(file, "%c", token_table[j]->operand[0][k]);
					else
						fprintf(file, "\t");
				}
			}
		}
		fprintf(file,"\n");

		/*Text ���*/
		for (int j = 0; j < line_num; j++) {
			if (text_table[j].section_number == i) {
				if (count == 0) {
					fprintf(file, "T%06X", text_table[j].location);
					count = 1;
					length = 0;
					start = j;
				}
				length += strlen(text_table[j].object_code);
				if ((text_table[j + 1].object_code == NULL) || ((length + strlen(text_table[j + 1].object_code)) >= 60) || text_table[j].section_number != text_table[j+1].section_number) {
					fprintf(file, "%02X", length / 2);
					for (int k = start; k <= j; k++) {
						if ((text_table[k].object_code != NULL))
							fprintf(file, "%s", text_table[k].object_code);
						if (k == j)
							fprintf(file, "\n");
					}
					count = 0;
				}
			}
		}

		/*Modification record ���*/
		for (int j = 0; j < line_num; j++) {
			if ((!strcmp(token_table[j]->operator,"WORD") || (token_table[j]->operator[0] == '+')) && (token_table[j]->which_section == i)) {
				if (token_table[j]->operator[0] == '+') {
					fprintf(file, "M%06X05+%s\n", token_table[j]->itsloc + 1, token_table[j]->operand[0]);
				}
				else if (!strcmp(token_table[j]->operator,"WORD")) {
					strcpy(str, token_table[j]->operand[0]);
					ptr[0] = strtok(str, "-");
					ptr[1] = strtok(NULL, "");
					fprintf(file, "M%06X06+%s\n", token_table[j]->itsloc, ptr[0]);
					fprintf(file, "M%06X06-%s\n", token_table[j]->itsloc, ptr[1]);
				}
			}
		}
		/*End ���*/
		fprintf(file, "E\n\n");
	}
}
