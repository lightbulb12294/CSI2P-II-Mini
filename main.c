#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_LENGTH 100

// Token / AST kinds
enum {
	// Non-operators
	LPar, RPar, // parentheses
	Value, Variable,
	// Operators
	/// Precedence 1
	PostInc, PostDec,
	/// Precedence 2
	PreInc, PreDec, Plus, Minus,
	/// Precedence 3
	Mul, Div, Rem,
	/// Precedence 4
	Add, Sub,
	/// Precedence 14
	Assign
};
const char TYPE[20][20] = {
	"Par", "Par",
	"Value", "Variable",
	"PostInc", "PostDec",
	"PreInc", "PreDec", "Plus", "Minus",
	"Mul", "Div", "Rem",
	"Add", "Sub",
	"Assign"
};
typedef struct _TOKEN {
	int kind;
	int param; // Value, Variable, or Parentheses label
	struct _TOKEN *prev, *next;
} Token;
typedef struct _AST {
	int type;
	int val; // Value or Variable
	struct _AST *lhs, *rhs, *mid;
} AST;

// Utility Interface

// Function called when an unexpected expression occurs.
void err();
// Used to create a new Token.
Token *new_token(int kind, int param);
// Used to create a new AST node.
AST *new_AST(Token *mid);
// Convert Token linked list into array form.
int list_to_arr(Token **head);
// Use to check if the kind can be determined as a value section.
int isBinaryOperator(int kind);
// Pass "kind" as parameter. Return true if it is an operator kind.
int isOp(int x);
// Pass "kind" as parameter. Return true if it is an unary kind.
// unary contains increment, decrement, plus, and minus.
int isUnary(int x);
// Pass "kind" as parameter. Return true if it is a parentheses kind.
int isPar(int x);
// Pass "kind" as parameter. Return true if it is a plus or minus.
int isPlusMinus(int x);
// Pass "kind" as parameter. Return true if it is an operand(value or variable).
int isOperand(int x);
// Return the precedence of a kind. If doesn't have precedence, return -1.
int getOpLevel(int kind);
// Given the position of left parthesis, find the right part in range [tar,r]. If not found, return -1.
int findParPair(Token *arr, int tar, int r);
// Parse the expression the the next section. A section means a Value, Variable, or a parenthesis pair.
int nextSection(Token *arr, int l, int r);
// Used to find a appropriate operator that split the expression half.
int find_Tmid(Token *arr, int l, int r);
// Determine the memory location of variable
int var_memory(AST *ast);

// Debug Interface

// Print the AST. You may set the indent as 0.
void AST_print(AST *head, int indent);

// Main Function

char input[MAX_LENGTH];

// Convert the inputted string into multiple tokens.
Token *lexer(char *in);
// Use tokens to build the binary expression tree.
AST *parser(Token *arr, int l, int r);
// Checkif the expression(AST) is legal or not.
void semantic_check(AST *now);
// Generate the ASM.
void codegen(AST *ast);

int main() {
	while(fgets(input, MAX_LENGTH, stdin) != NULL) {
		// build token list by lexer
		Token *content = lexer(input);
		// convert token list into array
		int length = list_to_arr(&content);
		// build abstract syntax tree by parser
		AST *ast_root = parser(content, 0, length-1);
		// check if the syntax is correct
		semantic_check(ast_root);
		// generate the assembly
		codegen(ast_root);
	}
	return 0;
}

Token *lexer(char *in) {
	Token *head = NULL, *tmp = NULL;
	Token **now = &head, *prev = NULL;
	int par_cnt = 0;
	for(int i = 0; in[i]; i++) {
		if(in[i] == ' ' || in[i] == '\n')
			continue;

		else if('x' <= in[i] && in[i] <= 'z')
			(*now) = new_token(Variable, in[i]);

		else if(isdigit(in[i])) {
			int val = 0, oi = i;
			for(; isdigit(in[i]); i++)
				val = val * 10 + (in[i] - '0');
			i--;
			// Detect illegal number inputs such as "01"
			if(oi != i && in[oi] == '0')
				err();
			(*now) = new_token(Value, val);
		}

		else {
			switch(in[i]) {
				case '+':
					if(in[i+1] == '+') { // '++'
						tmp = prev;
						while(tmp != NULL && tmp->kind == RPar) tmp = tmp->prev;
						if(tmp != NULL && tmp->kind == Variable)
							(*now) = new_token(PostInc, 0);
						else (*now) = new_token(PreInc, 0);
						i++;
					}
					else { // '+'
						if(prev != NULL && isBinaryOperator(prev->kind))
							(*now) = new_token(Plus, 0);
						else (*now) = new_token(Add, 0);
					}
					break;
				case '-':
					if(in[i+1] == '-') { // '--'
						tmp = prev;
						while(tmp != NULL && tmp->kind == RPar) tmp = tmp->prev;
						if(tmp != NULL && tmp->kind == Variable)
							(*now) = new_token(PostDec, 0);
						else (*now) = new_token(PreDec, 0);
						i++;
					}
					else { // '-'
						if(prev != NULL && isBinaryOperator(prev->kind))
							(*now) = new_token(Minus, 0);
						else (*now) = new_token(Sub, 0);
					}
					break;
				case '*':
					(*now) = new_token(Mul, 0);
					break;
				case '/':
					(*now) = new_token(Div, 0);
					break;
				case '%':
					(*now) = new_token(Rem, 0);
					break;
				case '(':
					(*now) = new_token(LPar, par_cnt++);
					break;
				case ')':
					(*now) = new_token(RPar, --par_cnt);
					break;
				case '=':
					(*now) = new_token(Assign, 0);
					break;
				default:
					err();
			}
		}
		(*now)->prev = prev;
		if(prev != NULL) prev->next = (*now);
		prev = (*now);
		now = &((*now)->next);
	}
	return head;
}

