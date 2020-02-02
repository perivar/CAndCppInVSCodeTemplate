#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "wcxhead.h"

int main(int argc, char *argv[])
{
    int r;
    tOpenArchiveDataW archive;

    r = file_open(argv[0], &archive);
    if (r != 0)
        return r;
}

int file_open(const char *file_name, tOpenArchiveDataW *ret)
{
    int fd;
    int r;

    fd = open(file_name, O_RDONLY | O_BINARY);
    if (fd < 0)
        return -errno;

    r = archive_open_fd(fd, ret);
    if (r != 0)
    {
        close(fd);
        return r;
    }

    return 0;
}

bool read_string(int fd, char *ret, size_t size)
{
    ssize_t cnt;

    if (size > SSIZE_MAX)
        return false;

    cnt = read(fd, ret, size - 1);
    if (cnt < 0 || cnt + 1 != (ssize_t)size)
        return false;

    ret[size - 1] = 0;
    return true;
}

bool read_u32_le(int fd, uint32_t *ret)
{
    int n;
    uint8_t c;
    uint32_t num = 0;

    for (n = 0; n < 4; n++)
    {
        if (read(fd, &c, 1) != 1)
            return false;

        num |= (c << (8 * n));
    }

    *ret = num;
    return true;
}

bool read_u16_le(int fd, uint16_t *ret)
{
    int n;
    uint8_t c;
    uint16_t num = 0;

    for (n = 0; n < 2; n++)
    {
        if (read(fd, &c, 1) != 1)
            return false;

        num |= ((uint16_t)c << (8 * n));
    }

    *ret = num;
    return true;
}

int archive_open_fd(int fd, tOpenArchiveDataW *archive)
{
    archive->ArcName = L"TestName";

    if (!read_u32_le(fd, &archive->OpenMode))
        return -EIO;

    if (!read_u32_le(fd, &archive->OpenResult))
        return -EIO;

    if (!read_u32_le(fd, &archive->CmtSize))
        return -EIO;

    return 0;
}
