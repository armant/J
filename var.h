typedef struct var_tag {
  char *name;
  int offset;  
  struct var_tag *next; // Pointers to the previous and next elements in the list
} var;

var *lookup (var *first, char *name);
var *push (var *first, char *name, int offset);
void clear (var *first);