#pragma once
/*
* my_assembler �Լ��� ���� ���� ���� �� ��ũ�θ� ��� �ִ� ��� �����̴�.
*
*/
#define MAX_INST 256
#define MAX_LINES 5000
#define MAX_OPERAND 3

/*
* instruction ��� ���Ϸ� ���� ������ �޾ƿͼ� �����ϴ� ����ü �����̴�.
* ������ ������ instruction set�� ��Ŀ� ���� ���� �����ϵ�
* ���� ���� �ϳ��� instruction�� �����Ѵ�.
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
* ����� �� �ҽ��ڵ带 �Է¹޴� ���̺��̴�. ���� ������ ������ �� �ִ�.
*/
char *input_data[MAX_LINES];
static int line_num;

int label_num;

/*
* ����� �� �ҽ��ڵ带 ��ū������ �����ϱ� ���� ����ü �����̴�.
* operator�� renaming�� ����Ѵ�.
* nixbpe�� 8bit �� ���� 6���� bit�� �̿��Ͽ� n,i,x,b,p,e�� ǥ���Ѵ�.
*/
struct token_unit {
	char *label;
	char *operator;
	char *operand[MAX_OPERAND];
	char *comment;
	char nixbpe;
	int itsloc;//locctr �� ����
	int isOperator;
	char * obcode;
	int which_section; // ���� ��ȣ ����
};

typedef struct token_unit token;
token *token_table[MAX_LINES];
static int token_line; // token_table���� ���° line���� �˷��ִ� ����

/*control section�� �����ϴ� ����ü�̴�.
 *���� ����ü�� ���� �̸�, ������ ���̷� �����ȴ�.
 */
struct section_unit {
	char sect_name[10];
	int sect_length;
};

typedef struct section_unit section;
section section_table[5];
static int section_num;

/*
* �ɺ��� �����ϴ� ����ü�̴�.
* �ɺ� ���̺��� �ɺ� �̸�, �ɺ��� ��ġ�� �����ȴ�.
*/
struct symbol_unit {
	char symbol[10];
	int addr;
	int sect_num;
};

typedef struct symbol_unit symbol;
symbol sym_table[MAX_LINES];
static int symbol_line;

/*���ͷ��� �����ϴ� ����ü�̴�.*/
struct literal_unit {
	char literal[20];//���ͷ� ����
	int addr;//���ͷ��� �ּҰ�
	int which_section;//���ͷ��� ���ϴ� section ��ȣ
};

typedef struct literal_unit literal;
literal lit_table[MAX_LINES];
static int literal_line;

static int locctr;

/*�ؽ�Ʈ ���ڵ带 ����ϱ����� �ʿ��� ����ü�̴�.*/
struct text_unit {
	int section_number; // ������Ʈ �ڵ尡 ���ϴ� section ��ȣ
	int location; // ������Ʈ �ڵ��� �ּҰ�
	char *object_code; //������Ʈ �ڵ�
	//int used; //�ش� ������ ���Ǿ����� �Ǵ��ϱ����� ���� 
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