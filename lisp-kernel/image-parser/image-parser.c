#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <stdarg.h>

/*
┌─────────────────────────────────┐
│                                 │
├─────                        ────┤
│                                 │
└─────────────────────────────────┘
*/
#ifdef WINDOWS
#define LSEEK(fd, offset, how) _lseeki64(fd, offset, how)
#else
#define LSEEK(fd, offset, how) lseek(fd, offset, how)
#endif

uint32_t log2_page_size = 12; // 4KB page
static inline uint64_t
_align_to_power_of_2(uint64_t n, uint32_t power)
{
    uint64_t align = (1 << power) - 1;

    return (n + align) & ~align;
}

#define align_to_power_of_2(n, p) _align_to_power_of_2(((uint64_t)(n)), p)

#define IMAGE_SIG0 (('O' << 24) | ('p' << 16) | ('e' << 8) | 'n')
#define IMAGE_SIG1 (('M' << 24) | ('C' << 16) | ('L' << 8) | 'I')
#define IMAGE_SIG2 (('m' << 24) | ('a' << 16) | ('g' << 8) | 'e')
#define IMAGE_SIG3 (('F' << 24) | ('i' << 16) | ('l' << 8) | 'e')

#define PLATFORM_WORD_SIZE_32 0
#define PLATFORM_WORD_SIZE_64 64
#define PLATFORM_CPU_PPC (0 << 3)
#define PLATFORM_CPU_SPARC (1 << 3)
#define PLATFORM_CPU_X86 (2 << 3)
#define PLATFORM_CPU_ARM (3 << 3)
#define PLATFORM_OS_VXWORKS 0
#define PLATFORM_OS_LINUX 1
#define PLATFORM_OS_SOLARIS 2
#define PLATFORM_OS_DARWIN 3
#define PLATFORM_OS_FREEBSD 4
#define PLATFORM_OS_WINDOWS 5
#define PLATFORM_OS_ANDROID 6

#define PLATFORM_WORD_SIZE(f) (f & 0b1000000)

#define fixnumshift64 3L
#define fixnumshift32 2


typedef enum {
  AREA_VOID64 = 0,		/* Not really an area at all */
  AREA_CSTACK64 = 1<<fixnumshift64, /* A control stack */
  AREA_VSTACK64 = 2<<fixnumshift64, /* A value stack.  The GC sees it as being doubleword-aligned */
  AREA_TSTACK64 = 3<<fixnumshift64, /* A temp stack.  It -is- doubleword-aligned */
  AREA_READONLY64 = 4<<fixnumshift64, /* A (cfm) read-only section. */
  AREA_WATCHED64 = 5<<fixnumshift64, /* A static area containing a single object. */
  AREA_STATIC_CONS64 = 6<<fixnumshift64, /* static, conses only */
  AREA_MANAGED_STATIC64 = 7<<fixnumshift64, /* A resizable static area */
  AREA_STATIC64 = 8<<fixnumshift64, /* A  static section: contains
                                 roots, but not GCed */
  AREA_DYNAMIC64 = 9<<fixnumshift64/* A heap. Only one such area is "the heap."*/
} area_code64;

typedef enum {
  AREA_VOID32 = 0,		/* Not really an area at all */
  AREA_CSTACK32 = 1<<fixnumshift32, /* A control stack */
  AREA_VSTACK32 = 2<<fixnumshift32, /* A value stack.  The GC sees it as being doubleword-aligned */
  AREA_TSTACK32 = 3<<fixnumshift32, /* A temp stack.  It -is- doubleword-aligned */
  AREA_READONLY32 = 4<<fixnumshift32, /* A (cfm) read-only section. */
  AREA_WATCHED32 = 5<<fixnumshift32, /* A static area containing a single object. */
  AREA_STATIC_CONS32 = 6<<fixnumshift32, /* static, conses only */
  AREA_MANAGED_STATIC32 = 7<<fixnumshift32, /* A resizable static area */
  AREA_STATIC32 = 8<<fixnumshift32, /* A  static section: contains
                                 roots, but not GCed */
  AREA_DYNAMIC32 = 9<<fixnumshift32/* A heap. Only one such area is "the heap."*/
} area_code32;

