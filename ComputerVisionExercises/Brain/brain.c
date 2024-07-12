#include <stdio.h>
#include "../vc.h"

int main(void)
{
    IVC* image;
    IVC* temp_image;

    // Lê a imagem
    image = vc_read_image("../Import/brain.pgm");
    if (image == NULL)
    {
        printf("ERRO -> vc_read_image():\n\tFile not found!\n");
        getchar();
        return 0;
    }

    // Cria uma imagem temporária para armazenar os resultados intermediários
    temp_image = vc_image_new(image->width, image->height, image->channels, image->levels);
    if (temp_image == NULL)
    {
        printf("ERRO -> vc_image_new():\n\tNão foi possível criar a imagem temporária!\n");
        vc_image_free(image);
        getchar();
        return 0;
    }

    // Converte a imagem em escala de cinza para binária usando um limiar
    if (vc_gray_to_binary(image, temp_image, 70) == 0) // Alterado o limiar para 120
    {
        printf("ERRO -> vc_gray_to_binary():\n");
        vc_image_free(image);
        vc_image_free(temp_image);
        getchar();
        return 0;
    }

    // Aplica a erosão binária (usando um kernel de tamanho 3)
    if (vc_binary_erode(temp_image, image, 11) == 0) 
    {
        printf("ERRO -> vc_binary_erode():\n");
        vc_image_free(image);
        vc_image_free(temp_image);
        getchar();
        return 0;
    }

    

    // Aplica a dilatação binária (usando um kernel de tamanho 5)
    if (vc_binary_dilate(image, temp_image, 25) == 0) 
    {
        printf("ERRO -> vc_binary_dilate():\n");
        vc_image_free(image);
        vc_image_free(temp_image);
        getchar();
        return 0;
    }

    // Escreve a imagem resultante
    if (vc_write_image("brain_processed.pbm", temp_image) == 0) // Renomeada a imagem resultante
    {
        printf("ERRO -> vc_write_image():\n");
        vc_image_free(image);
        vc_image_free(temp_image);
        getchar();
        return 0;
    }


    // Limpa a memória
    vc_image_free(image);
    vc_image_free(temp_image);

    printf("Processamento concluído. Pressione qualquer tecla para sair...\n");
    getchar();

    return 0;
}
