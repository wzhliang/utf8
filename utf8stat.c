#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* ==================================================

Bits	Last code point	Byte 1	Byte 2	Byte 3	Byte 4	Byte 5	Byte 6
 7	U+007F	        0xxxxxxx
11	U+07FF	        110xxxxx	10xxxxxx
16	U+FFFF	        1110xxxx	10xxxxxx	10xxxxxx
21	U+1FFFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
26	U+3FFFFFF	111110xx	10xxxxxx	10xxxxxx	10xxxxxx	10xxxxxx
31	U+7FFFFFFF	1111110x	10xxxxxx	10xxxxxx	10xxxxxx	10xxxxxx	10xxxxxx
================================================ */

#define TRACE(c) printf("%d, %c\n", (c), (c))

void print_wchar(unsigned char *start, int len);

typedef struct utfstat_tag
{
    unsigned int _7bits;
    unsigned int _12bits;
    unsigned int _16bits;
    unsigned int _21bits;
    unsigned int _26bits;
    unsigned int _31bits;
} u8stats_t;

static u8stats_t stats;

inline void inc_7bit(void) 
{
    stats._7bits ++;
}

inline void inc_12bit(void) 
{
    stats._12bits ++;
}

inline void inc_16bit(void) 
{
    stats._16bits ++;
}

inline void inc_21bit(void) 
{
    stats._21bits ++;
}

inline void inc_26bit(void) 
{
    stats._26bits ++;
}

inline void inc_31bit(void) 
{
    stats._31bits ++;
}


/* Handlers:
 * return value:
 * > 0   -    handled and the number of bytes it consumed
 *   0   -    not my type
 *  -1   -    my type but something wrong with the coding
 */
inline int IS_TRAILING(unsigned char c) { return ( c >> 6 == 0x2); }
inline int IS_7BITS(unsigned char c)   { return ( c < 127 ); }
inline int IS_12BITS(unsigned char c)   { return ( c >> 5 == 0x6); }
inline int IS_16BITS(unsigned char c)   { return ( c >> 4 == 0xE); }
inline int IS_21BITS(unsigned char c)   { return ( c >> 3 == 0x1E); }
inline int IS_26BITS(unsigned char c)   { return ( c >> 2 == 0x3E); }
inline int IS_31BITS(unsigned char c)   { return ( c >> 1 == 0xEE); }

typedef int (*TEST)(unsigned char c);
typedef void (*ACTION)(void);

/* Actual function that checks a UTF-8 byte sequence and do something
start: 1st byte of the sequence
test: function that check if the first byte is valid
act: action taken when the sequence is fully validated
len: length of the expected sequence
*/
static int _try_real(unsigned char *start, TEST test, ACTION act, int len)
{
    int i;
    int ret = 0;

    /* Test first byte */
    if ( test(*start) )
    {
	/* Test sanity for the remaining bytes */
	for ( i = 1; i < len; i++ )
	{
	    if ( ! IS_TRAILING( *(start+i) ) )
	    {
		ret = -1;
		break;
	    }
	}

	if ( i == len )
	{
	    act();
	    return len;
	}
	else
	{
	    printf("You file doesn't seem to be corrected encoded in UTF-8.\n");
	    return -1;
	}
    }
    else
    {
	return 0;
    }

}

static int try_7bits(unsigned char *start)
{
    return _try_real(start, IS_7BITS, inc_7bit, 1);
}

static int try_12bits(unsigned char *start)
{
    return _try_real(start, IS_12BITS, inc_12bit, 2);
}

static int try_16bits(unsigned char *start)
{
    return _try_real(start, IS_16BITS, inc_16bit, 3);
}

static int try_21bits(unsigned char *start)
{
	int ret = _try_real(start, IS_21BITS, inc_21bit, 4);
	if ( ret > 0 )
	{
		print_wchar(start, 4);
	}

	return ret;
}

static int try_26bits(unsigned char *start)
{
    return _try_real(start, IS_26BITS, inc_26bit, 5);
}

static int try_31bits(unsigned char *start)
{
    return _try_real(start, IS_31BITS, inc_31bit, 6);
}

typedef int (*code_handler)(unsigned char *start);

code_handler handlers[] = 
{
    try_7bits,
    try_12bits,
    try_16bits,
    try_21bits,
    try_26bits,
    try_31bits
};

int analyse(unsigned char *buffer, int len)
{
    int i = 0;
    int ret = 0;
    unsigned char *tmp;

    tmp = buffer;

    while( 1 )
    {
        for (i = 0; i < sizeof(handlers); i ++ )
        {
            ret = handlers[i](tmp);
            if ( ret > 0 )
            {
                tmp += ret;
                break;
            }
            else if ( ret == 0 )
            {
                continue;
            }
            else
            {
                printf("Something wrong with file encoding format.\n");
                return -1;
            }
        }
        if ( tmp - buffer > len )
        {
            return 0;
        }
    }
}

void init(void)
{
    stats._7bits = 0;
    stats._12bits = 0;
    stats._16bits = 0;
    stats._21bits = 0;
    stats._26bits = 0;
    stats._31bits = 0;
}

void report(void)
{
    double total = stats._7bits 
	+ stats._12bits + stats._16bits 
	+ stats._21bits + stats._26bits
	+ stats._31bits;

	printf("\n\n");
    printf(" 7 bits: %07d, %0.2f%%\n", stats._7bits,  100. * stats._7bits/total);
    printf("12 bits: %07d, %0.2f%%\n", stats._12bits, 100. * stats._12bits/total);
    printf("16 bits: %07d, %0.2f%%\n", stats._16bits, 100. * stats._16bits/total);
    printf("21 bits: %07d, %0.2f%%\n", stats._21bits, 100. * stats._21bits/total);
    printf("26 bits: %07d, %0.2f%%\n", stats._26bits, 100. * stats._26bits/total);
    printf("31 bits: %07d, %0.2f%%\n", stats._31bits, 100. * stats._31bits/total);
}

void print_wchar(unsigned char *start, int len)
{
    int i;

    for (i = 0; i<len; i++)
    {
        printf("%c", start[i]);
    }
}

#define TST_STR "你好，我叫梁文智。Wenzhi Liang."
int main(int argc, char **argv)
{
	unsigned char *block;
	int fd;
	struct stat st;

    init();

    if ( argc < 2 )
    {
        printf("INPUT: %s\n\n\n", TST_STR);
        analyse((unsigned char*)TST_STR, strlen(TST_STR));
    }
	else
	{
		fd = open(argv[1], O_RDONLY);
		fstat(fd, &st);

		block = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
		if ( block == MAP_FAILED )
		{
			printf("Failed to map file. block: %p\n", block);
			exit(1);
		}
		printf("INPUT: <%s>\n", argv[1]);
		analyse(block, st.st_size);
		munmap(block, st.st_size);
		close(fd);
	}
					
    report();
    return 0;
}

/* EoF */