wchar_t *enum_names[10] = {
    L"AREA_VOID",
    L"AREA_CSTACK",
    L"AREA_VSTACK",
    L"AREA_TSTACK",
    L"AREA_READONLY",
    L"AREA_WATCHED",
    L"AREA_STATIC_CONS",
    L"AREA_MANAGED_STATIC",
    L"AREA_STATIC",
    L"AREA_DYNAMIC"
};

typedef struct
{
    uint32_t code;
    uint32_t *area;
    uint32_t memory_size;
    uint32_t static_dnodes;
} openmcl_image_section_header32;

typedef struct
{
    uint64_t code;
    uint64_t *area;
    uint64_t memory_size;
    uint64_t static_dnodes;
} openmcl_image_section_header64;

typedef struct
{
    uint32_t sig0, sig1, sig2, sig3;
    uint32_t timestamp;
    uint32_t canonical_image_base_32; /* IMAGE_BASE_ADDRESS */
    uint32_t actual_image_base_32;    /* Hopefully the same */
    uint32_t nsections;
    uint32_t abi_version;
    int32_t section_data_offset_high; /* 64-bit signed offset from end of
                                           section headers to first
                                           section's data.  May be zero. */
    uint32_t section_data_offset_low; /* 64-bit */
    uint32_t flags;
    uint64_t canonical_image_base_64; /* 64-bit */
    uint64_t actual_image_base_64;    /* 64-bit */
} openmcl_image_file_header;

typedef struct
{
    uint32_t sig0, sig1, sig2;
    int32_t delta;
} openmcl_image_file_trailer;

FILE *dbgout = NULL;

/*
  fd is positioned to EOF; header has been allocated by caller.
  If we find a trailer (and that leads us to the header), read
  the header & return true else return false.
*/
bool find_openmcl_image_file_header_and_trailer(
    int fd,
    off_t *eof_pos,
    off_t *trailer_pos,
    off_t *header_pos,
    openmcl_image_file_header *header /* out */,
    openmcl_image_file_trailer *trailer /* out */)
{
    int disp;
    off_t pos;
    unsigned version, flags;

    pos = LSEEK(fd, 0, SEEK_END);
    if (pos < 0)
        return false;

    *eof_pos = pos;

    pos -= sizeof(openmcl_image_file_trailer);

    if ((*trailer_pos = LSEEK(fd, pos, SEEK_SET)) < 0)
        return false;

    if (read(fd, trailer, sizeof(openmcl_image_file_trailer)) != sizeof(openmcl_image_file_trailer))
        return false;

    if ((trailer->sig0 != IMAGE_SIG0) ||
        (trailer->sig1 != IMAGE_SIG1) ||
        (trailer->sig2 != IMAGE_SIG2))
        return false;

    disp = trailer->delta;

    /* Found a valid trailer and got the offset to the header (disp) */

    if (disp >= 0)
        return false;

    if ((*header_pos = LSEEK(fd, disp, SEEK_CUR)) < 0)
        return false;

    if (read(fd, header, sizeof(openmcl_image_file_header)) !=
        sizeof(openmcl_image_file_header))
        return false;

    if ((header->sig0 != IMAGE_SIG0) ||
        (header->sig1 != IMAGE_SIG1) ||
        (header->sig2 != IMAGE_SIG2) ||
        (header->sig3 != IMAGE_SIG3))
        return false;

    return true;
}

off_t seek_to_next_page(int fd)
{
    off_t pos = LSEEK(fd, 0, SEEK_CUR); /* get the current file offset without moving the fd. */
    pos = align_to_power_of_2(pos, log2_page_size); /* align the offset to 4KB pages */
    return LSEEK(fd, pos, SEEK_SET); /* jump to the 4KB aligned version of the current offset */
}

bool load_image_file_sections64(int fd, openmcl_image_file_header *h, openmcl_image_section_header64 sh[])
{
    int i, nsections = h->nsections;
    int64_t section_data_delta = ((int64_t)(h->section_data_offset_high) << 32L) | h->section_data_offset_low;

    if (read(fd, sh, nsections * sizeof(openmcl_image_section_header64)) !=
        nsections * sizeof(openmcl_image_section_header64))
        return false;

    return true;
}

