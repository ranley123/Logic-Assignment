#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "laws.h"
#include "logic.h"


int apply(struct Expr *expr_tree, int cur_depth, int max_depth, LawSearch searches[],
          LawApplication applies[], int n_laws);
/* 
 * @brief This function is to parse the expression into the struct tree and output
 * - Read lines with expressions from standard input.
 * - Find derivations from each expression.
 * - Use the indicated laws.
 * 
 * @param int max_depth - the max depth (usually 6) and the threshold
 * @param LawSearch searches[] - the array contains all searching methods
 * @param LawApplication applies[] - the array contains all applying methods
 * @param char *names[] - the array contains all names of laws
 * @param int n_laws - the total number of laws
 * 
 * @return void
 */
void find_derivations_for_strings(int max_depth, LawSearch searches[],
                                  LawApplication applies[], char *names[],
                                  int n_laws)
{
  char *line = NULL;
  size_t len = 0;
  while (getline(&line, &len, stdin) != -1)
  {
    int size = strlen(line);
    if (size >= 1 && line[size - 1] == '\n')
      line[size - 1] = '\0';

    struct Expr *expr_tree = read_expr(line); // read expression
    int res = apply(expr_tree, max_depth, max_depth, searches, applies, n_laws);
    printf("%d\n", res);
    free_expr(expr_tree);
  }
}

/**
 * This function is to find the minimum proof steps in an array
 * @brief Function to return a minimum value from the array
 * - iterate over the whole array
 * - if min is still the initial dummy value, then return -1(meaning no successful derivations)
 * 
 * @param int size - the length of the array
 * @param int *deri - the target array storing proofs
 * @parma int max_depth - the threshold
 * 
 * @return int - minimum value inside the array
 */
int min_deri(int size, int *deri, int max_depth)
{
  int min = max_depth + 1; // dummy value
  for (int i = 0; i < size; i++)
  {
    // when there is a valid proof and is less than current min
    if (deri[i] < min && deri[i] != -1) 
      min = deri[i];
  }
  if (min == max_depth + 1) // when there is no successful derivation
    return -1;
  return min;
}

/**
 * This function is the overal apply function and return a shortest proof.
 * @brief Function to apply every law and return the shortest proof.
 * - base case: depends on the cur_depth and if the derivation is successful
 * - find every applicable path and apply laws
 * - store the returned proof values from each law
 * - return a minimum proof
 * 
 * @param struct Expr *expr_tree - the current expression that needs applications
 * @param int cur_depth - the current depth
 * @param int max_depth - the max depth (usually 6) and the threshold
 * @param LawSearch searches[] - the array contains all searching methods
 * @param LawApplication applies[] - the array contains all applying methods
 * @param int n_laws - the total number of laws
 * 
 * @return the shortest proof
 * 
 */
int apply(struct Expr *expr_tree, int cur_depth, int max_depth, LawSearch searches[],
          LawApplication applies[], int n_laws)
{
  if (cur_depth == 0) // when 6 is exceeded.
    return -1;

  if (expr_tree->tag == isTrue) // when the derivation is successful
    return max_depth - cur_depth;

  int *path = non_path(); // initialise the path
  int deri[n_laws]; // to store returned proof values of each law
  for (int i = 0; i < n_laws; i++) // initialise the array
    deri[i] = -1;

  for (int i = 0; i < n_laws; i++)
  {
    struct Expr *cur = copy_expr(expr_tree);
    int *cur_path = searches[i](expr_tree, path);
    while (cur_path != NULL) // keep searching for the next applicable path
    {
      int *temp_path = cur_path; // reserve the old address for next free()
      struct Expr *cur_expr = applies[i](cur, cur_path);
      int temp_res = apply(cur_expr, cur_depth - 1, max_depth, searches, applies, n_laws);

      if (deri[i] == -1) // when there is no valid proof yet, store the current valid proof directly
      {
        if (temp_res != -1)
          deri[i] = temp_res;
      }
      else if (temp_res != -1 && temp_res < deri[i]) // when there is a valid proof, compare and store the smaller one
      {
        deri[i] = temp_res;
      }

      cur_path = searches[i](expr_tree, cur_path);

      free_expr(cur_expr); // free all malloced things
      free(temp_path);
    }
    free(cur_path);
    free_expr(cur);
  }
  free(path);
  return min_deri(n_laws, deri, max_depth); // find the shortest value
}
