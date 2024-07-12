#include <stdio.h>
#include "vc.h"
#include <stdlib.h>
#include <string.h>

int main(void)
{
    IVC* image;
    IVC* gray;
    IVC* binary;
    IVC* dilated;
    IVC* eroded;
    IVC* labeled;
    IVC* output_image;
    OVC* blobs;
    int nlabels;

    // Lê a imagem
    image = vc_read_image("resis.ppm");
    if (image == NULL) {
        printf("ERRO -> vc_read_image(): Não foi possível ler a imagem\n");
        return 1;
    }

    // Converte frame para escala de cinza
    gray = vc_image_new(image->width, image->height, 1, 255);
    if (gray == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem em escala de cinza\n");
        vc_image_free(image);
        return 1;
    }
    vc_rgb_to_gray(image, gray);

    // Escreve a imagem em escala de cinza
    if (vc_write_image("gray_image.pgm", gray) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem em escala de cinza\n");
        vc_image_free(image);
        vc_image_free(gray);
        return 1;
    }

    // Binariza a imagem em escala de cinza
    binary = vc_image_new(gray->width, gray->height, 1, 255);
    if (binary == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem binarizada\n");
        vc_image_free(image);
        vc_image_free(gray);
        return 1;
    }
    vc_gray_to_binary(gray, binary, 160); // Ajuste o valor do limiar conforme necessário

    // Escreve a imagem binarizada
    if (vc_write_image("binary_image.pbm", binary) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem binarizada\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        return 1;
    }

    // Aplica dilatação
    dilated = vc_image_new(binary->width, binary->height, binary->channels, 255);
    if (dilated == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem dilatada\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        return 1;
    }
    vc_binary_dilate(binary, dilated, 30);

    // Escreve a imagem dilatada
    if (vc_write_image("dilated_image.pbm", dilated) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem dilatada\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        return 1;
    }

    // Aplica erosão
    eroded = vc_image_new(dilated->width, dilated->height, dilated->channels, 255);
    if (eroded == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem erodida\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        return 1;
    }
    vc_binary_erode(dilated, eroded, 30);

    // Escreve a imagem erodida
    if (vc_write_image("eroded_image.pbm", eroded) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem erodida\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        vc_image_free(eroded);
        return 1;
    }

    // Cria uma imagem para armazenar os rótulos
    labeled = vc_image_new(eroded->width, eroded->height, eroded->channels, 255);
    if (labeled == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem rotulada\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        vc_image_free(eroded);
        return 1;
    }

    // Detecta e rotula os blobs na imagem erodida
    blobs = vc_binary_blob_labelling(eroded, labeled, &nlabels);
    if (blobs == NULL) {
        printf("ERRO -> vc_binary_blob_labelling(): Não foi possível detectar blobs\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        vc_image_free(eroded);
        vc_image_free(labeled);
        return 1;
    }

    printf("Número de blobs detectados: %d\n", nlabels);

    // Cria uma imagem colorida para a saída
    output_image = vc_image_new(labeled->width, labeled->height, image->channels, 255);
    if (output_image == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem de saída\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        vc_image_free(eroded);
        vc_image_free(labeled);
        free(blobs);
        return 1;
    }

    // Sobrepõe a imagem original nos pixels rotulados
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            int index_labeled = y * labeled->bytesperline + x * labeled->channels;
            int index_output = y * output_image->bytesperline + x * output_image->channels;
            int index_image = y * image->bytesperline + x * image->channels;

            if (labeled->data[index_labeled] > 0) { // Se o pixel for parte de um blob
                output_image->data[index_output] = 255;     // R
                output_image->data[index_output + 1] = 255; // G
                output_image->data[index_output + 2] = 255; // B
            } else {
                output_image->data[index_output] = image->data[index_image];         // R
                output_image->data[index_output + 1] = image->data[index_image + 1]; // G
                output_image->data[index_output + 2] = image->data[index_image + 2]; // B
            }
        }
    }

    // Processa a imagem de saída com blobs detectados
    for (int i = 0; i < nlabels; i++) {
        printf("Blob %d: área = %d, perímetro = %d\n", i + 1, blobs[i].area, blobs[i].perimeter);

        // Exemplo de como desenhar um retângulo ao redor de cada blob
        for (int y = blobs[i].y; y < blobs[i].y + blobs[i].height; y++) {
            for (int x = blobs[i].x; x < blobs[i].x + blobs[i].width; x++) {
                int index_output = y * output_image->bytesperline + x * output_image->channels;

                if (x == blobs[i].x || x == blobs[i].x + blobs[i].width - 1 ||
                    y == blobs[i].y || y == blobs[i].y + blobs[i].height - 1) {
                    output_image->data[index_output] = 255;     // R
                    output_image->data[index_output + 1] = 0;   // G
                    output_image->data[index_output + 2] = 0;   // B
                }
            }
        }
    }

    // Escreve a imagem resultante
    if (vc_write_image("resis_processed.ppm", output_image) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem\n");
        vc_image_free(image);
        vc_image_free(gray);
        vc_image_free(binary);
        vc_image_free(dilated);
        vc_image_free(eroded);
        vc_image_free(labeled);
        vc_image_free(output_image);
        free(blobs);
        return 1;
    }

    // Limpa a memória
    vc_image_free(image);
    vc_image_free(gray);
    vc_image_free(binary);
    vc_image_free(dilated);
    vc_image_free(eroded);
    vc_image_free(labeled);
    vc_image_free(output_image);
    free(blobs);

    printf("Processamento concluído. Pressione qualquer tecla para sair");

    return 0;
}