/* load_openmcl_image
 *  fd - a read-only file descriptor to the *.image file for the kernel.
 *  *h - a caller-allocated pointer to the image file header struct.
 *
 *  returns 0 on failure, otherwise a pointer to the NIL object in memory.
//  */
// LispObj
// open_openmcl_image(int fd, openmcl_image_file_header *h /* out */, openmcl_image_file_trailer *t /* out */)
// {
//     LispObj image_nil = 0;
//     area *a;
//     off_t eof_pos, header_pos, trailer_pos;
//     if (find_openmcl_image_file_header_and_trailer(fd, &eof_pos, &trailer_pos, &header_pos, h, t))
//     {
//         int i, nsections = h->nsections;
//         openmcl_image_section_header sections[nsections], *sect = sections;
//         LispObj bias = image_base - ACTUAL_IMAGE_BASE(h);
// #if (WORD_SIZE == 64)
//         signed_natural section_data_delta =
//             ((signed_natural)(h->section_data_offset_high) << 32L) | h->section_data_offset_low;
// #endif

//         if (read(fd, sections, nsections * sizeof(openmcl_image_section_header)) !=
//             nsections * sizeof(openmcl_image_section_header))
//         {
//             return 0;
//         }
// #if WORD_SIZE == 64
//         LSEEK(fd, section_data_delta, SEEK_CUR);
// #endif
//         for (i = 0; i < nsections; i++, sect++)
//         {
//             load_image_section(fd, sect);
//             a = sect->area;
//             if (a == NULL)
//             {
//                 return 0;
//             }
//         }

//         for (i = 0, sect = sections; i < nsections; i++, sect++)
//         {
//             a = sect->area;
//             switch (sect->code)
//             {
//             case AREA_STATIC:
//                 nilreg_area = a;
// #ifdef PPC
// #ifdef PPC64
//                 image_nil = ptr_to_lispobj(a->low + (1024 * 4) + sizeof(lispsymbol) + fulltag_misc);
// #else
//                 image_nil = (LispObj)(a->low + 8 + 8 + (1024 * 4) + fulltag_nil);
// #endif
// #endif
// #ifdef X86
// #ifdef X8664
//                 image_nil = (LispObj)(a->low) + (1024 * 4) + fulltag_nil;
// #else
//                 image_nil = (LispObj)(a->low) + (1024 * 4) + fulltag_cons;
// #endif
// #endif
// #ifdef ARM
//                 image_nil = (LispObj)(a->low) + (1024 * 4) + fulltag_nil;
// #endif
//                 set_nil(image_nil);
//                 if (bias)
//                 {
//                     LispObj weakvll = lisp_global(WEAKVLL);

//                     if ((weakvll >= ((LispObj)image_base - bias)) &&
//                         (weakvll < (ptr_to_lispobj(active_dynamic_area->active) - bias)))
//                     {
//                         lisp_global(WEAKVLL) = weakvll + bias;
//                     }
//                 }
//                 break;

//             case AREA_READONLY:
//                 readonly_area = a;
//                 break;
//             }
//         }
//         for (i = 0, sect = sections; i < nsections; i++, sect++)
//         {
//             a = sect->area;
//             switch (sect->code)
//             {
//             case AREA_MANAGED_STATIC:
//                 break;
//             case AREA_STATIC_CONS:
//                 if (bias)
//                 {
//                     LispObj static_conses = lisp_global(STATIC_CONSES);
//                     if (static_conses && static_conses != lisp_nil)
//                     {
//                         lisp_global(STATIC_CONSES) += bias;
//                     }
//                 }
//                 /* not yet
//          lower_heap_start(static_cons_area->low,tenured_area);
//                 */
//                 break;
//             case AREA_DYNAMIC:
//                 break;
//             }
//         }
//     }
//     return image_nil;
// }

#define BOX_WIDTH 70

void print_boxpart(wchar_t left, wchar_t right, int width)
{
    wprintf(L"%lc", left);
    for (int i = 0; i < width - 2; i++)
        wprintf(L"─");
    wprintf(L"%lc\n", right);
}

void print_boxtop(int width)
{
    print_boxpart(L'┌', L'┐', width);
}

void print_boxbottom(int width)
{
    print_boxpart(L'└', L'┘', width);
}

void print_boxmiddle(int width)
{
    print_boxpart(L'├', L'┤', width);
}

void print_row(int width, wchar_t *label, wchar_t *format, ...)
{
    va_list args;
    int label_size = wcsnlen(label, 25);
    wchar_t buf[40];
    va_start(args, format);
    wprintf(L"│ %ls", label);
    for (int i = 0; i < 26 - label_size; i++)
    {
        wprintf(L" ");
    }
    vswprintf(buf, 40, format, args);
    int value_size = wcsnlen(buf, 40);
    wprintf(buf);
    for (int i = 0; i < (width - 26 - 3) - value_size; i++)
    {
        wprintf(L" ");
    }
    wprintf(L"│\n");
}

