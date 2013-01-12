#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Basically print out same thing as
 * http://jrgraphix.net/r/Unicode/4E00-9FFF
 */

#define IDEO_MIN 0x4E00
#define IDEO_MAX 0x9FFF
#define UTF8_MAX_LEN 6

/* 
 * xxxx    (xxxx xx)   (xx xxxx)
 */
char *gen_code_point(unsigned int ideo)
{
    static char buffer[UTF8_MAX_LEN];

    memset(buffer, 0, UTF8_MAX_LEN);

    if (ideo < IDEO_MIN || ideo > IDEO_MAX)
    {
        return NULL;
    }

    buffer[0] = 0xE0 | ideo >> 12;
    buffer[1] = 0x2 << 6 | ((ideo & 0x0FFF) >> 6);
    buffer[2] = 0x2 << 6 | (ideo & 0x3F);
    buffer[3] = 0;

    return buffer;
}

void print_all(void)
{
    unsigned int point;
    int count = 0;
    char *str = NULL;

    printf("CJK Unified Ideographs\n\n");

    for (point = IDEO_MIN; point <= IDEO_MAX; point ++)
    {
        str = gen_code_point(point);
        if ( str == NULL )
        {
            break;
        }
        else
        {
            count ++;
            printf("%s", str);
            if ( count == 16 )
            {
                count = 0;
                printf("\n");
            }
        }
    }

    printf("\n\n");
}

int main(vod)
{
    print_all();

    return 0;
}
