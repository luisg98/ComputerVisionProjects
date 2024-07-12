#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "../vc.h"

//Abrir imagem, alterar e gravar em novo ficheiro
//VC03_19 INVERTER CORES ---image->data[i] = 255 - image->data[i]---;
int main(void)
{
	IVC* image;
	int i;
	image = vc_read_image("Import/Images/FLIR/flir-05.pgm");

	for (i = 0; i < image->bytesperline * image->height; i += image->channels)
	{
		image->data[i] = 255 - image->data[i];
	}

	vc_write_image("Export/vc-0001.pgm", image);

	vc_image_free(image);

	printf("Press any key to exit...\n");
	getchar();

	return 0;
}