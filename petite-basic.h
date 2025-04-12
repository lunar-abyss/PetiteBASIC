////////////////////////////////// ABOUT.TXT ///////////////////////////////////
/******************************************************************************\
        ABOUT:   Petite Basic interpreter made in C, with only stdc libs
        MOD:     No mods
        AUTHOR:  Lunaryss, 2025
        LICENSE: Public Domain, no warranty given, use at your own risk
        VERSION: Beta 0.1
\******************************************************************************/



//////////////////////////////// PETITE-BASIC.H ////////////////////////////////
#ifndef PETITE_BASIC_H
#define PETITE_BASIC_H

// constants
#define PB_VAR_NAME_LEN 10
#define PB_MEMORY_SIZE  256
#define PB_VARS_COUNT   64
#define PB_CODE_LEN     4096

// petite basic value
typedef
  unsigned char
  pb_value;

// variable type
typedef
  struct {
    char     name[PB_VAR_NAME_LEN + 1];
    pb_value addr;
  }
  pb_var;

// statement type
typedef struct {
  char*  name;
  void (*func)(char*, char);
} pb_cmd;

// variables
extern pb_value  pb_mem[PB_MEMORY_SIZE];
extern pb_var    pb_vars[PB_VARS_COUNT];
extern pb_cmd    pb_cmds[];
extern char      pb_code[PB_CODE_LEN];
extern char      pb_pause;

// functions
void     pb_init();
void     pb_exec();
void     pb_line(char* line, char len);
pb_value pb_expr(char* expr, char len);
void     pb_goto(char* label, char len);
pb_value pb_get(char* name, char len);
void     pb_set(char* name, char len, pb_value value);

// petite_basic_h
#endif



/////////////////////////// PETITE-BASIC-COMMANDS.H ////////////////////////////
#ifdef PETITE_BASIC_COMMANDS_C

// include libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// all statements
void pb_cmd_if(char*, char);
void pb_cmd_rem(char*, char);
void pb_cmd_let(char*, char);
void pb_cmd_goto(char*, char);
void pb_cmd_peek(char*, char);
void pb_cmd_poke(char*, char);
void pb_cmd_rect(char*, char);
void pb_cmd_read(char*, char);
void pb_cmd_print(char*, char);

// list with all the statements
pb_cmd pb_cmds[] = {
  { "if",    &pb_cmd_if },
  { "rem",   &pb_cmd_rem },
  { "let",   &pb_cmd_let },
  { "goto",  &pb_cmd_goto },
  { "peek",  &pb_cmd_peek },
  { "poke",  &pb_cmd_poke },
  { "read",  &pb_cmd_read },
  { "print", &pb_cmd_print }
};

// setting the size of the pb_cmds
const int pb_cmds_count =
  sizeof(pb_cmds) / sizeof(pb_cmd);

// this command does nothing
void pb_cmd_rem(char* args, char len) { }

// if command
void pb_cmd_if(char* args, char len)
{
  // getting the condition
  char colon = strchr(args, ':') - args;
  char comma = strchr(args + colon, ',') - args;
  char condlen = colon;
  char thenlen = comma - colon - 1;
  char elselen = len - comma - 1;

  // evaluating the condition
  pb_value cond = pb_expr(args, condlen);

  // going to the then or else
  if (cond != 0)
    pb_goto(args + colon + 1, thenlen);
  else
    pb_goto(args + comma + 1, elselen);
}

// defining a variable
void pb_cmd_let(char* args, char len) {
  char sep = strchr(args, ':') - args;
  pb_set(args, sep, pb_expr(args + sep + 1, len - sep - 1));
}

// going to some label
void pb_cmd_goto(char* args, char len) {
  pb_goto(args, len);
}

// getting a value from memory
void pb_cmd_peek(char* args, char len) {
  char sep = strchr(args, ',') - args;
  pb_value val = pb_mem[pb_expr(args + sep + 1, len - sep - 1)];
  pb_set(args, sep, val);
}

// setting a value to memory
void pb_cmd_poke(char* args, char len) {
  char sep = strchr(args, ',') - args;
  pb_mem[pb_expr(args + sep + 1, len - sep - 1)] = pb_expr(args, sep);
}

// readint an integer value from the console
void pb_cmd_read(char* args, char len) {
  int val;
  scanf("%d", &val);
  pb_set(args, len, (pb_value)val);
}

// formatted print to the console
void pb_cmd_print(char* args, char len)
{
  // printing characters and vars
  for (char i = 0; i < len; i++)
  {
    // printing a variable
    if (args[i] == '$') {
      char pos = strchr(args + i + 1, '$') - args;
      printf("%d", pb_expr(args + i + 1, pos - i - 1));
      i = pos;
    }

    // printing an escaped character
    else if (args[i] == '\\') {
      i++;
      if (args[i] == 'n')
        printf("\n");
      else if (args[i] == 't')
        printf("\t");
      else
        printf("%c", args[i]);
    }

    // printing a normal character
    else
      printf("%c", args[i]);
  }
}

// petite_basic_commands_h
#endif



//////////////////////////////// PETITE-BASIC.C ////////////////////////////////
#ifdef PETITE_BASIC_C

// include libs
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// enum of priority levels
enum {
  PB_PRIO_CMP,
  PB_PRIO_ADD,
  PB_PRIO_MUL,
  PB_PRIO_GRP, 
  PB_PRIO_LIT,
};

// importing count of commands
extern const int pb_cmds_count;

