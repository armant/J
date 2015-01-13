//
// parser.c : Implements a recursive descent parser for the J language
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "token.h"
#include "parser.h"


// Variables used to measure s-expression depth
static int s_expr_depth;

// Boolean used to disallow lets in bad places
// If this is non-zero lets will trigger a parse error
static int no_more_lets = 0;

FILE *output;
int let_count;
int label_count = 0;

void sexpr (FILE *theFile, var **head) {
  int arg_no, label_fixer, i;
  char function_name[MAX_LINE_LENGTH];
  tokenType operator;
  
  arg_no = 0;

  expect (theFile, LPAREN);
  ++s_expr_depth;

  switch (current_token.type) {

    // Handle built in operators
    case PLUS:
    case MINUS:
    case MPY:
    case DIV:
    case MOD:
    case GT:
    case LT:
    case GEQ:
    case LEQ:
    case EQ:

      if (s_expr_depth == 1) no_more_lets = 1;
      
      // record the operation type for later use
      operator = current_token.type;

      printf ("Handling a %s operation\n", tokenType_to_string (operator));

      // get the next token
      get_token (theFile);

      // handle the first argument
      expr (theFile, head);
      fprintf(output,
          "    ADD R6, R6, #-1 ;; put the value of the first operand on top of the stack (1/2)\n");
      fprintf(output,
          "    STR R0, R6, #0  ;; put the value of the first operand on top of the stack (2/2)\n");

      // handle the second argument
      expr (theFile, head);
      fprintf(output, "    LDR R1, R6, #0  ;; load first operand to R1\n");
      fprintf(output,
          "    ADD R6, R6, #1  ;; pop first operand off the stack\n");

      if (operator == PLUS) {
        fprintf(output, "    ADD R0, R1, R0  ;; OPERATION + R0 = R1 + R0\n");
      } else if (operator == MINUS) {
        fprintf(output, "    SUB R0, R1, R0  ;; OPERATION - R0 = R1 - R0\n");
      } else if (operator == MPY) {
        fprintf(output, "    MUL R0, R1, R0  ;; OPERATION * R0 = R1 * R0\n");
      } else if (operator == DIV) {
        fprintf(output, "    DIV R0, R1, R0  ;; OPERATION / R0 = R1 / R0\n");
      } else if (operator == MOD) {
        fprintf(output, "    MOD R0, R1, R0  ;; OPERATION %% R0 = R1 %% R0\n");
      } else if (operator == GT) {
        label_count++;
        fprintf(output,
            "    CMP R1, R0      ;; COMPARISON > R0 = R1 > R0 (1/7)\n");
        fprintf(output,
            "    BRp greater%d    ;; COMPARISON > R0 = R1 > R0 (2/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #0    ;; COMPARISON > R0 = R1 > R0 (3/7)\n");
        fprintf(output,
            "    JMP continue%d   ;; COMPARISON > R0 = R1 > R0 (4/7)\n",
            label_count);
        fprintf(output,
            "greater%d            ;; COMPARISON > R0 = R1 > R0 (5/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #1    ;; COMPARISON > R0 = R1 > R0 (6/7)\n");
        fprintf(output,
            "continue%d           ;; COMPARISON > R0 = R1 > R0 (7/7)\n",
            label_count);
      } else if (operator == LT) {
        label_count++;
        fprintf(output,
            "    CMP R1, R0      ;; COMPARISON < R0 = R1 < R0 (1/7)\n");
        fprintf(output,
            "    BRn less%d       ;; COMPARISON < R0 = R1 < R0 (2/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #0    ;; COMPARISON < R0 = R1 < R0 (3/7)\n");
        fprintf(output,
            "    JMP continue%d   ;; COMPARISON < R0 = R1 < R0 (4/7)\n",
            label_count);
        fprintf(output,
            "less%d               ;; COMPARISON < R0 = R1 < R0 (5/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #1    ;; COMPARISON < R0 = R1 < R0 (6/7)\n");
        fprintf(output,
            "continue%d           ;; COMPARISON < R0 = R1 < R0 (7/7)\n",
            label_count);
      } else if (operator == GEQ) {
        label_count++;
        fprintf(output,
            "    CMP R1, R0      ;; COMPARISON >= R0 = R1 >= R0 (1/7)\n");
        fprintf(output,
            "    BRzp greater_or_equal%d ;; COMPARISON >= R0 = R1 >= R0 (2/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #0    ;; COMPARISON >= R0 = R1 >= R0 (3/7)\n");
        fprintf(output,
            "    JMP continue%d   ;; COMPARISON >= R0 = R1 >= R0 (4/7)\n",
            label_count);
        fprintf(output,
            "greater_or_equal%d   ;; COMPARISON >= R0 = R1 >= R0 (5/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #1    ;; COMPARISON >= R0 = R1 >= R0 (6/7)\n");
        fprintf(output,
            "continue%d           ;; COMPARISON >= R0 = R1 >= R0 (7/7)\n",
            label_count);
      } else if (operator == LEQ) {
        label_count++;
        fprintf(output,
            "    CMP R1, R0      ;; COMPARISON <= R0 = R1 <= R0 (1/7)\n");
        fprintf(output,
            "    BRnz less_or_equal%d ;; COMPARISON <= R0 = R1 <= R0 (2/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #0    ;; COMPARISON <= R0 = R1 <= R0 (3/7)\n");
        fprintf(output,
            "    JMP continue%d   ;; COMPARISON <= R0 = R1 <= R0 (4/7)\n",
            label_count);
        fprintf(output,
            "less_or_equal%d      ;; COMPARISON <= R0 = R1 <= R0 (5/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #1    ;; COMPARISON <= R0 = R1 <= R0 (6/7)\n");
        fprintf(output,
            "continue%d           ;; COMPARISON <= R0 = R1 <= R0 (7/7)\n",
            label_count);
      } else if (operator == EQ) {
        label_count++;
        fprintf(output,
            "    CMP R1, R0      ;; COMPARISON == R0 = R1 == R0 (1/7)\n");
        fprintf(output,
            "    BRz equal%d      ;; COMPARISON == R0 = R1 == R0 (2/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #0    ;; COMPARISON == R0 = R1 == R0 (3/7)\n");
        fprintf(output,
            "    JMP continue%d   ;; COMPARISON == R0 = R1 == R0 (4/7)\n",
            label_count);
        fprintf(output,
            "equal%d              ;; COMPARISON == R0 = R1 == R0 (5/7)\n",
            label_count);
        fprintf(output,
            "    CONST R0, #1    ;; COMPARISON == R0 = R1 == R0 (6/7)\n");
        fprintf(output,
            "continue%d           ;; COMPARISON == R0 = R1 == R0 (7/7)\n",
            label_count);
      }

      break;

      // Handle let statements
    case LET:

      if (s_expr_depth != 1) {
	      parse_error ("Let's cannot be nested inside other s-expressions");
      }

      if (no_more_lets) {
	      parse_error ("All let's must occur at the start of the function");
      }

	    expect (theFile, LET);

      if (current_token.type == IDENT) {
	      printf ("Binding %s to a value\n", current_token.str);

        let_count--;
        *head = push(*head, current_token.str, let_count);
        printf("Top of the stack is %s with value %d\n", (*head)->name, 
            (*head)->offset);
        fprintf(output,
            "    ADD R6, R6, #-1 ;; allocating R5%d for the variable %s\n",
            let_count, (*head)->name);
      }

      // get the identifier being bound
      expect (theFile, IDENT);

      // expression value to be associated
      expr (theFile, head);
      fprintf(output,
          "    STR R0, R6, #0  ;; the value of %s is what's stored in R0\n",
          (*head)->name);
      break;

      // Handle if statements
    case IF:

      if (s_expr_depth == 1) no_more_lets = 1;

      expect (theFile, IF);

      printf ("Handling an IF statement \n");

      // get test expr
      expr (theFile, head);
      label_count++;
      label_fixer = label_count;
      fprintf(output, "    CMPI R0, #0     ;; IF %d (1/5)\n", label_fixer);
      fprintf(output, "    BRz if_false%d   ;; IF %d (2/5)\n", label_fixer,
          label_fixer);
      
      // get true clause - to be returned if test is non-zero
      expr (theFile, head);
      fprintf(output, "    JMP continue%d   ;; IF %d (3/5)\n", label_fixer,
          label_fixer);
      
      // get false clause - to be returned if test is zero
      fprintf(output, "if_false%d           ;; IF %d (4/5)\n", label_fixer,
          label_fixer);
      expr (theFile, head);
      
      fprintf(output, "continue%d           ;; IF %d (5/5)\n", label_fixer,
          label_fixer);

      break;

      // Handle function calls
    case IDENT:

      if (s_expr_depth == 1) no_more_lets = 1;

      // Note the function name

      printf ("Handling a function call to %s\n", current_token.str);
      strcpy(function_name, current_token.str);
      expect (theFile, IDENT);
    
      // Handle the arguments
      while ((current_token.type == NUMBER) || (current_token.type == IDENT)  ||
	        (current_token.type == LPAREN)) {

	      //
	      // Handle each function argument and put it on the stack
	      // at the appropriate location
	      //

	      printf ("Argument number %d\n", arg_no);
	      ++arg_no;
        expr (theFile, head);
        fprintf(output,
            "    STR R0, R6, #-1 ;; FUNCTION CALL TO %s: putting the value of R0 on the top of the stack (1/2)\n",
            function_name);
        fprintf(output,
            "    ADD R6, R6, #-1 ;; FUNCTION CALL TO %s: putting the value of R0 on the top of the stack (2/2)\n",
            function_name);
      }

      for (i = 0; i < arg_no / 2; i++) {
        fprintf(output,
            "    LDR R1, R6, #%d  ;; FUNCTION CALL TO %s: swapping the arguments at R6+%d and R6+%d (1/4)\n",
            i, function_name, i, arg_no - 1 - i);
        fprintf(output,
            "    LDR R2, R6, #%d  ;; FUNCTION CALL TO %s: swapping the arguments at R6+%d and R6+%d (2/4)\n",
            arg_no - 1 - i, function_name, i, arg_no - 1 - i);
        fprintf(output,
            "    STR R2, R6, #%d  ;; FUNCTION CALL TO %s: swapping the arguments at R6+%d and R6+%d (3/4)\n",
            i, function_name, i, arg_no - 1 - i);
        fprintf(output,
            "    STR R1, R6, #%d  ;; FUNCTION CALL TO %s: swapping the arguments at R6+%d and R6+%d (4/4)\n",
            arg_no - 1 - i, function_name, i, arg_no - 1 - i);
      }

      fprintf(output, "    JSR %s           ;; FUNCTION CALL TO %s: the call\n",
          function_name, function_name);
      fprintf(output,
          "    LDR R0, R6, #-1 ;; FUNCTION CALL TO %s: save the return value to R0\n",
          function_name);
      fprintf(output,
          "    ADD R6, R6, #%d  ;; FUNCTION CALL TO %s: free %d argument slot(s)\n",
          arg_no, function_name, arg_no);
      break;

    default:
      parse_error ("Bad sexpr");
      break;
  };

  expect (theFile, RPAREN);
  --s_expr_depth;
}

