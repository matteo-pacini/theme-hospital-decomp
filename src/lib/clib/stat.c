#include "clib.h"

#define ENOENT 1

#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#define AT_ISCHR  0x40

#define S_IFDIR 0x4000
#define S_IFCHR 0x2000
#define S_IFREG 0x8000
#define S_IRUSR 0x0100
#define S_IWUSR 0x0080
#define S_IXUSR 0x0040
#define S_IRGRP 0x0020
#define S_IWGRP 0x0010
#define S_IXGRP 0x0008
#define S_IROTH 0x0004
#define S_IWOTH 0x0002
#define S_IXOTH 0x0001

typedef long time_t;

struct find_t {
    char reserved[21];
    char attrib;
    unsigned short wr_time;
    unsigned short wr_date;
    unsigned long size;
    char name[13];
};

struct stat {
    int st_dev;
    int st_ino;
    unsigned short st_mode;
    short st_nlink;
    int st_uid;
    short st_gid;
    int st_rdev;
    long st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
    time_t st_btime;
    unsigned long st_attr;
    unsigned long st_archivedID;
    unsigned long st_updatedID;
    unsigned short st_inheritedRightsMask;
    unsigned char st_originatingNameSpace;
    unsigned char st_name[13];
};

typedef int tiny_ret_t;

void _TinySetDTA(struct find_t *dta);
#pragma aux _TinySetDTA = \
    "mov ah,1ah" \
    "int 21h" \
    parm caller [edx] modify [eax];

tiny_ret_t _TinyFindFirst(const char *path, unsigned attr);
#pragma aux _TinyFindFirst = \
    "mov ah,4eh" \
    "int 21h" \
    "rcl eax,1" \
    "ror eax,1" \
    parm caller [edx] [ecx] value [eax];

unsigned char _TinyGetCurrDrive(void);
#pragma aux _TinyGetCurrDrive = \
    "mov ah,19h" \
    "int 21h" \
    value [al] modify [eax];

#define TINY_ERROR(rc) ((rc) < 0)

#pragma function(strchr)


char *strpbrk(const char *s, const char *set);
int strcmp(const char *s, const char *t);
char *strncpy(char *dst, const char *src, size_t n);
int tolower(int c);
char *getcwd(char *buf, size_t size);
int chdir(const char *path);
time_t _d2ttime(unsigned date, unsigned time);

unsigned short at2mode(int attr, char *fname);

// FUNCTION: 0xec750
int stat(const char *path, struct stat *buf)
{
    struct find_t dta;
    char cwd[68];
    const char *ptr;
    tiny_ret_t rc;

    if (*path == '\0' || strpbrk(path, "*?") != NULL) {
        __set_errno(ENOENT);
        return -1;
    }
    ptr = path;
    if ((int)path[1] == ':')
        ptr += 2;
    if (((int)ptr[0] == '\\' || (int)ptr[0] == '/') && ptr[1] == '\0') {
        getcwd(cwd, sizeof(cwd));
        if (chdir(path) != 0)
            return -1;
        chdir(cwd);
        dta.attrib = _A_SUBDIR;
        dta.wr_time = 0;
        dta.wr_date = 0;
        dta.size = 0;
    } else {
        _TinySetDTA(&dta);
        rc = _TinyFindFirst(path, _A_RDONLY | _A_HIDDEN | _A_SYSTEM | _A_SUBDIR | _A_ARCH);
        if (TINY_ERROR(rc)) {
            __set_errno(ENOENT);
            return -1;
        }
    }
    if ((int)path[1] == ':')
        buf->st_dev = tolower(*path) - 'a';
    else
        buf->st_dev = _TinyGetCurrDrive();
    buf->st_rdev = buf->st_dev;
    buf->st_size = dta.size;
    buf->st_mode = at2mode(dta.attrib, dta.name);
    buf->st_mtime = _d2ttime(dta.wr_date, dta.wr_time);
    buf->st_atime = buf->st_ctime = buf->st_btime = buf->st_mtime;
    buf->st_nlink = 1;
    buf->st_ino = buf->st_uid = buf->st_gid = 0;
    buf->st_attr = dta.attrib;
    buf->st_archivedID = 0;
    buf->st_updatedID = 0;
    buf->st_inheritedRightsMask = 0;
    buf->st_originatingNameSpace = 0;
    strncpy(buf->st_name, dta.name, 13);
    return 0;
}

// FUNCTION: 0xec89a
unsigned short at2mode(int attr, char *fname)
{
    unsigned short mode;
    char *ext;

    if (attr & _A_SUBDIR) {
        mode = S_IFDIR | S_IXUSR | S_IXGRP | S_IXOTH;
    } else if (attr & AT_ISCHR) {
        mode = S_IFCHR;
    } else {
        mode = S_IFREG;
        if ((ext = strchr(fname, '.')) != NULL) {
            ++ext;
            if (strcmp(ext, "EXE") == 0 || strcmp(ext, "COM") == 0)
                mode |= S_IXUSR | S_IXGRP | S_IXOTH;
        }
    }
    mode |= S_IRUSR | S_IRGRP | S_IROTH;
    if (!(attr & _A_RDONLY))
        mode |= S_IWUSR | S_IWGRP | S_IWOTH;
    return mode;
}
