#include <stdio.h>
#include "vc.h"  

int main() {
    IVC *src, *dst;

    // Carrega a imagem de origem em escala de cinza
    src = vc_read_image("Import/barbara2.pgm");
    if (src == NULL) {
        fprintf(stderr, "Erro ao carregar a imagem de origem!\n");
        return -1;
    }

    // Verifica se a imagem de origem é em escala de cinza
    if (src->channels != 1) {
        fprintf(stderr, "A imagem de origem não está em escala de cinza!\n");
        vc_image_free(src);
        return -1;
    }

    // Cria uma imagem de destino com as mesmas propriedades da imagem de origem
    dst = vc_image_new(src->width, src->height, src->channels, src->levels);
    if (dst == NULL) {
        fprintf(stderr, "Erro ao criar a imagem de destino!\n");
        vc_image_free(src);
        return -1;
    }

    // Processa a imagem de origem para mostrar o histograma na imagem de destino
    if (vc_gray_histogram_show(src, dst) == 0) {
        fprintf(stderr, "Erro ao processar o histograma!\n");
        vc_image_free(src);
        vc_image_free(dst);
        return -1;
    }

    // Guarda a imagem de destino
    if (vc_write_image("histograma_resultante.pgm", dst) != 1) {
        fprintf(stderr, "Erro ao salvar a imagem de destino!\n");
        vc_image_free(src);
        vc_image_free(dst);
        return -1;
    }

  
    vc_image_free(src);
    vc_image_free(dst);

    printf("Histograma processado e salvo com sucesso!\n");

    return 0;
}
