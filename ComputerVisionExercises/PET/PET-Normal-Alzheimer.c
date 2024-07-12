#include <stdio.h>
#include "vc.h"

#define HSV_RED_MIN_1 0
#define HSV_RED_MAX_1 45
#define HSV_RED_MIN_2 291
#define HSV_RED_MAX_2 360
#define HSV_YELLOW_MIN 46
#define HSV_YELLOW_MAX 70
#define HSV_GREEN_MIN 71
#define HSV_GREEN_MAX 160
#define HSV_BLUE_MIN 161
#define HSV_BLUE_MAX 290
#define HSV_SAT_MIN 0
#define HSV_SAT_MAX 255
#define HSV_VAL_MIN 0
#define HSV_VAL_MAX 255


int main(void) {
    IVC *image, *segmented_image;
    int count_red_0_45, count_red_291_360, count_yellow, count_green, count_blue, total_pixels;
    float percent_red, percent_yellow, percent_green, percent_blue;

    // Read the image
    image = vc_read_image("Import/Imagens - PETAlzheimer/PET-Alzheimer.ppm");
    if (image == NULL) {
        printf("Error reading the image.\n");
        return 1;
    }

    // Convertendo para o espaço de cor HSV
    vc_rgb_to_hsv(image, image);

    // Criando uma nova imagem para armazenar a segmentação
    segmented_image = vc_image_new(image->width, image->height, image->channels, image->levels);
    if (segmented_image == NULL) {
        printf("Error creating segmented image.\n");
        vc_image_free(image);
        return 1;
    }

    // Segmentação por cor
    vc_hsv_segmentation(image, segmented_image, HSV_RED_MIN_1, HSV_RED_MAX_1, HSV_SAT_MIN, HSV_VAL_MIN, HSV_SAT_MAX, HSV_VAL_MAX); // Vermelho (0-45)
    count_red_0_45 = vc_binary_count(segmented_image);

    vc_hsv_segmentation(image, segmented_image, HSV_RED_MIN_2, HSV_RED_MAX_2, HSV_SAT_MIN, HSV_VAL_MIN, HSV_SAT_MAX, HSV_VAL_MAX); // Vermelho (291-360)
    count_red_291_360 = vc_binary_count(segmented_image);

    vc_hsv_segmentation(image, segmented_image, HSV_YELLOW_MIN, HSV_YELLOW_MAX, HSV_SAT_MIN, HSV_VAL_MIN, HSV_SAT_MAX, HSV_VAL_MAX); // Amarelo
    count_yellow = vc_binary_count(segmented_image);

    vc_hsv_segmentation(image, segmented_image, HSV_GREEN_MIN, HSV_GREEN_MAX, HSV_SAT_MIN, HSV_VAL_MIN, HSV_SAT_MAX, HSV_VAL_MAX); // Verde
    count_green = vc_binary_count(segmented_image);

    vc_hsv_segmentation(image, segmented_image, HSV_BLUE_MIN, HSV_BLUE_MAX, HSV_SAT_MIN, HSV_VAL_MIN, HSV_SAT_MAX, HSV_VAL_MAX); // Azul
    count_blue = vc_binary_count(segmented_image);

    // Calculando o total de pixels
    total_pixels = image->width * image->height;

    // Calculando as porcentagens
    percent_red = ((float)(count_red_0_45 + count_red_291_360) / total_pixels) * 100;
    percent_yellow = ((float)count_yellow / total_pixels) * 100;
    percent_green = ((float)count_green / total_pixels) * 100;
    percent_blue = ((float)count_blue / total_pixels) * 100;

    // Exibindo os resultados
    printf("Percentagem de Área do Cérebro com atividade vermelha: %.2f%%\n", percent_red);
    printf("Percentagem de Área do Cérebro com atividade amarela: %.2f%%\n", percent_yellow);
    printf("Percentagem de Área do Cérebro com atividade verde: %.2f%%\n", percent_green);
    printf("Percentagem de Área do Cérebro com atividade azul: %.2f%%\n", percent_blue);

    // Liberando memória
    vc_image_free(image);
    vc_image_free(segmented_image);

    printf("Pressione qualquer tecla para sair...\n");
    getchar();

    return 0;
}
