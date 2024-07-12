#include <stdio.h>
#include "vc.h" 

int main() {
    // Carrega a imagem em tons de cinza
    IVC* src = vc_read_image("Import/coins.pgm");
    if (src == NULL) {
        printf("Erro ao carregar a imagem de entrada.\n");
        return -1;
    }

    // Cria uma nova imagem binarizada com as mesmas dimensões da imagem original
    IVC* dst = vc_image_new(src->width, src->height, 1, src->channels);
    if (dst == NULL) {
        printf("Erro ao criar a imagem binarizada.\n");
        vc_image_free(src);
        return -1;
    }

    // Aplica a binarização usando a média global como limiar
    int result = vc_gray_to_binary_global_mean(src, dst);
    if (result == 0) {
        printf("Erro ao aplicar a binarização.\n");
        vc_image_free(src);
        vc_image_free(dst);
        return -1;
    }

    // Salva a imagem binarizada
    vc_write_image("Export/05_04/output_binary_global_mean.pbm", dst);

    // Libera a memória alocada para as imagens
    vc_image_free(src);
    vc_image_free(dst);

    return 0;
}

