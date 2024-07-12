#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"


//CRIA IMAGEM
//WHITE-BLACK
//BLACK-WHITE
//VC03_24 
int main(void)
{
	IVC* image;
	int x, y;
	long int pos;

	image = vc_image_new(320, 280, 1, 1);

	for (x = 0; x < image->width; x++)
	{
		for (y = 0; y < image->height; y++)
		{
			pos = y * image->bytesperline + x * image->channels;

			if ((x <= image->width / 2) && (y <= image->height / 2)) image->data[pos] = 1;
			else if ((x > image->width / 2) && (y > image->height / 2)) image->data[pos] = 1;
			else image->data[pos] = 0;
		}
	}

	vc_write_image("vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}