// initializing vars
pb_value pb_mem[PB_MEMORY_SIZE];
int      pb_mem_ptr = 0;
pb_var   pb_vars[PB_VARS_COUNT];
char     pb_code[PB_CODE_LEN];
char*    pb_ptr = 0;
char     pb_pause = 0;

// trimming a string
char* pb_trim(char* str, char* len) {
  while (*str == ' ')
    str++, len[0]--;
  while (*(str + *len - 1) == ' ' && *len > 0)
    len[0]--;
  return str;
}

// getting value of the variable
pb_value pb_get(char* name, char len)
{
  // new position of the variable
  int var = -1;

  // do exist
  for (int i = 0; i < PB_VARS_COUNT; i++)
    if (strncmp(name, pb_vars[i].name, len) == 0)
      var = i;

  // return the value
  if (var != -1)
    return pb_mem[pb_vars[var].addr];
}

void pb_set(char* name, char len, pb_value value)
{
  // new position of the variable
  int var = pb_mem_ptr;
  name = pb_trim(name, &len);

  // do already exist
  for (int i = 0; i < PB_VARS_COUNT; i++)
    if (strncmp(name, pb_vars[i].name, len) == 0)
      var = i;
  
    // new variable, allocating and naming
    if (var == pb_mem_ptr) {
      strncpy(pb_vars[var].name, name, len);
      pb_vars[var].addr = pb_mem_ptr;
      pb_mem_ptr++;
    }

    // setting the value
    pb_mem[pb_vars[var].addr] = value;
}

// going to the label
void pb_goto(char* label, char len) {
  label = pb_trim(label, &len);
  for (int i = 0; i < PB_CODE_LEN - len - 1; i++) 
    if (pb_code[i + len] == ':' && strncmp(pb_code + i, label, len) == 0)
      pb_ptr = i + pb_code;
}

// interpreting the expression
pb_value pb_expr(char* expr, char len)
{
  // trimming
  expr = pb_trim(expr, &len);

  // position of the operator
  char oper = 0;
  char prio = PB_PRIO_LIT;

  // scaning for the lowest priority operator
  for (char i = 0; i < len; i++)
  {
    // comparision operators
    if (expr[i] == '=' || expr[i] == '<' || expr[i] == '>')
      oper = i, prio = PB_PRIO_CMP;

    // additive operators
    if ((expr[i] == '+' || expr[i] == '-') && prio >= PB_PRIO_ADD)
      oper = i, prio = PB_PRIO_ADD;
    
    // multiplicative operators
    else if ((expr[i] == '*' || expr[i] == '/') && prio >= PB_PRIO_MUL)
      oper = i, prio = PB_PRIO_MUL;

    // grouping and function calls
    else if (expr[i] == '(' && prio >= PB_PRIO_GRP)
    {
      // setting the values
      oper = i, prio = PB_PRIO_GRP;

      // skipping the parenthesis
      char nesting = 1;
      while (nesting > 0)
        if (expr[++i] == '(')
          nesting++;
        else if (expr[i] == ')')
          nesting--;
    }
  }
  
  // in case of no operator found
  if (prio == PB_PRIO_LIT)
  {
    // the value is the number
    if (expr[0] >= '0' && expr[0] <= '9')
      return atoi(expr);
    
    // the value is variable
    return pb_get(expr, len);
  }

  // operator is binary
  else if (prio >= PB_PRIO_CMP && prio <= PB_PRIO_MUL)
  {
    // computing the operands
    pb_value a = pb_expr(expr, oper);
    pb_value b = pb_expr(expr + oper + 1, len - oper - 1);

    // computing the result
    switch (expr[oper]) {
      case '=': return a == b;
      case '<': return a < b;
      case '>': return a > b;
      case '+': return a + b;
      case '-': return a - b;
      case '*': return a * b;
      case '/': return a / b;
    }
  }

  // operator is grouping
  else
  {
    // it is grouping
    if (oper == 0)
      return pb_expr(expr + 1, len - 2);
  }
}

// interpreting a line
void pb_line(char* line, char len)
{
  // trimming and getting the start position
  line = pb_trim(line, &len);
  char* start = line;

  // skipping if ends with : or empty
  if (line[len - 1] == ':' && line[len - 2] != '\\' || len <= 0)
    return;

  // lowercasing the line
  for (char i = 0; i < len; i++)
    line[i] =
      line[i] >= 'A' && line[i] <= 'Z'
        ? line[i] + 32
        : line[i];

  // length of command and function to call
  char cmdlen =
    strchr(line, ' ') - line < len
      ? strchr(line, ' ') - line
      : len;
  void (*func)(char*, char) = 0;

  // look for the statement
  for (char i = 0; i < pb_cmds_count; i++)
    if (strncmp(line, pb_cmds[i].name, cmdlen) == 0)
      func = pb_cmds[i].func;

  // calling the function
  if (func)
    func(line + cmdlen + 1, len - cmdlen - 1 - (start - line));
}

// initializing the interpreter
void pb_init() {
  pb_ptr = pb_code;
}

// interpret the sequence of commands
void pb_exec()
{
  // beggining of the line
  char* line = pb_ptr;

  // resetting the pause
  pb_pause = 0;

  // looping through all lines
  while (*line != '\0' && !pb_pause)
    if (*pb_ptr == '\n' || *pb_ptr == '\0')
      pb_line(line, pb_ptr - line),
      line = ++pb_ptr;
    else
      pb_ptr++;
}

// petite_basic_c
#endif