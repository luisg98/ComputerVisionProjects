#include <stdio.h>
#include "vc.h"  

int main() {
    IVC *image;

    // Carrega a imagem em escala de cinza do arquivo
    image = vc_read_image("../Import/Images/FLIR/flir-01.pgm");
    if (image == NULL) {
        fprintf(stderr, "Erro ao carregar a imagem!\n");
        return -1;
    }

    // Verifica se a imagem está em escala de cinza
    if (image->channels != 1) {
        fprintf(stderr, "A imagem não está em escala de cinza!\n");
        vc_image_free(image);
        return -1;
    }

    // Aplica a função para inverter a imagem
    if (vc_gray_negative(image) == 0) {
        fprintf(stderr, "Erro ao inverter a imagem!\n");
        vc_image_free(image);
        return -1;
    }

    // Salva a imagem resultante
    if (vc_write_image("imagem_invertida.pgm", image) != 1) {
        fprintf(stderr, "Erro ao guardar a imagem invertida!\n");
        vc_image_free(image);
        return -1;
    }

    // Libera a memória da imagem
    vc_image_free(image);

    printf("A imagem foi invertida e guardada com sucesso.\n");
    return 0;
}
