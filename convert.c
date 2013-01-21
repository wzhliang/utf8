#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "utf8.h"

/*
 * converts gb2312 full text file into [pinyin Chinese] mapping 
 */

#define PY_MAX 128 /* Maximum pinyin string length */
#define TY_MAX 1024 /* Maximum tongyi character string length */

typedef struct py_map_tag
{
    char py[PY_MAX];
    char chinese[TY_MAX];
    struct py_map_tag *next;
} py_map_t;

py_map_t *root;
py_map_t *tail;
static int count = 0;

py_map_t *find_map(char *py)
{
    py_map_t *node = root;

    while( strcmp(node->py, py) != 0 )
    {
        node = node->next;
        if ( node == NULL )
        {
            return NULL;
        }
    }

    return node;
}

py_map_t *new_map(char *py)
{
    /*printf("new_map(%s)\n", py);*/

    py_map_t *new = malloc( sizeof(py_map_t) );

    strcpy(new->py, py);
    memset(new->chinese, 0, TY_MAX);

    return new;
}

char *py2digits(char *py)
{
    char *array[8] =
    {
        "abc",
        "def",
        "ghi",
        "jkl",
        "mon",
        "pqrs",
        "tuv",
        "wxyz"
    };
    static char digits[PY_MAX];

    int i, j;

    for( j=0; j<strlen(py); j++)
    {
        for( i=0; i<8; i++)
        {
            if(strchr(array[i], py[j]))
            {
                digits[j] = 2+i+'0';
            }
        }
    }
    digits[j] = 0;

    return digits;
}

void init(void)
{
    root = NULL;
    tail = NULL;
}

void done(void)
{
    py_map_t *node;

    while(root)
    {
        node = root->next;
        free(root);
        root = node;
    }
}


py_map_t *append_char(char *py, char *utf)
{
    py_map_t *node;

    /*printf("append_char(%s, %s)\n", py, utf);*/

    if (root == NULL)
    {
        root = new_map(py);
        strcpy(root->chinese, utf);
        tail = root;

        return root;
    }

    node = find_map(py);
    if (node)
    {
        strcat(node->chinese, utf);
    }
    else
    {
        node = new_map(py);
        strcpy(node->chinese, utf);
        tail->next = node;
        tail = node;
    }
    return node;
}


int is_pinyin(char *str)
{
    while(*str != 0)
    {
        if (! IS_7BITS(*str))
        {
            return 0;
        }
        str ++;
    }

    return 1;
}

int is_utf8(unsigned char *str)
{

    while(*str != 0)
    {
        if ( IS_TRAILING(*str) )
        {
            str++;
            continue;
        }
        if ( IS_12BITS(*str) )
        {
            str++;
            continue;
        }
        if ( IS_16BITS(*str) )
        {
            str++;
            continue;
        }
        if ( IS_21BITS(*str) )
        {
            str++;
            continue;
        }
        if ( IS_26BITS(*str) )
        {
            str++;
            continue;
        }
        if ( IS_31BITS(*str) )
        {
            str++;
            continue;
        }
        return 0;
    }

    count++;
    return 1;
}

int do_stuff(char *buf, unsigned int len)
{
    char *tmp = NULL;
    unsigned int i = 0;
    static char utf[UTF8_MAX_LEN+1];

    tmp = buf;

    while(1)
    {
        while(*tmp != ' ' && *tmp != '\n')
        {
            tmp++;
            i++;
        }
        if (i >= len)
        {
            /*printf("End of buffer.\n");*/
            break;
        }
        *tmp = 0;
        if ( is_pinyin(buf) )
        {
            /* Assuming that the file is in right format and we always have a good utf
             * string*/
            append_char(buf, utf);
        }
        else if ( is_utf8((unsigned char*)buf) )
        {
            strcpy(utf, buf);
        }

        *tmp = 0x20;

        /* Skip any trailing space */
        while(*tmp == ' ' && *tmp != '\n')
        {
            tmp++;
            i++;
        }
        if (i >= len)
        {
            /*printf("End of buffer.\n");*/
            break;
        }
        buf = tmp;
        tmp = buf;
    }

    return 0;
}

void report(void)
{
    py_map_t *node = root;
    while(node)
    {
        printf("%s\t%s\t%s\n", py2digits(node->py), node->py, node->chinese);
        node = node->next;
    }
}

int main(int argc, char **argv)
{
    char *block;
    int fd;
    struct stat st;

    init();

    if ( argc < 2 )
    {
        printf("I need a file name.\n");
    }
    else
    {
        fd = open(argv[1], O_RDONLY);
        fstat(fd, &st);

        block = mmap(NULL, st.st_size, PROT_WRITE, MAP_PRIVATE, fd, 0);
        if ( block == MAP_FAILED )
        {
            printf("Failed to map file. block: %p\n", block);
            exit(1);
        }
        printf("INPUT: <%s>\n", argv[1]);
        do_stuff(block, st.st_size);
        munmap(block, st.st_size);
        close(fd);
    }

    report();
    done();

    return 0;
}
