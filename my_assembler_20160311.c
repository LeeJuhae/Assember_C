/*
* 설  명 : 이 프로그램은 SIC/XE 머신을 위한 간단한 Assembler 프로그램의 메인루틴으로,
* 입력된 파일의 코드 중, 명령어에 해당하는 OPCODE를 찾아 출력한다.
*/

/*
* 프로그램의 헤더를 정의한다.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "my_assembler_20160311.h"

/*token_unit에서 nixbpe를 설정할때 사용할 매크로 상수*/
#define n_flag 32
#define i_flag 16
#define x_flag 8
#define b_flag 4
#define p_flag 2
#define e_flag 1
/* --------------------------------------------------------------------------------
* 설명 : 사용자로 부터 어셈블리 파일을 받아서 명령어의 OPCODE를 찾아 출력한다.
* 매계 : 실행 파일, 어셈블리 파일
* 반환 : 성공 = 0, 실패 = < 0
* 주의 : 현재 어셈블리 프로그램의 리스트 파일을 생성하는 루틴은 만들지 않았다.
*		   또한 중간파일을 생성하지 않는다.
* ---------------------------------------------------------------------------------
*/
int main(int args, char *arg[])
{
	if (init_my_assembler() < 0)
	{
		printf("init_my_assembler : 프로그램 초기화에 실패 했습니다.\n");
		return -1;
	}
	
	if (assem_pass1() < 0) {
		printf("assem_pass1 : 패스 1 과정에서 실패하였습니다.\n");
		return -1;
	}
	//make_opcode_output("output_20160311");
	//make_opcode_output(NULL);

	//make_symtab_output(NULL);
	make_symtab_output("symtab_20160311");
	
	if(assem_pass2() < 0 ){
		printf(" assem_pass2: 패스2 과정에서 실패하였습니다.  \n") ;
		return -1 ;
	}

	make_objectcode_output("output_20160311") ;
	//make_objectcode_output(NULL);
	return 0;
}

