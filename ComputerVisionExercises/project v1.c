#include <stdio.h>
#include <stdlib.h>
#include "vc.h"

int main(void)
{
    IVC* image;
    IVC* temp_image;

    // Lê a imagem
    image = vc_read_image("resis.ppm");
    if (image == NULL) {
        printf("ERRO -> vc_read_image(): Não foi possível ler a imagem\n");
        return 1;
    }

    // Converte frame para escala de cinza
    IVC* gray = vc_image_new(image->width, image->height, 1, 255);
    if (gray == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem em escala de cinza\n");
        vc_image_free(image);
        return 1;
    }
    vc_rgb_to_gray(image, gray);

    // Detecção de contornos usando o operador de Prewitt
    IVC* edges = vc_image_new(gray->width, gray->height, 1, 255);
    if (edges == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem para detecção de contornos\n");
        vc_image_free(image);
        vc_image_free(gray);
        return 1;
    }
    vc_gray_edge_prewitt(gray, edges, 100.0); // Ajuste o valor do limiar (th) conforme necessário

    // Cria uma imagem colorida para a saída com contornos sobrepostos
    IVC* output_image = vc_image_new(image->width, image->height, image->channels, image->levels);
    if (output_image == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem de saída\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(edges);
        return 1;
    }

    // Copia a imagem original para a imagem de saída
    memcpy(output_image->data, image->data, image->bytesperline * image->height);

    // Sobrepõe os contornos na imagem original
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int index = y * edges->bytesperline + x * edges->channels;
            if (edges->data[index] > 0) { // Se o pixel for um contorno
                output_image->data[y * output_image->bytesperline + x * output_image->channels + 0] = 255; // R
                output_image->data[y * output_image->bytesperline + x * output_image->channels + 1] = 0;   // G
                output_image->data[y * output_image->bytesperline + x * output_image->channels + 2] = 0;   // B
            }
        }
    }

    // Escreve a imagem resultante
    if (vc_write_image("resis_processed.ppm", output_image) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(edges);
        vc_image_free(output_image);
        return 1;
    }

    // Limpa a memória
    vc_image_free(image);
    vc_image_free(gray);
    vc_image_free(edges);
    vc_image_free(output_image);

    printf("Processamento concluído. Pressione qualquer tecla para sair...\n");
    getchar();

    return 0;
}
