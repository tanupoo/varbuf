#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>

#include "varbuf.h"

void
varbuf_stat(struct varbuf *x)
{
	printf("HEAD=%p TAIL=%p DATALEN=%d RESTLEN=%d\n",
	    VARBUF_HEAD(x), VARBUF_TAIL(x),
	    VARBUF_DATALEN(x), VARBUF_RESTLEN(x));
	varbuf_print(x);
	varbuf_dump(x);
}

int
test_varbuf_base()
{
	struct varbuf *x;
	int buflen = 16;
	int n_forward = 2;
	char *s0 = "0123456789";
	char *s1 = "012";
	char *s2 = "345";
	int len;

	printf("\n====\n");

	printf("-- allocate varbuf buflen=%d\n", buflen);
	x = varbuf_strnew(buflen);
	varbuf_stat(x);

	printf("-- add %s\n", s0);
	len = varbuf_strcat(x, s0);
	varbuf_stat(x);
	if (varbuf_cmp(x, "0123456789") != 0)
		warnx("ERROR: unexpected result");

	printf("-- add %s\n", s0);
	len = varbuf_strcat(x, s0);
	varbuf_stat(x);
	if (varbuf_cmp(x, "012345678901234") != 0)
		warnx("ERROR: unexpected result");

	printf("-- reset\n");
	varbuf_reset(x);
	varbuf_stat(x);

	printf("-- add %s\n", s1);
	len = varbuf_strcat(x, s1);
	varbuf_stat(x);
	if (varbuf_cmp(x, "012") != 0)
		warnx("ERROR: unexpected result");

	printf("-- add %s\n", s2);
	len = varbuf_strcat(x, s2);
	varbuf_stat(x);
	if (varbuf_cmp(x, "012345") != 0)
		warnx("ERROR: unexpected result");

	printf("-- forward %d bytes\n", n_forward);
	varbuf_forward(x, n_forward);
	varbuf_stat(x);
	if (varbuf_cmp(x, "2345") != 0)
		warnx("ERROR: unexpected result");

	printf("-- rewind\n");
	varbuf_rewind(x);
	varbuf_stat(x);
	if (varbuf_cmp(x, "2345") != 0)
		warnx("ERROR: unexpected result");

	printf("-- add %s%.3fcdef\n", "ab", 99.1234);
	len = varbuf_vstrcat(x, "%s%.3fcdef", "ab", 99.1234);
	varbuf_stat(x);
	if (varbuf_cmp(x, "2345ab99.123cde") != 0)
		warnx("ERROR: unexpected result");

	varbuf_free(x);

	return 0;
}

int
test_varbuf_memcpy()
{
	struct varbuf *x;
	char *s1 = "01234";
	char *s2 = "56789";
	char *s3 = "abce";

	printf("\n==== %s\n", __FUNCTION__);

	x = varbuf_strnew(10);
	varbuf_stat(x);

	printf("-- memcpy \"%s\" to buf[0]\n", s1);
	varbuf_memcpy(x, 0, s1, strlen(s1));
	varbuf_stat(x);

	printf("-- memcpy \"%s\" to buf[3]\n", s2);
	varbuf_memcpy(x, 3, s2, strlen(s2));
	varbuf_stat(x);

	printf("-- memcpy \"%s\" to buf[1]\n", s3);
	varbuf_memcpy(x, 1, s3, strlen(s2));
	varbuf_stat(x);

	printf("[%s]\n", x->buf);

	varbuf_free(x);

	return 0;
}

int
main(int argc, char *argv[])
{
	test_varbuf_base();
	test_varbuf_memcpy();

	return 0;
}

