#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "absyn.h"
#include "escape.h"
#include "errormsg.h"
#include "parser-wrap.h"
#include "prabsyn.h"
#include "semantic.h"
#include "util.h"

int main(int argc, char **argv)
{
    A_exp prog;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s filename\n", argv[0]);
        exit(1);
    }

    /* yydebug = 1; */
    if (!(prog = parse(argv[1])) || EM_anyErrors)
        exit(1);


   /* pr_exp(stdout, prog, 0);*/
    /*printf("\n");*/

    esc_find_escape(prog);


    char* result = semantic_proc(prog);

    
    char output[100];
    strcpy(output, "");

    strcat(output, argv[1]);
    strcat(output, ".ll");

    FILE* fp;
    fp = fopen(output, "w+");
    
    fputs(result, fp);
    fclose(fp);

    return 0;
}
