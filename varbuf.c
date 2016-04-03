#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <ctype.h>
#include <err.h>

#include "varbuf.h"

void
varbuf_reset(struct varbuf *xp)
{
	xp->offset = 0;
	xp->datalen = 0;
	xp->buf[0] = '\0';
}

void
varbuf_forward(struct varbuf *xp, int len)
{
	if (xp->offset + len > xp->buflen) {
		varbuf_reset(xp);
		return;
	}

	xp->offset += len;
	xp->datalen -= len;
}

/**
 * copy the data pointed by varbuf->offset into the top of the buffer.
 */
void
varbuf_rewind(struct varbuf *xp)
{
	memcpy(xp->buf, VARBUF_HEAD(xp), VARBUF_DATALEN(xp));
	xp->offset = 0;
	*VARBUF_TAIL(xp) = '\0';
}

void
varbuf_fdump(struct varbuf *xp, const char sprtr, const char newline)
{
	int i;

	for (i = 0; i < VARBUF_DATALEN(xp); i++) {
		if (i != 0) {
			if (i % 16 == 0 && newline)
				printf("%c", newline);
			else
			if (i % 4 == 0 && sprtr)
				printf("%c", sprtr);
		}
		printf("%02x", VARBUF_HEAD(xp)[i]&0xff);
	}
	printf("%c", newline);
}

void
varbuf_dump(struct varbuf *xp)
{
	varbuf_fdump(xp, ' ', '\n');
}

void
varbuf_print(struct varbuf *xp)
{
	int max, i;

	/* check the buffer if those are printable ? */
	if (xp->datalen == 0) {
		printf("(nothing)\n");
		return;
	}

	max = VARBUF_DATALEN(xp);

	for (i = 0; i < max; i++) {
		if (!isprint(VARBUF_HEAD(xp)[i]&0xff)) {
			printf("(non-printable charactor %02x at %d)",
			    VARBUF_HEAD(xp)[i]&0xff, i);
		} else {
			printf("%c", VARBUF_HEAD(xp)[i]&0xff);
		}
	}
	printf("\n");
}

void
varbuf_free(struct varbuf *xp)
{
	free(xp->buf);
	free(xp);
}

struct varbuf *
varbuf_new(int buflen, int type)
{
	struct varbuf *new;

	switch (type) {
	case VARBUF_STR:
	case VARBUF_MEM:
		break;
	default:
		err(1, "ERROR: %s: invalid type %d", __FUNCTION__, type);
	}

	if ((new = calloc(1, sizeof(*new))) == NULL)
		err(1, "ERROR: %s: calloc(varbuf)", __FUNCTION__);

	new->buflen = buflen;
	if ((new->buf = calloc(1, new->buflen)) == NULL)
		err(1, "ERROR: %s: calloc(varbuf->buflen)", __FUNCTION__);
	new->offset = 0;

	new->type = type;

	return new;
}

struct varbuf *
varbuf_strnew(int buflen)
{
	return varbuf_new(buflen, VARBUF_STR);
}

struct varbuf *
varbuf_memnew(int buflen)
{
	return varbuf_new(buflen, VARBUF_MEM);
}

int
varbuf_cmp(struct varbuf *xp, const char *str)
{
	return strcmp(VARBUF_HEAD(xp), str);
}

int
varbuf_ncmp(struct varbuf *xp, const char *str, size_t size)
{
	return strncmp(VARBUF_HEAD(xp), str, size);
}

/**
 * @brief copy the formatted string into the end of the data in the buffer.
 * @param xp pointer to the varbuf, it must not be NULL.
 */
size_t
varbuf_vstrcat(struct varbuf *xp, const char *fmt, ...)
{
	va_list ap;
	size_t len;

	/*
	 * more than or equal to 2 bytes is required to copy a string
	 * if there is no enough space, just return the current datalen.
	 */
	if (VARBUF_RESTLEN(xp) < 2)
		return VARBUF_DATALEN(xp);

	/* make it sure of NULL termination. */
	*VARBUF_TAIL(xp) = '\0';

	va_start(ap, fmt);
	len = vsnprintf(VARBUF_TAIL(xp), 1 + VARBUF_RESTLEN(xp), fmt, ap);
	va_end(ap);

	if (len > VARBUF_RESTLEN(xp))
		xp->datalen += VARBUF_RESTLEN(xp);
	else
		xp->datalen += len;

	return VARBUF_DATALEN(xp);
}

/*
 * copy the formatted string into the top of the varbuf.
 * if the type of varbuf is MEMORY, it returns 0.
 *
 * @return copyied length.
 */
size_t
varbuf_vstrcpy(struct varbuf *xp, const char *fmt, ...)
{
	va_list ap;
	int len;

	varbuf_reset(xp);

	va_start(ap, fmt);
	len = varbuf_vstrcat(xp, fmt, ap);
	va_end(ap);

	return len;
}

/**
 * @brief  copy str into the end of the data in the buffer.
 * @return the length of the total data in the buffer.
 *         the return length doesn't include the NULL in the end.
 * @note   if the adding operation is done, the buffer is assured to be
 *         terminated by NULL.  Otherwise, it doesn't care.
 */
size_t
varbuf_strcat(struct varbuf *xp, const char *str)
{
	return varbuf_vstrcat(xp, str);
}

size_t
varbuf_strcpy(struct varbuf *xp, const char *str)
{
	varbuf_reset(xp);

	return varbuf_strcat(xp, str);
}

/**
 * copy the size of the data into the varbuf pointed by the offset passed.
 * the point is actually xp->offset (valid point) + offset
 * if the size is bigger than the remaining length of the buffer,
 * it will copy the data as long as possible.
 * it puts NULL at the end of the data if xp->type is STRING.
 *
 * @return copyied length.
 */
size_t
varbuf_memcpy(struct varbuf *xp, size_t offset, const char *data, size_t size)
{
	if (size + offset >= VARBUF_RESTLEN(xp))
		size = VARBUF_RESTLEN(xp);

	memcpy(VARBUF_HEAD(xp) + offset, data, size);
	if (offset + size > VARBUF_DATALEN(xp))
		xp->datalen = offset + size;

	if (xp->type == VARBUF_STR)
		*VARBUF_TAIL(xp) = '\0';

	return size;
}
