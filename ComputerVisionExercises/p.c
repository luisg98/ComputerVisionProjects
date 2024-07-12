#include <stdio.h>
#include "vc.h"
#include <stdlib.h>
#include <string.h>

int main(void)
{
    IVC* image;
    IVC* blurred;
    int nlabels;

    // Lê a imagem
    image = vc_read_image("barbara-sp.pgm");
    if (image == NULL) {
        printf("ERRO -> vc_read_image(): Não foi possível ler a imagem\n");
        return 1;
    }

    // Aplica o filtro de média
    blurred = vc_image_new(image->width, image->height, 1, 255);
    if (blurred == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem borrada\n");
        vc_image_free(image);
        return 1;
    }
    vc_gray_lowpass_median_filter(image, blurred, 5); // Aplica o filtro de média com kernel de 5x5
    // Escreve a imagem resultante
    if (vc_write_image("barbara_processed.pgm", blurred) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem\n");
        vc_image_free(image);
        vc_image_free(blurred);
        return 1;
    }

    // Limpa a memória
    vc_image_free(image);
    vc_image_free(blurred);


    printf("Processamento concluído. Pressione qualquer tecla para sair\n");

    return 0;
}
