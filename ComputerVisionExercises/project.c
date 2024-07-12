#include <stdio.h>
#include "vc.h"
#include <stdlib.h>
#include <string.h>

int main(void)
{
    IVC* image;
    IVC* hsv;
    IVC* segmented;
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

    // Exporta a imagem lida
    if (vc_write_image("resis_read.ppm", image) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem lida\n");
        vc_image_free(image);
        return 1;
    }

    // Converte a imagem para HSV
    hsv = vc_image_new(image->width, image->height, image->channels, 255);
    if (hsv == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem HSV\n");
        vc_image_free(image);
        return 1;
    }
    vc_rgb_to_hsv(image, hsv);

    // Exporta a imagem HSV
    if (vc_write_image("resis_hsv.ppm", hsv) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem HSV\n");
        vc_image_free(image);
        vc_image_free(hsv);
        return 1;
    }

    // Cria imagem para a segmentação
    segmented = vc_image_new(hsv->width, hsv->height, 1, 255);
    if (segmented == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem segmentada\n");
        vc_image_free(image);
        vc_image_free(hsv);
        return 1;
    }

    // Segmenta a cor amarela
    unsigned char minHue = 20, maxHue = 30;
    unsigned char minSaturation = 100, maxSaturation = 255;
    unsigned char minValue = 100, maxValue = 255;

    vc_hsv_segmentation(hsv, segmented, minHue, maxHue, minSaturation, maxSaturation, minValue, maxValue);

    // Exporta a imagem segmentada
    if (vc_write_image("resis_segmented.ppm", segmented) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem segmentada\n");
        vc_image_free(image);
        vc_image_free(hsv);
        vc_image_free(segmented);
        return 1;
    }

    // Processa a segmentação (rotulação e output)
    // Cria uma imagem para armazenar os rótulos
    labeled = vc_image_new(segmented->width, segmented->height, segmented->channels, 255);
    if (labeled == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem rotulada\n");
        vc_image_free(image);
        vc_image_free(hsv);
        vc_image_free(segmented);
        return 1;
    }

    // Detecta e rotula os blobs na imagem segmentada
    blobs = vc_binary_blob_labelling(segmented, labeled, &nlabels);
    if (blobs == NULL) {
        printf("ERRO -> vc_binary_blob_labelling(): Não foi possível detectar blobs\n");
        vc_image_free(image);
        vc_image_free(hsv);
        vc_image_free(segmented);
        vc_image_free(labeled);
        return 1;
    }

    // Exporta a imagem rotulada
    if (vc_write_image("resis_labeled.ppm", labeled) == 0) {
        printf("ERRO -> vc_write_image(): Não foi possível escrever a imagem rotulada\n");
        vc_image_free(image);
        vc_image_free(hsv);
        vc_image_free(segmented);
        vc_image_free(labeled);
        free(blobs);
        return 1;
    }

    printf("Número de blobs detectados: %d\n", nlabels);

    // Cria uma imagem colorida para a saída
    output_image = vc_image_new(labeled->width, labeled->height, image->channels, 255);
    if (output_image == NULL) {
        printf("ERRO -> vc_image_new(): Não foi possível criar imagem de saída\n");
        vc_image_free(image);
        vc_image_free(hsv);
        vc_image_free(segmented);
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

    // Processa a imagem de saída com blobs detectados e desenha retângulos ao redor dos blobs
    for (int i = 0; i < nlabels; i++) {
        printf("Blob %d: área = %d, perímetro = %d\n", i + 1, blobs[i].area, blobs[i].perimeter);

        // Desenhar um retângulo ao redor de cada blob
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
        vc_image_free(hsv);
        vc_image_free(segmented);
        vc_image_free(labeled);
        vc_image_free(output_image);
        free(blobs);
        return 1;
    }

    // Limpa a memória
    vc_image_free(image);
    vc_image_free(hsv);
    vc_image_free(segmented);
    vc_image_free(labeled);
    vc_image_free(output_image);
    free(blobs);

    printf("Processamento concluído. Pressione qualquer tecla para sair");

    return 0;
}