void expr (FILE *theFile, var **head) {
  var *temp;

  switch (current_token.type) {
    case NUMBER:
      
      // Handle a numeric literal
      printf ("Literal value %d\n", current_token.value);

      if (current_token.value < -32768 || current_token.value > 65535) {
        printf("WARNING: integer overflow occurs for %d.\n",
            current_token.value);
      }

      fprintf(output, "    CONST R0, x%x    ;; storing %d in R0 (1/2)\n",
          current_token.value & 0x00FF, current_token.value);
      fprintf(output, "    HICONST R0, x%x  ;; storing %d in R0 (2/2)\n",
          ((current_token.value & 0xFF00) >> 8), current_token.value);

      if (s_expr_depth == 0) no_more_lets = 1;
    
      expect (theFile, NUMBER);
      break;

    case IDENT:

      // Handle an identifier
      printf ("Identifier %s\n", current_token.str);

      if (s_expr_depth == 0) {
        no_more_lets = 1;
      }

      temp = lookup(*head, current_token.str);

      if (temp == NULL) {
        printf("ERROR: an undeclared variable is used.\n");
        exit(1);
      }

      fprintf(output, "    LDR R0, R5, #%d  ;; R0 stores the value of %s\n",
          temp->offset, temp->name);
      expect (theFile, IDENT);
      break;
      
    case LPAREN:
      sexpr (theFile, head);
      break;
      
    default:
      parse_error ("Bad expression");
      break;
  }
}

