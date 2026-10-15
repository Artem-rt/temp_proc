// assembler - simple version

// useful libraries--------------
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "stack.h"
#include "commands.h"
// ------------------------------

// macro-------------------------
#define USER_COMAND_FILE "calc.txt"
#define ERROR_FILE "ERRORS_AS.log"
#define EXECUTABLE_FILE "EXECUTE.txt"
#define OPEN_FILE_FOR_REWRITING "a"
#define MIN_BUFFER_SIZE 1000
#define MAX_COMMAND_LEN 20
#define MIN_CODE_LEN 20
#define LABEL_CNT 20
// ------------------------------

// functions---------------------
void label_init (int* label);
void assembly (char* buffer_commands, long long* len_buffer_commands, TYPE_OF_STACK_ELEM* code, int* label, FILE* error_file);
void PUSH_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem);
void POP_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem);
void JMP_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem, FILE* error_file, int* label);
void null_now_command (char* now_command, int* len_now_command);
PROG_CODE decode_command (char* now_command);
void destroy_variables (FILE* error_file, FILE* execute_file, char* buffer_commands, int user_comand_file, TYPE_OF_STACK_ELEM* code);
// ------------------------------

// main--------------------------
int main ()
{
    // variables && etc.---------
    FILE* error_file = NULL;
    FILE* execute_file = NULL;
    int user_comand_file = 0;

    struct stat file_size = { .st_size = 0 } ;
    long long len_buffer_commands = 0;
    // char* buffer_commands = NULL;
    char buffer_commands [1000] = { } ;

    TYPE_OF_STACK_ELEM* code = NULL;
    int label [LABEL_CNT] = { } ;
    // --------------------------

    // working area--------------
    if ((error_file = fopen (ERROR_FILE, OPEN_FILE_FOR_REWRITING)) == NULL)
    {
        error_file = stderr;
        printf (ERROR_FILE " wasn't open -> errors'll printf to stderr");
        fprintf (error_file, ERROR_FILE " wasn't open, sorry.\nfile: %s, func: main.cpp: %d\ndate: %s time: %s\n",
                                                                         __FILE__, __LINE__, __DATE__, __TIME__);
    }

    if ((user_comand_file = open (USER_COMAND_FILE, O_RDONLY)) == 0)
    {
        fprintf (error_file, USER_COMAND_FILE " wasn't open, sorry.\nfile: %s, func: main.cpp: %d\ndate: %s time: %s\n",
                                                                         __FILE__, __LINE__, __DATE__, __TIME__);
        abort();
    }

    if ((execute_file = fopen(EXECUTABLE_FILE, OPEN_FILE_FOR_WRITING)) == 0)
    {
        fprintf (error_file, EXECUTABLE_FILE " wasn't open -> programme stoped, sorry...\nfile: %s, func: main.cpp: %d\n", __FILE__, __LINE__);
        abort();
    }

    // if ((buffer_commands = (char*)calloc(MIN_BUFFER_SIZE, sizeof(char))) == NULL)
    // {
    //     fprintf (error_file, "buffer for commands didn't create, sorry...\nfile: %s, func: main.cpp: %d\n", __FILE__, __LINE__);
    //     abort ();
    // }

    stat (USER_COMAND_FILE, &file_size);

    if ((len_buffer_commands = read (user_comand_file, buffer_commands, file_size.st_size)) == 0)
    {
        fprintf (error_file, "commands array didn't create, sorry...\nfile: %s, func: main.cpp: %d\n", __FILE__, __LINE__);
        abort ();
    }

    buffer_commands [len_buffer_commands - 1] = '\0';

    if ((code = (TYPE_OF_STACK_ELEM*)calloc (MIN_CODE_LEN, sizeof(TYPE_OF_STACK_ELEM))) == NULL)
    {
        fprintf (error_file, "code_array didn't create, sorry...\nfile: %s, func: main.cpp: %d\n", __FILE__, __LINE__);
        abort ();
    }

    label_init (label);

    assembly (buffer_commands, &len_buffer_commands, code, label, error_file);

    destroy_variables (error_file, execute_file, buffer_commands, user_comand_file, code);
    // --------------------------

}
// ------------------------------

