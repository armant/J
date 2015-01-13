#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "var.h"

/* 
 * lookup - finds a variable by name in the list, returns NULL if not found
 */

var *lookup (var *first, char *name) {
  var *cur = first;

  // Iterate down the linked list
  while (cur != NULL) {
    
    // Compare the name using strcmp
    if (strcmp (name, cur->name) == 0) {
      return cur;
    }

    cur = cur->next;
  }

  /* If you didn't find it return NULL */
  return NULL;
}

/* 
 * Creates a new list element and pushes it on the front of the list
 * returns a pointer to the newly created element.
 */

var *push (var *first, char *name, int offset) {
  var *cur, *temp;

  temp = first;
    
  while (temp != NULL) {
    printf("Current temp->name is %s, temp->offset is %d, and name is %s\n",
        temp->name, temp->offset, name);
    
    if (strcmp (temp->name, name) == 0) {
      printf("ERROR: duplicate variable name.");
      exit(1);
    }

    temp = temp->next;
  }

  // Allocate a new list element with malloc
  cur = malloc (sizeof(*cur));

  // If malloc fails end the program
  if (cur == NULL) {
    printf ("Couldn't allocate a new list element\n");
    exit (1);
  }

  // Allocate space to store the name
  cur->name = malloc(strlen(name) + 1);

  // If malloc fails end the program
  if (cur->name == NULL) {
    printf ("Couldn't allocate a new name\n");
    exit (1);
  }

  // Copy the name
  strcpy (cur->name, name);
  cur->offset = offset;
  cur->next = first;
  // return the pointer to the new list_elt
  return cur;
}

/*
 * delete : Deletes an element from the list returns a pointer to the new
 * first element of the list which may just be the old first element.
 */

void clear (var *first) {
  var *cur, *temp;
  cur = first;

  // Iterate down the linked list
  while (cur != NULL) {
    temp = cur;
    cur = cur->next;
    free(temp->name);
    free(temp);
  }

}