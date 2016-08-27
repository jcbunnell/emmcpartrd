
#include <stdio.h>
#include <fcntl.h>

typedef unsigned long long       u64;
typedef unsigned int       u32;
typedef signed int     s32;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned char      u8;
typedef signed char    s8;

#define BLOCK_DEF_SIZE  0x80
#define NAME_SIZE       16

typedef struct _PARTITION_DESC
{
    u32 unknown1[8];
    u32 startBlockLo;
    u32 startBlockHi;
    u32 endBlockLo;
    u32 endBlockHi;
    u32 unknown2[2];
    u16 partitionName[NAME_SIZE];
    u32 unknown3[10];
} PARTITION_DESC;
int main(int argc, char *argv[])
{
    int k;
    char *imageName = NULL;
    FILE *fp = NULL;

    for (k=1;k<argc;++k)
    {
        if (argv[k][0] == '-')
        {
            switch (argv[k][1])
            {
                case 'f':
                    if (argv[k][2])
                    {
                        imageName = &argv[k][2];
                    }
                    else if ((++k) < argc)
                    {
                        imageName =  argv[k];
                    }
                    break;
                default:
                    break;
            }   // end SWITCH
        }   // end dash check
    }   // end FOR loop on argc

    if (imageName)
    {
        char name[NAME_SIZE+1];
        char *psize;
        u64 partSize;
        s32 bytesRead;
        int partNum=1;
        u32 hole;
        u32 lastend = 0;
        unsigned char buffer[BLOCK_DEF_SIZE];
        PARTITION_DESC *pPart = (PARTITION_DESC *)buffer;
        fp = fopen(imageName, "rb");
        if (fp)
        {
            fseek(fp, 0x400, SEEK_SET);
            do
            {
                bytesRead = fread(buffer, 1, BLOCK_DEF_SIZE, fp);
                if ((bytesRead == BLOCK_DEF_SIZE) && pPart->unknown1[0])
                {
                    psize = "bytes";
                    partSize = (u64)((u64)(1 + pPart->endBlockLo - pPart->startBlockLo) * (u64)512);
                    if (partSize > (u64)1024)
                    {
                        psize = "KB";
                        partSize /= (u64)1024;
                        if (partSize > (u64)1024)
                        {
                            psize = "MB";
                            partSize /= (u64)1024;
                            if (partSize > (u64)1024)
                            {
                                psize = "GB";
                                partSize /= (u64)1024;
                            }
                        }
                    }
                    k=0;
                    while (pPart->partitionName[k])
                    {
                        name[k] = (char)pPart->partitionName[k];
                        ++k;
                    }
                    name[k] = 0;
                    if (lastend)
                    {
                        hole = pPart->startBlockLo - (lastend +1);
                        if (hole)
                        {
                            printf("partNum=%d startBlock=0x%08X endBlock=0x%08X size=%d%s hole=0x%X name=%s\n", partNum, pPart->startBlockLo, pPart->endBlockLo, (int)partSize, psize, hole, name);
                        }
                        else
                        {
                            printf("partNum=%d startBlock=0x%08X endBlock=0x%08X size=%d%s name=%s\n", partNum, pPart->startBlockLo, pPart->endBlockLo, (int)partSize, psize, name);
                        }
                    }
                    else
                    {
                        printf("partNum=%d startBlock=0x%08X endBlock=0x%08X size=%d%s name=%s\n", partNum, pPart->startBlockLo, pPart->endBlockLo, (int)partSize, psize, name);
                    }
                    lastend = pPart->endBlockLo;
                }

                ++partNum;
            } while (pPart->unknown1[0]);
            close(fp);
        }
    }
    return 0;
}
