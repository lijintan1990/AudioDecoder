#include <stdio.h>
#include <stdlib.h>

#include "resample.h"

#define RS_UNIT	320

struct rs_data *rs;

/*int start_resample(src, dst, int src_rate, int dst_rate)
{
	struct rs_data *data = resample_init(src_rate, dst_rate, length);
	resample(data, in_buf, in_buf_size, out_buf, out_buf_size, int last);
	resample_close(rs);
}
*/
int main(int argc, char const *argv[])
{
	/* code */

	if (argc != 5) {
		printf("use like this: ./run in_file in_rate out_file out_rate\n");
		return -1;
	}

	const char *in_file = argv[1];
	int in_rate = atoi(argv[2]);

	const char *out_file = argv[3];
	int out_rate = atoi(argv[4]);


	printf("in_file: %s\n", in_file);
	printf("in_rate: %d\n", in_rate);
	printf("out_file: %s\n", out_file);
	printf("out_rate: %d\n", out_rate);

	FILE *fp1 = fopen(in_file, "rb");
	if (!fp1) {
		printf("file %s open error\n", in_file);
		return -1;
	}

	FILE *fp2 = fopen(out_file, "wb");
	if (!fp2) {
		printf("file %s open error\n", out_file);
		fclose(fp1);
		return -1;
	}

	fseek(fp1, 0, SEEK_END);
	int length = ftell(fp1);

	double factor = out_rate/(double)in_rate;

	short *in_buf = malloc(length * 2 );
	short *out_buf = malloc(length * 2 * ((int)factor + 1));

	fseek(fp1, 0, SEEK_SET);
	int read_len = fread(in_buf, 1, length, fp1);

	printf("length is %d, read_len is %d\n", length, read_len);

	int loop = length/RS_UNIT;
	int remainder = length - loop * RS_UNIT;

	printf("%s(), loop: %d, remainder: %d\n", __func__, loop, remainder);
	int i = 0, num = 0, count = 0;
    for (i = 0; i < loop; i++) {
		num = resample_simple(factor, in_buf + i * RS_UNIT, out_buf + count, RS_UNIT);
		count += num;
		printf("%s(), index: %d, num: %d, count: %d\n", __func__, i, num, count);
	}
    num = resample_simple(factor, in_buf + i * RS_UNIT, out_buf + count, remainder);
    count += num;

    printf("%s(), num: %d, total count: %d\n", __func__, num, count);


#if 0
	rs = resample_init(in_rate, out_rate, length);

	int num = resample(rs, in_buf, length, out_buf, length * 2 * ((int)factor + 1), 1);

	resample_close(rs);

	printf("num is %d\n", num);
#else
	//int num = resample_simple(factor, in_buf, out_buf, 10240);
#endif
	fwrite(out_buf, 1, count, fp2);

	fclose(fp1);
	fclose(fp2);

	free(in_buf);
	free(out_buf);

	return 0;
}