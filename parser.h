//
// parser.h
//

#include <stdio.h>
#include "var.h"

void program (FILE *theFile);
void function (FILE *theFile);
void expr (FILE *theFile, var **head);
void sexpr (FILE *theFile, var **head);
