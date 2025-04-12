# Petite Basic
Petite Basic is a simple first-generation basic-like programming language, designed to be as small as possible.
It is written in C17 (may be C11 and C99 standarts also supported), with only stdc libs.

Current version: `Beta 0.1`.
Extension: `.pb`.

Features:
- Simple syntax
- Named labels instead of numeric ones
- Well-documented
- Implemented in one file <500 lines of code
- No mallocs => no memory leaks
- Configurable
- Well commented implementation

## Language

### Data Types and Memory
There are only one data type: integer, which is represented by an 8-bit unsigned integer.
To make arrays, use `peek` and `poke` commands.

Memory is just an array of bytes (usually 256), first `n` (defined by settings) bytes are usually used for variables, the rest is for arrays.

### Label
Labels are used to mark a position in the code.
They are defined with the syntax: `label-name:`.
They are replacing the original numeric labels.
It was made to increase the writing comfort and the readability of the code.

### Commands
Commands (aka statements) have the syntax: `<command> <arguments-in-any-form>`.
Here are default commands:

| Command | Arguments | Example | Description |
| ------- | --------- | ------- | ----------- |
| `if` | `condition-expr: then-label, else-label` | `if num < 5: less, greater` | If condition is true, jump to then-label, else jump to else-label |
| `rem` | `comment` | `rem any thing in your code` | Ignore comment |
| `let` | `variable: value-expr` | `let x: 12 * 5` | Assign value to variable |
| `goto` | `label` | `goto start` | Jump to label |
| `peek` | `variable, address-expr` | `peek x, arr1 + 5` | Get value of variable at address |
| `poke` | `value-expr, address-expr` | `poke 12, arr1 + 5` | Set value of variable at address |
| `read` | `variable` | `read num` | Gets user entered value and assigns it to variable|
| `print` | `special-format-string` | `print result: $num + 1$\n` | Prints a formatted string without newline |

### Expressions
Expressions are used to calculate values.
Here are all the expressions, in order of precedence (from highest to lowest):

| Operator | Group Name |
| -------- | ---------- |
| `123`, `var` | Literals and variables
| `(...)` | Grouping |
| `*`, `/` | Multiplication and division |
| `+`, `-` | Addition and subtraction |
| `=`, `<`, `>` | Comparison |

## C API

### Embedding
To embed the interpreter into your project, just copy `petite-basic.h` file into your project and include it.
In file where you want the implementation of interpreter pasted, add `#define PETITE_BASIC_C` and `#define PETITE_BASIC_COMMANDS_C` before the include.
In other files just include `petite-basic.h`, without definitions.

### API
Simple API, nothing unnecessary.

```c
// constants, change them if you want
#define PB_VAR_NAME_LEN 10      // maximum length of a variable name; 10 + 1(null-terminator) + 1(value) makes pb_var 12 bytes
#define PB_MEMORY_SIZE  256     // if you change it, consider changing peek and poke commands
#define PB_VARS_COUNT   64      // maximum number of variables
#define PB_CODE_LEN     1024    // freely change it to bigger values

// value type
typedef unsigned char pb_value;

// variable type
typedef struct {
  char     name[PB_VAR_NAME_LEN + 1];
  pb_value addr;
} pb_var;

// command type
typedef struct {
  char*  name;
  void (*func)(char*, char);
} pb_cmd;

// variables
pb_value  pb_mem[PB_MEMORY_SIZE];     // all memory used by .pb programs
pb_var    pb_vars[PB_VARS_COUNT];     // all variables with names and addresses
pb_cmd    pb_cmds[];                  // all commands (defined in petite-basic-commands.c section)
char      pb_code[PB_CODE_LEN];       // array with all code
char      pb_pause;                   // used only in modified versions, to stop iterpreting the code

// functions
void     pb_init();                                     // initializes the interpreter, call after pb_code is set
void     pb_exec();                                     // interprets the whole code given
void     pb_line(char* line, char len);                 // interprets a single line of code
pb_value pb_expr(char* expr, char len);                 // interprets an expression
void     pb_goto(char* label, char len);                // moves the program pointer to the label
pb_value pb_get(char* name, char len);                  // gets a value of a variable
void     pb_set(char* name, char len, pb_value value);  // sets a value of a variable
```


### Creating New Commands
Will be explaned on the example of `peek` command.
1. To create new commands, firstly find the `PETITE_BASIC_COMMANDS_C` section.
2. Find the place where all commands are defined (they start with `pb_cmd_` prefix).
3. Then, add a new command to the list (`peek` is already there).
4. Add it to the `pb_cmds` array. It's only the name of the command, and the pointer to the function to call.
5. After the array here are all commands implemented. Lets take a look at the `peek` command implementation with some additional comments.
```c
// getting a value from memory
void pb_cmd_peek(char* args, char len)
{
  // args is a plain text (not null-terminated)
  // len is the length of the text

  // firstly we "split" the arguments by comma
  // sep is the position of the comma
  char sep = strchr(args, ',') - args;

  // then we get the value from memory by address returned by the expression
  // only the pointer to text and length of the text are passed to the function
  pb_value val = pb_mem[pb_expr(args + sep + 1, len - sep - 1)];

  // then we set the value of the variable
  // args and sep select only the name of the variable
  // the val is already the value of the variable
  pb_set(args, sep, val);
}
```
6. You can embed: label names, variable names, expressions and even other commands (and lines) in your command.

### Sample Code
Here is sample code of how to use the API:
```c
#define PETITE_BASIC_C
#define PETITE_BASIC_COMMANDS_C
#include "pb.h"

int main() {
  strcpy(pb_code,
    "read num\n"
    "let num: num * num\n"
    "print Square: $num$");

  pb_init();
  pb_exec();
  system("pause>nul");
}
```