void function (FILE *theFile) {
  int arg_no = 0;
  var *head, *temp;
  char function_name[MAX_LINE_LENGTH];

  head = NULL;

  expect (theFile, LPAREN);
  expect (theFile, DEFUN);

  s_expr_depth = 0;
  no_more_lets = 0;
  strcpy(function_name, current_token.str);

  if (current_token.type == IDENT) {
    printf ("Parsing a function called %s\n", current_token.str);
    fprintf(output, "\n        .CODE\n        .FALIGN\n%s", current_token.str);
    fprintf(output, "\n    ;; prologue\n");
    fprintf(output, "    ADD R6, R6, #-3 ;; allocate 3 stack slots\n");
    fprintf(output, "    STR R7, R6, #1  ;; save return address\n");
    fprintf(output, "    STR R5, R6, #0  ;; save caller FP\n");
    fprintf(output, "    ADD R5, R6, #0  ;; setup local FP\n");
  }
  
  // Parse the function name
  expect (theFile, IDENT);

  // Parse the argument list
  expect (theFile, LPAREN);

  while (current_token.type == IDENT) {
    
    // Do something with each argument

    printf ("\t argument %d : %s\n", arg_no, current_token.str);
    head = push(head, current_token.str, arg_no + 3);
    ++arg_no;
    
    expect (theFile, IDENT);
  }
  
  expect (theFile, RPAREN);
  let_count = 0;

  do {

    temp = head; // debugging:
    while (temp != NULL) {
      printf("Current var's name is %s and the value is %d\n", 
          temp->name, temp->offset);
      temp = temp->next;
    }

    // The function body consists of at least one expression.
    // the value of the last expression should be returned as the
    // function value.
    expr (theFile, &head);
    
  } while ((current_token.type == NUMBER) ||
	   (current_token.type == IDENT)  ||
	   (current_token.type == LPAREN));

  expect (theFile, RPAREN);

  fprintf(output, "    ;; epilogue\n");
  fprintf(output,
      "    STR R0, R5, #2  ;; write the return value of %s to R5+2\n",
      function_name);
  fprintf(output, "    ADD R6, R5, #0  ;; free space for locals\n");
  fprintf(output, "    LDR R5, R6, #0  ;; restore caller FP\n");
  fprintf(output, "    LDR R7, R6, #1  ;; restore RA\n");
  fprintf(output, "    ADD R6, R6, #3  ;; free space for RA,FP,RV\n");
  fprintf(output, "    RET             ;; return\n");
  clear(head);
}

void program (FILE *theFile)
{
  if (get_token(theFile) != 0) {
    parse_error ("problem reading first token");
  }

  printf ("Parsing a J program\n");
  
  while (current_token.type != END_OF_FILE) {
    function (theFile);
  }

}

int main(int argc, char **argv) {
  FILE *input;
  char *asmFile;
  asmFile = malloc(strlen(argv[1]) + 2);
  memcpy(asmFile, &argv[1][0], strlen(argv[1]) - 1);
  strcat(asmFile, "asm");
  input = fopen(argv[1], "r");
  output = fopen (asmFile, "w");
  program(input);
  free(asmFile);
  fclose(input);
  fclose(output);
}