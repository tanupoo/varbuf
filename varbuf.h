/*
 * varbuf is a simple library to manipulate a buffer.
 *
 *  |------------------ buflen ----------------|
 *  |-- offset --|------- datalen -------|
 *  v
 * buf
 *        * buflen < offset + datalen
 *
 *  e.g.  buflen = 10
 *    0 1 2 3 4 5 6 7 8 9
 *        |
 *        +- if offset = 2, max datalen = 7
 */

#ifdef __linux__
#ifndef strlcpy
#define strlcpy(a, b, c) snprintf(a, c, "%s", b)
#endif
#endif

struct varbuf {
/* pointer to the next of the end of the valid data. */
#define VARBUF_HEAD(x)    (x->buf + x->offset)
#define VARBUF_TAIL(x)    (VARBUF_HEAD(x) + x->datalen)
#define VARBUF_DATALEN(x) (x->datalen)
/* remain length in the buffer */
#define VARBUF_RESTLEN(x) (x->buflen - x->offset - x->datalen - 1)
	char *buf;	/* pointer to the top of the buffer */
	int buflen;	/* maximum size of the buffer */
	int offset;	/* offset to the top of the valid data */
	int datalen;	/* valid data length */

	int type;	/* buffer type: string or memory */
#define VARBUF_STR 1	/* the buffer is always terminating by NULL */
#define VARBUF_MEM 2
};

void varbuf_reset(struct varbuf *);
void varbuf_forward(struct varbuf *, int);
void varbuf_rewind(struct varbuf *);
void varbuf_fdump(struct varbuf *, const char, const char);
void varbuf_dump(struct varbuf *);
void varbuf_print(struct varbuf *);
void varbuf_free(struct varbuf *);
struct varbuf *varbuf_new(int, int);
struct varbuf *varbuf_strnew(int);
struct varbuf *varbuf_memnew(int);
int varbuf_cmp(struct varbuf *, const char *);
int varbuf_ncmp(struct varbuf *, const char *, size_t);
size_t varbuf_strcat(struct varbuf *, const char *);
size_t varbuf_strcpy(struct varbuf *, const char *);
size_t varbuf_vstrcat(struct varbuf *, const char *, ...);
size_t varbuf_vstrcpy(struct varbuf *, const char *, ...);
size_t varbuf_memcpy(struct varbuf *, size_t, const char *, size_t);