// functions---------------------
void label_init (int* label)
{
    assert (label);

    for (int i = 0; i < LABEL_CNT; i ++)
    {
        label [i] = -100;
    }

}

void assembly (char* buffer_commands, long long* len_buffer_commands, TYPE_OF_STACK_ELEM* code, int* label, FILE* error_file)
{
    assert (buffer_commands);
    assert (code);
    assert (label);

    // variables && etc.---------
    // long long len_code = 0;
    // long long len_realloc_code = MIN_CODE_LEN;

    char now_command [MAX_COMMAND_LEN] = {0} ;
    int len_now_command = 0;

    int code_elem = 0;
    // --------------------------

    // working area--------------
    for (long long i = 0; i <= *len_buffer_commands; i ++)
    {
        if (buffer_commands [i] == ';')
        {
            while (buffer_commands [i] != '\n')
                i ++ ;
            continue;
        }

        if (buffer_commands [i] == ':')
        {
            int temp_label = 0;
            i ++ ;

            if (!isdigit(buffer_commands [i]))
            {
                fprintf (error_file, "SORRY, BUT YOU USE NO_EXISTENT LABEL (must 0 < label < %d)\nREAD documentation, plz :)\n", LABEL_CNT);
                fprintf (error_file, "YOU LABEL START WITH: %c\n", buffer_commands [i]);
                exit (-1);
            }

            while (buffer_commands [i] != '\n')
            {
                temp_label = temp_label * 10 + (buffer_commands[i] - '0');
                i ++ ;
            }

            label [temp_label] = temp_label;
            i --;
            continue;
        }

        if (isalpha (buffer_commands [i]))
        {
            now_command [len_now_command] = buffer_commands [i];
            len_now_command ++ ;
            continue;
        }

        if (buffer_commands [i] == ' ')
        {

            if (!strcmp (now_command, "PUSH"))
            {
                PUSH_decode (buffer_commands, &i, now_command, &len_now_command, code, &code_elem);
                continue ;
            }

            if (!strcmp (now_command, "POP"))
            {
                POP_decode (buffer_commands, &i, now_command, &len_now_command, code, &code_elem);
                continue ;
            }

            if (!strcmp (now_command, "JMP"))
            {
                JMP_decode (buffer_commands, &i, now_command, &len_now_command, code, &code_elem, error_file, label);
                continue;
            }

            fprintf (error_file, "SORRY, extra space after [now_command]: %s\nIt killed me :(\n", now_command);
            exit (-1);
        }

        if (buffer_commands [i] == '\n')
        {
            PROG_CODE temp_code = NO;
            if ((temp_code = decode_command (now_command)) == NO )
            {
                printf (VIOLET_START "WARNING! (check " ERROR_FILE ")\n" COLOR_STOP);
                fprintf (error_file, "WARNING!!! command is NO\n");
            }

            code [code_elem] = (TYPE_OF_STACK_ELEM)temp_code;
            code_elem ++ ;
            null_now_command (now_command, &len_now_command);
            continue;
        }

        if (buffer_commands [i] == '\0')
        {
            PROG_CODE temp_code = NO;
            if ((temp_code = decode_command (now_command)) == NO )
            {
                printf (VIOLET_START "WARNING! (check " ERROR_FILE ")\n" COLOR_STOP);
                fprintf (error_file, "WARNING!!! command is NO\n");
            }

            code [code_elem] = (TYPE_OF_STACK_ELEM)temp_code;
            code_elem ++ ;
            null_now_command (now_command, &len_now_command);
            break;
        }
    }
    // --------------------------
}