void print_label(int width, wchar_t *label, ...)
{
    va_list args;
    wchar_t buf[100];
    va_start(args, label);
    vswprintf(buf, 100, label, args);
    int value_size = wcsnlen(buf, 100);
    wprintf(L"│ ");
    wprintf(buf);
    for (int i = 0; i < width - 3 - value_size; i++)
    {
        wprintf(L" ");
    }
    wprintf(L"│\n");
}

void print_image_file_header(openmcl_image_file_header *h)
{
    uint32_t platform_os = h->flags & 0b0000111;
    uint32_t platform_cpu = h->flags & 0b0111000;
    uint32_t platform_word_size = PLATFORM_WORD_SIZE(h->flags);
    int box_width = BOX_WIDTH;
    print_boxtop(box_width);
    print_label(box_width, L"openmcl_image_file_header: (size: %lu bytes)", sizeof(openmcl_image_file_header));
    print_boxmiddle(box_width);
    print_row(box_width, L"sig0:", L"0x%08x", h->sig0);
    print_row(box_width, L"sig1:", L"0x%08x", h->sig1);
    print_row(box_width, L"sig2:", L"0x%08x", h->sig2);
    print_row(box_width, L"sig3:", L"0x%08x", h->sig3);
    print_row(box_width, L"timestamp:", L"%u", h->timestamp);
    print_row(box_width, L"canonical_image_base_32:", L"0x%08x", h->canonical_image_base_32);
    print_row(box_width, L"actual_image_base_32:", L"0x%08x", h->actual_image_base_32);
    print_row(box_width, L"nsections:", L"%u", h->nsections);
    print_row(box_width, L"abi_version:", L"%u", h->abi_version);

    if (platform_word_size == PLATFORM_WORD_SIZE_64)
    {
        print_row(box_width, L"section_data_offset_high:", L"%d", h->section_data_offset_high);
        print_row(box_width, L"section_data_offset_low:", L"%u", h->section_data_offset_low);
    }
    print_label(box_width, L"flags:");
    wchar_t *label = NULL;

    switch (platform_word_size)
    {
    case PLATFORM_WORD_SIZE_32:
        label = L"32-BIT";
        break;
    case PLATFORM_WORD_SIZE_64:
        label = L"64-BIT";
        break;
    default:
        label = L"UNKNOWN";
        break;
    }
    print_row(box_width, L"    word size:", label);

    switch (platform_os)
    {
    case PLATFORM_OS_VXWORKS:
        label = L"VXWORKS";
        break;
    case PLATFORM_OS_LINUX:
        label = L"LINUX";
        break;
    case PLATFORM_OS_SOLARIS:
        label = L"SOLARIS";
        break;
    case PLATFORM_OS_DARWIN:
        label = L"DARWIN";
        break;
    case PLATFORM_OS_FREEBSD:
        label = L"FREEBSD";
        break;
    case PLATFORM_OS_WINDOWS:
        label = L"WINDOWS";
        break;
    case PLATFORM_OS_ANDROID:
        label = L"ANDROID";
        break;
    default:
        label = L"UNKNOWN";
        break;
    }
    print_row(box_width, L"    os:", label);

    switch (platform_cpu)
    {
    case PLATFORM_CPU_PPC:
        label = L"PPC";
        break;
    case PLATFORM_CPU_SPARC:
        label = L"SPARC";
        break;
    case PLATFORM_CPU_X86:
        label = L"X86";
        break;
    case PLATFORM_CPU_ARM:
        label = L"ARM";
        break;
    default:
        label = L"UNKNOWN";
        break;
    }
    print_row(box_width, L"    cpu:", label);
    if (platform_word_size == PLATFORM_WORD_SIZE_64)
    {
        print_row(box_width, L"canonical_image_base_64:", L"0x%08llx", h->canonical_image_base_64);
        print_row(box_width, L"actual_image_base_64:", L"0x%08llx", h->actual_image_base_64);
    }

    print_boxbottom(box_width);
}