/* ----------------------------------------------------------------------------------
* 설명 : 프로그램 초기화를 위한 자료구조 생성 및 파일을 읽는 함수이다.
* 매계 : 없음
* 반환 : 정상종료 = 0 , 에러 발생 = -1
* 주의 : 각각의 명령어 테이블을 내부에 선언하지 않고 관리를 용이하게 하기
*		   위해서 파일 단위로 관리하여 프로그램 초기화를 통해 정보를 읽어 올 수 있도록
*		   구현하였다.
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
* 설명 : 머신을 위한 기계 코드목록 파일을 읽어 기계어 목록 테이블(inst_table)을
*        생성하는 함수이다.
* 매계 : 기계어 목록 파일
* 반환 : 정상종료 = 0 , 에러 < 0
* 주의 : 기계어 목록파일 형식은 자유롭게 구현한다. 예시는 다음과 같다.
*
*	===============================================================================
*		   | 이름 | 형식 | 기계어 코드 | 오퍼랜드의 갯수 | NULL|
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
			
			/*inst data파일의 내용을 inst_table에 입력*/
			fscanf(file,"%s %s %s %d", inst_table[inst_index]->mnemo,inst_table[inst_index]->format,inst_table[inst_index]->opcode, &(inst_table[inst_index]->opcount));
			inst_index++;
		}
	}
	return errno;
}
/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 할 소스코드를 읽어 소스코드 테이블(input_data)를 생성하는 함수이다.
* 매계 : 어셈블리할 소스파일명
* 반환 : 정상종료 = 0 , 에러 < 0
* 주의 : 라인단위로 저장한다.
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
			
			/*input 텍스트 파일에서 한줄씩 input_data에 넣어줌*/
			fgets(input_data[line_num], sizeof(input_data[line_num])*20, file);
			line_num++;
		}
	}
	return errno;
}
/* ----------------------------------------------------------------------------------
* 설명 : 소스 코드를 읽어와 파싱을 원하는 문자열(str)에서 tab의 개수를 기준으로
		 토큰단위로 분석하고 토큰 테이블을 작성하는 함수이다. 
		 str에서 tab을 만날때마다 tabcount를 증가시킨다.
*        패스 1로 부터 호출된다.
* 매개 : 파싱을 원하는 문자열
* 반환 : 정상종료 = 0 , 에러 < 0
* 주의 : my_assembler 프로그램에서는 라인단위로 토큰 및 오브젝트 관리를 하고 있다.
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
		else if (str[i] == 0) { // 파일의 끝부분 일때 for문 탈출
			break;
		}
		else {
			/*tab을 0개 만났을 때  str[i]을 label에 입력*/
			if (tabcount == 0) {
				token_table[token_line]->label[j] = str[i];
				j++;
			}
			/*tab을 1개 만났을 때  str[i]을 operator에 입력*/
			else if (tabcount == 1) {
				token_table[token_line]->operator[j] = str[i];
				token_table[token_line]->isOperator = search_opcode(token_table[token_line]->operator);
				j++;
			}
			/*tab을 2개 만났을 때 str[i]을 operand에 입력*/
			else if (tabcount == 2) {
				//	token_table[token_line]->isOperator = search_opcode(token_table[token_line]->operator);
					/*token의 operator가 명령어 일 경우*/
				if (token_table[token_line]->isOperator >= 0) {

					int inst_opcount = inst_table[token_table[token_line]->isOperator]->opcount; // operator의 operand개수를 뜻함.
					for (int k = 0; k < inst_opcount; k++) { // inst.data에 적혀있는 operator의 operand의 개수만큼 loop
						token_table[token_line]->operand[k] = (char*)calloc(40, sizeof(char));
						for (;; i++, j++) {
							if (str[i] == '\t') {
								tabcount++;
								/*operand의 개수가 2인데 input파일에 적힌 operand가 1개일 경우 ex)CLEAR A , CLEAR X*/
								if (tabcount == 4)
									tabcount--;
								break;
							}
							else if (str[i] == '\n') {
								i--;
								break;
							}
							else if (str[i] == ',') {
								/* operand 개수가 1일때 x레지스터 사용하는 경우 x를 operand[2]에 넣어줌. ex) +LDCH	BUFFER,X */
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
				/*token의 operator가 명령어가 아닐 경우*/
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
						else if (str[i] == 0) { // 파일의 끝을 만났을 때
							i--;
							break;
						}
						token_table[token_line]->operand[0][j] = str[i];
					}
				}
				j = 0;
			}
			/*tab을 3개 만났을 때 str[i]을 comment에 입력*/
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
* 설명 : 입력 문자열이 기계어 코드인지를 검사하는 함수이다.
* 매개 : 토큰 단위로 구분된 문자열
* 반환 : 정상종료 = 기계어 테이블 인덱스, 에러 < 0
* ----------------------------------------------------------------------------------
*/
int search_opcode(char *str)
{
	int res;
	if (str[0] == '+')
		str = str++;
	for (int k = 0; k < inst_index; k++) {
		res = strcmp(str, inst_table[k]->mnemo);
		/*토큰 단위로 구분된 문자열(str)과 inst.data 파일의 명령어들과 비교하여 하나라도 같을때*/
		if (res == 0) {
			return k;
		}
	}
	return -1;
}
/* ----------------------------------------------------------------------------------
* 설명 : 어셈블리 코드를 위한 패스1과정을 수행하는 함수이다.
*		   패스1에서는..
*		   1. 프로그램 소스를 스캔하여 해당하는 토큰단위로 분리하여 프로그램 라인별 토큰
*		   테이블을 생성한다.
*
* 매개 : 없음
* 반환 : 정상 종료 = 0 , 에러 = < 0
* -----------------------------------------------------------------------------------
*/
static int assem_pass1(void)
{
	int errno;
	int x = 0;
	for (int i = 0; i < line_num; i++) {
		/* input_data의 문자열을 한줄씩 입력 받아서 token_parsing()을 호출하여 token_unit에 저장*/
		errno = token_parsing(input_data[i]);
		/*token_parsing()의 리턴값이 -1일때 */
		if (errno < 0)
			return errno;
		/*리터럴 구조체의 which_section과 addr을 모두 -1로 초기화시킴*/
		lit_table[i].which_section = -1;
		lit_table[i].addr = -1;
	}
	for (int i = 0; i < line_num; i++) {
		/*섹션 구조체에 섹션이름과 섹션번호를 저장해줌*/
		if (i == 0)
			strcpy(section_table[section_num].sect_name, token_table[i]->label);
		else if (!strcmp(token_table[i]->operator,"CSECT")) {
			section_num++;
			strcpy(section_table[section_num].sect_name, token_table[i]->label);
		}
		token_table[i]->which_section = section_num;

		/*리터럴 구조체에 리터럴과 어느 섹션의 리터럴인지 적어줌*/
		if ((token_table[i]->operand[0] != NULL) && (token_table[i]->operand[0][0] == '=')) {
			strcpy(lit_table[literal_line].literal, token_table[i]->operand[0]);
			lit_table[literal_line].which_section = token_table[i]->which_section;
			literal_line++;
		}
	}
	for (int i = 0; i < literal_line; i++) {
		for (int j = i+1; j < literal_line; j++) {
			/*리터럴 구조체에 중복되는 내용 있으면 삭제 해줌*/
			if ((!strcmp(lit_table[i].literal, lit_table[j].literal)) && (lit_table[i].which_section == lit_table[j].which_section)) {
				strcpy(lit_table[j].literal,"");
				lit_table[j].addr = -1;
				lit_table[j].which_section = -1;
				x = 1;
			}
		}
	}
	/*리터럴 삭제했을때 literal_line변수값 1 감소시킴*/
	if (x == 1)
		literal_line--;
	
	assign_memory_loc();
	return 0;
}
/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 명령어 옆에 OPCODE가 기록된 표(과제 4번) 이다.
* 매개 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
*        또한 과제 4번에서만 쓰이는 함수이므로 이후의 프로젝트에서는 사용되지 않는다.
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

		/*token의 label이 ""이 아닐 경우*/
		if (strcmp(token_table[i]->label, "") != 0)
			fprintf(file, "%s\t", token_table[i]->label);
		/*token의 label이 ""일 경우*/
		else
			fprintf(file, "\t");

		/*token의 operator가 ""이 아닐 경우*/
		if (strcmp(token_table[i]->operator, "") != 0)
			fprintf(file, "%s\t", token_table[i]->operator);
		/*token의 operator가  ""일 경우*/
		else
			fprintf(file, "\t");

		/*token의 operator가 명령어 일 경우 (기계어 테이블 인덱스 >=0)*/
		if (isOperator >= 0) {

			/*명령어의 operand가 0개일 경우*/
			if (inst_table[isOperator]->opcount == 0)
				fprintf(file, "\t");

			/*명령어의 operand가 1개 이상일 경우*/
			for (int j = 0; j < inst_table[isOperator]->opcount; j++) {

				/*operand가 ""이 아닐경우*/
				if (strcmp(token_table[i]->operand[j], "") != 0) {
					/*operand가 2개 이상일때, ','를 찍어주기 위한 코드 ex) COMPR A,S*/
					if ((0 < j) && (j <= (inst_table[isOperator]->opcount) - 1))
						fprintf(file, ",");

					fprintf(file, "%s", token_table[i]->operand[j]);

					/*token_table[i]->operand[j]가 마지막 operand일때*/
					if (j == inst_table[isOperator]->opcount - 1) {
						/*token_talbe[i]->operand[2]가 존재할 때*/
						if (token_table[i]->operand[2] != NULL) {
							fprintf(file, ",%s", token_table[i]->operand[2]);
						}
						fprintf(file, "\t");
					}
				}
				/*operand가 ""일 경우*/
				else {
					fprintf(file, "\t");
					break;
				}
			}
		}
		/* token의 operator가 명령어가 아닐 경우(기계어 테이블 인덱스 = -1)*/
		else {
			if (token_table[token_line]->operand[0] != NULL)
				fprintf(file, "%s\t", token_table[token_line]->operand[0]);
			else
				fprintf(file, "\t");
		}

		/*token의 operator가 명령어 일 경우 (기계어 테이블 인덱스 >=0)*/
		if (isOperator >= 0) {
			fprintf(file, "%s\n", inst_table[isOperator]->opcode);
		}
		/* token의 operator가 명령어가 아닐 경우(기계어 테이블 인덱스 = -1)*/
		else
			if (i < line_num - 2)
				fprintf(file, "\n");
	}
	fclose(file);
}