AST *parser(Token *arr, int l, int r) {
	if(l > r) return NULL;
	// covered in parentheses
	if(r == findParPair(arr, l, r)) {
		AST *res = new_AST(arr+l);
		res->mid = parser(arr, l+1, r-1);
		return res;
	}

	int mid = find_Tmid(arr, l, r);
	AST *newN = new_AST(arr + mid);
	
	if(l == r) {
		if(newN->type <= Variable)
			return newN;
		else err();
	}

	if(getOpLevel(arr[mid].kind) == 1) // a++, a--
		newN->mid = parser(arr, l, mid-1);
	// TODO: Implement the remaining parser part.
	// hint: else if(other op type?) then do something ..etc
	return newN;
}

void semantic_check(AST *now) {
	if(isUnary(now->type) || isPar(now->type)) {
		if(now->lhs != NULL || now->rhs != NULL)
			err();
		if(now->mid == NULL)
			err();
		if(isUnary(now->type)) {
			AST *tmp = now->mid;
			if(isPar(tmp->type)) {
				while(isPar(tmp->type))
					tmp = tmp->mid;
			}
			if(isPlusMinus(now->type)) {
				if(isUnary(tmp->type));
				else if(isOperand(tmp->type));
				else err();
			}
			else if(tmp->type != Variable)
				err();
		}

		semantic_check(now->mid);
	}
	// TODO: Implement the remaining semantic check part.
	// hint: else if(other op type?) then do something ...etc
}

void codegen(AST *ast) {
	// TODO: Implement your own codegen.
	// You may modify the pass parameter(s) or the return type as you wish.
}


void err() {
	puts("Compile Error!");
	exit(0);
}

Token *new_token(int kind, int param) {
	Token *res = (Token*)malloc(sizeof(Token));
	res->kind = kind;
	res->param = param;
	res->prev = res->next = NULL;
	return res;
}

AST* new_AST(Token *mid) {
	AST *newN = (AST*)malloc(sizeof(AST));
	newN->lhs = newN->mid = newN->rhs = NULL;
	newN->type = mid->kind;
	newN->val = mid->param;
	return newN;
}

int list_to_arr(Token **head) {
	int res = 0;
	Token *now = (*head), *t_head = NULL, *del;
	while(now!=NULL) {
		res++;
		now = now->next;
	}
	now = (*head);
	t_head = (Token*)malloc(sizeof(Token)*res);
	for(int i = 0; i < res; i++) {
		t_head[i] = (*now);
		del = now;
		now = now->next;
		free(del);
	}
	(*head) = t_head;
	return res;
}

int isBinaryOperator(int kind) {
	int res = getOpLevel(kind);
	if(res >= 3) return 1;
	return 0;
}

int isOp(int x) {
	return Mul <= x && x <= Assign;
}

int isUnary(int x) {
	return PostInc <= x && x <= Minus;
}

int isPar(int x) {
	return LPar <= x && x<= RPar;
}

int isPlusMinus(int x) {
	if(x == Plus) return 1;
	if(x == Minus) return 1;
	return 0;
}

int isOperand(int x) {
	if(x == Value) return 1;
	if(x == Variable) return 1;
	return 0;
}

int getOpLevel(int kind) {
	int res;
	if(kind <= Variable) res = -1;
	else if(kind <= PostDec) res = 1;
	else if(kind <= Minus) res = 2;
	else if(kind <= Rem) res = 3;
	else if(kind <= Sub) res = 4;
	else if(kind <= Assign) res = 14;
	else res = -1;
	return res;
}

int findParPair(Token *arr, int tar, int r) {
	if(arr[tar].kind != LPar) return -1;
	for(int i = tar + 1; i <= r; i++)
		if(arr[i].kind == RPar)
			if(arr[i].param == arr[tar].param)
				return i;
	return -1;
}

int nextSection(Token *arr, int l, int r) {
	int res = l;
	if(arr[l].kind == LPar) {
		res = findParPair(arr, l, r);
		if(res == -1) err();
	}
	return res + 1;
}

int find_Tmid(Token *arr, int l, int r) {
	int big = l;
	for(int i = l; i <= r;) {
		if(getOpLevel(arr[big].kind) <= getOpLevel(arr[i].kind)) {
			if(isPlusMinus(arr[big].kind) && isPlusMinus(arr[i].kind));
			else if(getOpLevel(arr[big].kind) != 14)
				big = i;
		}
		i = nextSection(arr, i, r);
	}
	return big;
}

int var_memory(AST *ast) {
	while(ast->type != Variable)
		ast = ast->mid;
	
	switch(ast->val) {
		case 'x': return 0;
		case 'y': return 4;
		case 'z': return 8;
		default: 
			err();
			return -1;
	}
}

void AST_print(AST *head, int indent) {
	if(head == NULL) return;
	const char kind_only[] = "<%s>\n";
	const char kind_para[] = "<%s>, <%s = %d>\n";
	for(int i=0;i<indent;i++) printf("  ");
	switch(head->type) {
		case LPar:
		case RPar:
		case PostInc:
		case PostDec:
		case PreInc:
		case PreDec:
		case Plus:
		case Minus:
		case Mul:
		case Div:
		case Rem:
		case Add:
		case Sub:
		case Assign:
			printf(kind_only, TYPE[head->type]);
			break;
		case Value:
			printf(kind_para, TYPE[head->type], "value", head->val);
			break;
		case Variable:
			printf(kind_para, TYPE[head->type], "name", head->val);
			break;
		default:
			puts("Undefined AST Type!");
	}
	AST_print(head->lhs, indent+1);
	AST_print(head->mid, indent+1);
	AST_print(head->rhs, indent+1);
}