void PUSH_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem)
{
    assert (buffer_commands);
    assert (i);
    assert (now_command);
    assert (code);
    assert (code_elem);

    char minus ='\0';
    int temp_arg = 0;

    (*i) ++ ;

    if (buffer_commands [*i] == '-')
    {
        minus = buffer_commands [*i];
        (*i) ++ ;
    }

    while (buffer_commands [*i] != '\n')
    {
        temp_arg = 10 * temp_arg + (buffer_commands [*i] - '0');
        (*i) ++ ;
    }

    if (minus == '-')
    {
        temp_arg = (-1) * temp_arg;
    }

    code [*code_elem] = (TYPE_OF_STACK_ELEM)PUSH;
    (*code_elem) ++ ;
    code [*code_elem] = temp_arg;
    (*code_elem) ++ ;

    null_now_command (now_command, len_now_command);
}

void POP_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem)
{
    assert (buffer_commands);
    assert (i);
    assert (now_command);
    assert (code);
    assert (code_elem);

    char minus ='\0';
    int temp_arg = 0;

    (*i) ++ ;

    if (buffer_commands [*i] == '-')
    {
        minus = buffer_commands [*i];
        (*i) ++ ;
    }

    while (buffer_commands [*i] != '\n')
    {
        temp_arg = 10 * temp_arg + (buffer_commands [*i] - '0');
        (*i) ++ ;
    }

    if (minus == '-')
    {
        temp_arg = (-1) * temp_arg;
    }

    code [*code_elem] = (TYPE_OF_STACK_ELEM)POP;
    (*code_elem) ++ ;
    code [*code_elem] = temp_arg;
    (*code_elem) ++ ;

    null_now_command (now_command, len_now_command);
}

void JMP_decode (char *buffer_commands, long long* i, char* now_command, int* len_now_command, TYPE_OF_STACK_ELEM* code, int* code_elem, FILE* error_file, int* label)
{
    assert (buffer_commands);
    assert (i);
    assert (now_command);
    assert (code);
    assert (code_elem);

    int temp_arg = 0;

    (*i) ++ ;

    code [*code_elem] = (TYPE_OF_STACK_ELEM)JMP;
    (*code_elem) ++ ;

    if (buffer_commands [*i] == ':')
    {
        (*i) ++ ;
        while (buffer_commands [*i] != '\n')
        {
            temp_arg = 10 * temp_arg + (buffer_commands [*i] - '0');
            (*i) ++ ;
        }

        if (temp_arg < 0 || temp_arg > LABEL_CNT)
        {
            fprintf (error_file, "SORRY, but JMP use strange label - :%d", temp_arg);
            exit (-1);
        }

        code [*code_elem] = label [temp_arg];
        (*code_elem) ++ ;

         null_now_command (now_command, len_now_command);
    } else
    {
        while (buffer_commands [*i] != '\n')
        {
            temp_arg = 10 * temp_arg + (buffer_commands [*i] - '0');
            (*i) ++ ;
        }

        code [*code_elem] = temp_arg;
        (*code_elem) ++ ;

        null_now_command (now_command, len_now_command);
    }
}

void null_now_command (char* now_command, int* len_now_command)
{
    assert (now_command);
    assert (len_now_command);

    for (int i = 0; i < *len_now_command; i++)
    {
        now_command [i] = '\0';
    }

    *len_now_command = 0;
}

PROG_CODE decode_command (char* now_command)
{
    if (!strcmp (now_command, "ADD")) return ADD;
    if (!strcmp (now_command, "SUB")) return SUB;
    if (!strcmp (now_command, "MULL")) return MULL;
    if (!strcmp (now_command, "DIV")) return DIV;
    if (!strcmp (now_command, "OUT")) return OUT;
    if (!strcmp (now_command, "HLT")) return HLT;
    if (!strcmp (now_command, "DUMP")) return ADD;

    return NO;
}

void destroy_variables (FILE* error_file, FILE* execute_file, char* buffer_commands, int user_comand_file, TYPE_OF_STACK_ELEM* code)
{
    assert (error_file);
    assert (execute_file);
    assert (buffer_commands);

    fclose (error_file);
    fclose (execute_file);
    close (user_comand_file);
    free (buffer_commands);
    free (code);
}
// ------------------------------

// THE END-----------------------