void print_image_file_trailer(openmcl_image_file_trailer *t)
{
    int box_width = BOX_WIDTH;
    print_boxtop(box_width);
    print_label(box_width, L"openmcl_image_file_trailer: (size: %lu bytes)", sizeof(openmcl_image_file_trailer));
    print_boxmiddle(box_width);
    print_row(box_width, L"sig0:", L"0x%08x", t->sig0);
    print_row(box_width, L"sig1:", L"0x%08x", t->sig1);
    print_row(box_width, L"sig2:", L"0x%08x", t->sig2);
    print_row(box_width, L"delta:", L"%d", t->delta);
    print_boxbottom(box_width);
}

void print_image_section_header64(openmcl_image_section_header64 *sh)
{
    int box_width = BOX_WIDTH;
    print_boxtop(box_width);
    print_label(box_width, L"openmcl_image_section_header64: (size: %lu bytes)", sizeof(openmcl_image_section_header64));
    print_boxmiddle(box_width);
    print_row(box_width, L"code:", L"0x%08x (%ls)", sh->code, enum_names[sh->code >> fixnumshift64]);
    print_row(box_width, L"*area:", L"0x%08x", sh->area);
    print_row(box_width, L"memory_size:", L"0x%08x", sh->memory_size);
    print_row(box_width, L"static_dnodes:", L"0x%08x", sh->static_dnodes);
    print_boxbottom(box_width);
}

void print_image_section_static64(int fd, openmcl_image_section_header64 *sh)
{
    off_t pos = seek_to_next_page(fd);
    uint8_t buf[0x4000];
    char chars[17];
    chars[16] = 0;
    int ch_idx = 0;
    ssize_t bytes_read = read(fd, buf, sh->memory_size);
    for (int i = 0; i < sh->memory_size; i++) {
        if (i % 16 == 0)
        {
            if (i > 0) 
                printf(" %s", chars);
            ch_idx = 0;
            
            for (int j=0; j < 17; j++)
                chars[j] = 0;
            
            printf("\n%04x: ", i);
        }
        if ((i + 8) % 16 == 0) 
            printf(" ");
        printf("%02x ", buf[i]);
        if (buf[i] >= 32 && buf[i] <= 126)
            chars[ch_idx] = buf[i];
        else
            chars[ch_idx] = '.';
        ch_idx++;
    }
    printf("%s", chars);
}

void usage()
{
    fprintf(stderr, "usage: image-parser <lisp image path>\n");
    exit(-1);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
        usage();

    setlocale(LC_ALL, "en_US.UTF-8");

    openmcl_image_file_header h;
    openmcl_image_file_trailer t;
    off_t eof_pos, trailer_pos, header_pos;
    

    dbgout = stderr;
    int fd = open(argv[1], O_RDONLY, 0666);
    bool found = find_openmcl_image_file_header_and_trailer(
        fd, &eof_pos, &trailer_pos, &header_pos, &h, &t);

    openmcl_image_section_header64 *sh = malloc(h.nsections * sizeof(openmcl_image_section_header64));
    load_image_file_sections64(fd, &h, sh);

    /* sections are stored at a different offset on 64-bit than 32-bit. This delta
     * is from the end of the section headers.
     */
    if (PLATFORM_WORD_SIZE(h.flags) == PLATFORM_WORD_SIZE_64)
    {
        off_t curr_pos = LSEEK(fd, 0, SEEK_CUR);
        int64_t section_data_delta = 
            ((int64_t)(h.section_data_offset_high) << 32L) | h.section_data_offset_low;
        wprintf(L" Section Data Offset: %lld, Absolute Position: %lld\n", section_data_delta, curr_pos + section_data_delta);
        LSEEK(fd, section_data_delta, SEEK_CUR);
    }


    printf(" Header offset: %lld (from end: %lld)\n", header_pos, header_pos - eof_pos);
    print_image_file_header(&h);
    for (int i=0; i<h.nsections; i++)
    {
        print_image_section_header64(&sh[i]);
    }
    
    printf(" Trailer offset: %lld (from end: %lld)\n", trailer_pos, trailer_pos - eof_pos);
    print_image_file_trailer(&t);
    printf(" EOF offset: %lld (from end: 0)\n", eof_pos);
/*
    for (int i = 0; i < h.nsections; i++)
    {
        openmcl_image_section_header64 *sect = &sh[i];
        if (sect->code == AREA_STATIC64)
        {
            print_image_section_static64(fd, sect);
        }
    }
*/
    printf("\n");

    if (sh) free(sh);
}