/* ----------------------------------------------------------------------------------
* 설명 : 메모리를 할당해주는 함수이다.
* 매개 : 없음
* 반환 : 없음
* 주의 :
* -----------------------------------------------------------------------------------
*/
void assign_memory_loc() {
	char * equ_operator;// EQU 지시어가 나왔을때 operator(+,-,*,/)를 저장하기위해 사용할 char형 포인터
	char * equ_operand[10];//EQU 지시어가 나왔을때 operand를 저장하기위해 사용할 char형 포인터 배열
	int equ_operand_value[10];
	equ_operator = (char *)calloc(9, sizeof(char));
	for(int i = 0 ; i < 10 ; i++)
		equ_operand[i] = (char *)calloc(10, sizeof(char));
	int p = 0;//equ_operand 포인터 배열의 방번호를 지정함.
	section_num = 0;//section num을 0으로 초기화해줌.

	for (int i = 0; i <= line_num; i++) {
		if (i == 0) {
			locctr = 0;
		}
		else if (!strcmp(token_table[i - 1]->operator,"CSECT")) { //CSECT을 만나면 바로 직전라인이 해당하는 섹션의 길이를 섹션구조체에 저장해줌.
			section_table[section_num].sect_length = locctr;
			section_num++;
			for (int j = 0; j < literal_line; j++) {
				/*LTORG 없이 리터럴 사용되는경우 assem_pass1()에서 미리 넣어둔 lit_table의 literal값이 동일한 리터럴에 주소값을 할당해줌.*/
				if ((token_table[i - 2]->which_section == lit_table[j].which_section) && (lit_table[j].addr == -1)) {
					lit_table[j].addr = locctr;
					section_table[lit_table[j].which_section].sect_length = locctr;
				}
			}
			locctr = 0;
			token_table[i-1]->itsloc = locctr;
		}
		/*지시어 END를 만났을때 주소할당*/
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
			if (search_opcode(token_table[i-1]->operator) >= 0) {//operator가 명령어일 경우
				if (!strcmp(inst_table[search_opcode(token_table[i-1]->operator)]->format, "1"))
					locctr++;
				else if (!strcmp(inst_table[search_opcode(token_table[i-1]->operator)]->format, "2"))
					locctr += 2;
				else
					locctr += 3;
				if (token_table[i-1]->operator[0] == '+')//4형식일때
					locctr++;
			}
			else {//operator가 명령어가 아닐 경우
				  /*operator자리에 RESW가 나왔을때*/
				if (!strcmp(token_table[i - 1]->operator,"RESW")) 
					locctr += 3 * atoi(token_table[i - 1]->operand[0]);
				/*operator자리에 RESB가 나왔을때*/
				else if (!strcmp(token_table[i - 1]->operator,"RESB")) 
					locctr += atoi(token_table[i - 1]->operand[0]);
				/*operator자리에 LTORG가 나왔을때*/
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
					/*EQU가 나왔을때 operand가 *일때*/
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
				/*operator자리에 BYTE가 나왔을때*/
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
				/*operator자리에 WORD가 나왔을때*/	
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
	/*symbol table 생성*/
	for(int i = 0 ; i < line_num ; i++)
		if (strcmp(token_table[i]->label, "")&& strcmp(token_table[i]->label, ".")) {
			strcpy(sym_table[symbol_line].symbol, token_table[i]->label);
			sym_table[symbol_line].addr = token_table[i]->itsloc;
			sym_table[symbol_line].sect_num = token_table[i]->which_section;
			symbol_line++;
		}
}
/* ----------------------------------------------------------------------------------
* 설명 : object code의 10진수 형태를 16진수로 만들어주는 함수이다.
* 매개 : object code의 10진수 형태(int int_obcode), 해당하는 line(int line) 
* 반환 : 없음
* 주의 : 
* -----------------------------------------------------------------------------------
*/
void make_obcode(int int_obcode, int line) {
	int i;
	/*4형식일때*/
	if (token_table[line]->operator[0] == '+')
		i =  7;
	/*1,2,3형식 일때*/
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
* 설명 : 어셈블리 코드를 기계어 코드로 바꾸기 위한 패스2 과정을 수행하는 함수이다.
*		   패스 2에서는 프로그램을 기계어로 바꾸는 작업은 라인 단위로 수행된다.
*		   다음과 같은 작업이 수행되어 진다.
*		   1. 실제로 해당 어셈블리 명령어를 기계어로 바꾸는 작업을 수행한다.
* 매계 : 없음
* 반환 : 정상종료 = 0, 에러발생 = < 0
* 주의 :
* -----------------------------------------------------------------------------------
*/
static int assem_pass2(void)
{
	int x, y=0, ta, pc;//x,y : object code생성과정에서 비트연산시  사용할 변수, ta : 타겟주소, pc : pc값 
	char  *str;
	char *str2;
	str = (char *)calloc(3, sizeof(char));
	str2 = (char *)calloc(10, sizeof(char));
	char * word_operator;// EQU 지시어가 나왔을때 operator(+,-,*,/)를 저장하기위해 사용할 char형 포인터
	char * word_operand[10];//EQU 지시어가 나왔을때 operand를 저장하기위해 사용할 char형 포인터 배열

	word_operator = (char *)calloc(9, sizeof(char));
	for (int i = 0; i < 10; i++)
		word_operand[i] = (char *)calloc(10, sizeof(char));
	int p = 0;

	for (int i = 0; i < line_num; i++) {
		token_table[i]->obcode = (char *)calloc(10, sizeof(char));
		/*명령어 일때*/
		if (token_table[i]->isOperator >= 0) {
			x = y = 0;
			ta = pc = 0;
			token_table[i]->nixbpe = 0;
			/*명령어의 opcode를 object code에 넣어줌*/
			for (int j = 0; j < 2; j++) {
				if ((inst_table[token_table[i]->isOperator]->opcode[j] >= '0') && (inst_table[token_table[i]->isOperator]->opcode[j] <= '9'))
					x = (x | (inst_table[token_table[i]->isOperator]->opcode[j] - '0'));
				else
					x = (x | (inst_table[token_table[i]->isOperator]->opcode[j] - 55));
				if (token_table[i]->operator[0] == '+') { //4형식일때
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
			/*명령어가 3 또는 4형식 일때*/
			if (!strcmp(inst_table[token_table[i]->isOperator]->format, "3/4")) {
				/*명령어의 operator가 존재할 때*/
				if (token_table[i]->operand[0]!= NULL){
					/*nixbpe 중 n과 i를 설정함*/
					if (token_table[i]->operand[0][0] == '#')
						token_table[i]->nixbpe |= i_flag;
					else if (token_table[i]->operand[0][0] == '@')
						token_table[i]->nixbpe |= n_flag;
					else {
						token_table[i]->nixbpe |= n_flag;
						token_table[i]->nixbpe |= i_flag;
					}
					/*nixbpe 중 x 설정함*/
					if (inst_table[token_table[i]->isOperator]->opcount == 1) {
						if (token_table[i]->operand[2] != NULL) {
							token_table[i]->nixbpe |= x_flag;
						}
					}
					/*4형식일때, b = 0, p = 0, e = 1로 설정함*/
					if (token_table[i]->operator[0] == '+') {
						token_table[i]->nixbpe |= e_flag;
					}
					/*3형식일때, b와 p값을 설정*/
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

					/*disp 지정*/
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
			/*명령어가 2형식 일때*/
			else if (!strcmp(inst_table[token_table[i]->isOperator]->format, "2")) {
				for (int j = 0; j < 2; j++) {
					if (token_table[i]->operand[j] != NULL) {
						/*레지스터 번호와 비트연산*/
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
		/*명령어가 아닐 때*/
		else {
			/*operator가 BYTE일때*/
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
			/*operator가 WORD일때*/
			else if (!strcmp(token_table[i]->operator,"WORD")) {
				strcpy(token_table[i]->obcode,"000000");
			}
		}
	}
}

/* ----------------------------------------------------------------------------------
* 설명 : 입력된 문자열의 이름을 가진 파일에 프로그램의 결과를 저장하는 함수이다.
*        여기서 출력되는 내용은 object code (프로젝트 1번) 이다.
* 매계 : 생성할 오브젝트 파일명
* 반환 : 없음
* 주의 : 만약 인자로 NULL값이 들어온다면 프로그램의 결과를 표준출력으로 보내어
*        화면에 출력해준다.
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

	/*text_table 구조체 생성*/
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

		/*Header 출력*/
		fprintf(file, "H%s\t%06X%06X\n", section_table[i].sect_name, 0, section_table[i].sect_length);

		/*Define record 출력*/
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

		/*Refer record 출력*/
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

		/*Text 출력*/
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

		/*Modification record 출력*/
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
		/*End 출력*/
		fprintf(file, "E\n\n");
	}
}
