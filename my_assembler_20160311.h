#pragma once
/*
* my_assembler 함수를 위한 변수 선언 및 매크로를 담고 있는 헤더 파일이다.
*
*/
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

/*
* instruction 목록 파일로 부터 정보를 받아와서 생성하는 구조체 변수이다.
* 구조는 각자의 instruction set의 양식에 맞춰 직접 구현하되
* 라인 별로 하나의 instruction을 저장한다.
*/
struct inst_unit {
	char *mnemo;
	char *format;
	char *opcode;
	int opcount;
};
typedef struct inst_unit inst;
inst *inst_table[MAX_INST];
int inst_index;

/*
* 어셈블리 할 소스코드를 입력받는 테이블이다. 라인 단위로 관리할 수 있다.
*/
char *input_data[MAX_LINES];
static int line_num;

int label_num;

/*
* 어셈블리 할 소스코드를 토큰단위로 관리하기 위한 구조체 변수이다.
* operator는 renaming을 허용한다.
* nixbpe는 8bit 중 하위 6개의 bit를 이용하여 n,i,x,b,p,e를 표시한다.
*/
struct token_unit {
	char *label;
	char *operator;
	char *operand[MAX_OPERAND];
	char *comment;
	char nixbpe;
	int itsloc;//locctr 값 저장
	int isOperator;
	char * obcode;
	int which_section; // 섹션 번호 저장
};

typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line; // token_table에서 몇번째 line인지 알려주는 변수

/*control section을 관리하는 구조체이다.
 *섹션 구조체는 섹션 이름, 섹션의 길이로 구성된다.
 */
struct section_unit {
	char sect_name[10];
	int sect_length;
};

typedef struct section_unit section;
section section_table[5];
static int section_num;

/*
* 심볼을 관리하는 구조체이다.
* 심볼 테이블은 심볼 이름, 심볼의 위치로 구성된다.
*/
struct symbol_unit {
	char symbol[10];
	int addr;
	int sect_num;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];
static int symbol_line;

/*리터럴을 관리하는 구조체이다.*/
struct literal_unit {
	char literal[20];//리터럴 내용
	int addr;//리터럴의 주소값
	int which_section;//리터럴이 속하는 section 번호
};

typedef struct literal_unit literal;
literal lit_table[MAX_LINES];
static int literal_line;

static int locctr;

/*텍스트 레코드를 출력하기위해 필요한 구조체이다.*/
struct text_unit {
	int section_number; // 오브젝트 코드가 속하는 section 번호
	int location; // 오브젝트 코드의 주소값
	char *object_code; //오브젝트 코드
	//int used; //해당 라인이 사용되었는지 판단하기위한 변수 
};

typedef struct text_unit text;
text text_table[MAX_LINES];
static int text_line;

//-------------

static char *input_file;
static char *output_file;
int init_my_assembler(void);
int init_inst_file(char *inst_file);
int init_input_file(char *input_file);
int token_parsing(char *str);
int search_opcode(char *str);
static int assem_pass1(void);
void make_opcode_output(char *file_name);
void assign_memory_loc(); 
void make_obcode(int, int);
void make_symtab_output(char *filename);
static int assem_pass2(void);
void make_objectcode_output(char *file_name);