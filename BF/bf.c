/*bf.c -- simple BrainF**k interpreter*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TEXT_SIZE 0x100000
#define DATA_SIZE 0x400000
#define OFF 0

uint8_t bf_interpret(uint8_t *text, uint32_t text_size,
                     uint8_t *data, uint32_t data_size)
{
    uint8_t *pc = text;
    uint32_t d_index = data_size / 2;
    int ljmp_level = 0;

    while (pc < (text + text_size))
    {
        switch (*pc)
        {
        case '+': data[d_index]++; break;
        case '-': data[d_index]--; break;
        case '<': d_index = (d_index - 1) % data_size; break;
        case '>': d_index = (d_index + 1) % data_size; break;
        case '[':
            if (!data[d_index])
                do {
                    switch (*pc)
                    {
                    case '[': ljmp_level++; break;
                    case ']': ljmp_level--; break;
                    default: break;
                    }
                    pc++;
                } while (ljmp_level);
            break;
        case ']':
            if (data[d_index])
                do {
                    switch (*pc)
                    {
                    case ']': ljmp_level++; break;
                    case '[': ljmp_level--; break;
                    default: break;
                    }
                    pc--;
                } while (ljmp_level);
            break;
        case ',': data[d_index] = getchar(); break;
        case '.': putchar(data[d_index]); break;
        default: break;
        }
        pc++;
    }
    return data[d_index];
}

int main(int argc, char *argv[])
{
    FILE *code = stdin;
    if (argc > 2)
    {
        puts("Sorry, this interpreter is too simple to analyze multiple arguments.");
        return -1;
    }
    else if (argc == 2)
    {
        if ((!strcmp(argv[1], "-h")) || (!strcmp(argv[1], "--help")))
        {
            puts("A simple BrainF**k interpreter.\n");
            printf("\033[01mUsage:\033[0m %s [code_file]\n\n", argv[0]);
            puts("Notice: if this program is started without arguments,"
                 " you should get the code from stdin.\n"
                 "Note that you cannot use ',' token to input when getting code from stdin.");
            return 0;
        }
        else
        {
            code = fopen(argv[1], "r");
            if (code == NULL)
            {
                puts("ERROR: No such file or directory.");
                return -2;
            }
        }
    }

    uint8_t *text = (uint8_t *)malloc(sizeof(uint8_t) * TEXT_SIZE);
    uint8_t *data = (uint8_t *)calloc(DATA_SIZE, sizeof(uint8_t));
    uint32_t text_size = 0, buff;
    
    while ((buff = fgetc(code)) != EOF)
    {
        text[text_size] = (uint8_t)buff;
        text_size++;
    }
    bf_interpret(text, text_size, data, DATA_SIZE);

    free(data);
    free(text);
    return 0;
}
