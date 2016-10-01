
#include <stdio.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys/stat.h>

typedef unsigned long long u64;
typedef long long s64;
typedef unsigned int u32;
typedef signed int s32;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned char u8;
typedef signed char s8;

#define BLOCK_DEF_START 0x400
#define BLOCK_DEF_SIZE  0x80
#define NAME_SIZE       36
#define EMMC_BLOCK_SIZE 512

typedef struct _PARTITION_ENUM
{
    u32 enumValue;
    u32 reserved[3];
} PARTITION_ENUM;

typedef struct _GUID
{
    u32 Data1;
    u16 Data2;
    u16 Data3;
    u8 Data4[8];
} GUID;


typedef struct _PARTITION_DESC
{
    GUID partGUID;
    PARTITION_ENUM partitionEnum;
    u64 startBlock;
    u64 endBlock;
    u64 qvalue;
    u16 partitionName[NAME_SIZE];
} PARTITION_DESC;

int verboseLevel = 0;

char* sizeSuffix(u64 *size)
{
    char *psize = " B";

    if (*size >= 1024)
    {
        psize = "KB";
        *size /= 1024;
        if (*size >= 1024)
        {
            psize = "MB";
            *size /= 1024;
            if (*size >= 1024)
            {
                psize = "GB";
                *size /= 1024;
            }
        }
    }
    return psize;
}

void showPartitions(PARTITION_DESC *pPart, u32 lastend)
{
    char aname[NAME_SIZE];
    char *phole;
    int k;
    u64 partSize = (u64)((u64)(1 + pPart->endBlock - pPart->startBlock) * (u64)EMMC_BLOCK_SIZE);
    char *psize = sizeSuffix(&partSize);
    u64 hole = 0;

    if (pPart)
    {
        for (k = 0; k < NAME_SIZE; ++k)
        {
            aname[k] = (char)pPart->partitionName[k];
            if (pPart->partitionName[k] == 0)
            {
                break;
            }
        }
        if (lastend)
        {
            hole = (pPart->startBlock - (lastend + 1)) * EMMC_BLOCK_SIZE;
        }

        if (hole)
        {
            phole = sizeSuffix(&hole);
            printf("partEnum=%02d  name=%9s\tstartBlock=%08llX\tendBlock=%08llX\tsize=%d%s\thole=%d%s\tGUID:{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}\r\n",
                   pPart->partitionEnum.enumValue,
                   aname,
                   pPart->startBlock,
                   pPart->endBlock,
                   (int)partSize,
                   psize,
                   (int)hole,
                   phole,
                   pPart->partGUID.Data1,
                   pPart->partGUID.Data2,
                   pPart->partGUID.Data3,
                   pPart->partGUID.Data4[0],
                   pPart->partGUID.Data4[1],
                   pPart->partGUID.Data4[2],
                   pPart->partGUID.Data4[3],
                   pPart->partGUID.Data4[4],
                   pPart->partGUID.Data4[5],
                   pPart->partGUID.Data4[6],
                   pPart->partGUID.Data4[7]);
        }
        else
        {
            printf("partEnum=%02d  name=%9s\tstartBlock=%08llX\tendBlock=%08llX\tsize=%d%s\tGUID:{%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X}\r\n",
                   pPart->partitionEnum.enumValue,
                   aname,
                   pPart->startBlock,
                   pPart->endBlock,
                   (int)partSize,
                   psize,
                   pPart->partGUID.Data1,
                   pPart->partGUID.Data2,
                   pPart->partGUID.Data3,
                   pPart->partGUID.Data4[0],
                   pPart->partGUID.Data4[1],
                   pPart->partGUID.Data4[2],
                   pPart->partGUID.Data4[3],
                   pPart->partGUID.Data4[4],
                   pPart->partGUID.Data4[5],
                   pPart->partGUID.Data4[6],
                   pPart->partGUID.Data4[7]);
        }
    }
}

int main(int argc, char *argv[])
{
    int k;
    char *imageName = NULL;
    int fpBlk0 = -1;
    int bShow = 0;
    int bShowHelp = 0;
    struct stat fs;

    for (k = 1; k < argc; ++k)
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
                    imageName = argv[k];
                }
                break;
            case 's':
                bShow = 1;
                break;
            case 'v':
                if (argv[k][2])
                {
                    int m = 1;
                    while (argv[k][m] == 'v')
                    {
                        ++verboseLevel;
                        ++m;
                    }
                }
                break;
            case 'h':
                bShowHelp = 1;
                break;
            default:
                break;
            }   // end SWITCH
        }   // end dash check
    }   // end FOR loop on argc

    if (verboseLevel)
    {
        printf("Verbose level = %d\n", verboseLevel);
    }

    if (imageName == NULL)
    {
        bShowHelp = 1;
    }

    if (bShowHelp == 0)
    {
        s32 bytesRead;
        u32 lastend = 0;
        unsigned char buffer[BLOCK_DEF_SIZE];
        PARTITION_DESC *pPart = (PARTITION_DESC *)buffer;

        // open the eMMC partition image (this can be a file created from dd (e.g. dd count=100 if=/dev/block/mmcblk0 of=inputFileToHere)
        fpBlk0 = open(imageName, O_RDWR);
        if (fpBlk0 != -1)
        {
            bytesRead = read(fpBlk0, buffer, 8);
            // check to see if this is a boot volume
            if (0 == memcmp(buffer, "ANDROID!", 8))
            {
                printf("This is a boot image (partition), not a mmcblk0 image.\n");
            }
            else
            {
                lseek(fpBlk0, BLOCK_DEF_START, SEEK_SET);
                do
                {
                    bytesRead = read(fpBlk0, buffer, BLOCK_DEF_SIZE);
                    if ((bytesRead == BLOCK_DEF_SIZE) && pPart->partGUID.Data1)
                    {
                        // display the information, if requested
                        if (bShow)
                        {
                            showPartitions(pPart, lastend);
                        }
                        lastend = pPart->endBlock;
                    }
                } while (pPart->partGUID.Data1);
            }
            close(fpBlk0);
        }
    }
    else
    {
        printf("listemmc: -f <partition filename>\n");
        printf("      -s      show partition information\n");
        printf("      -v      verbose level (additive)\n");
    }
    return 0;
}
