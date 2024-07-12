// ====================================================================================
// Instituto Politécnico do Cávado e do Ave
// Engenharia de Sistemas Informáticos
// Visão por Computador
// Ano Letivo 2023/2024
// ====================================================================================
// Autor: Luis Goncalves - a18851@alunos.ipca.pt
// ====================================================================================

// Prevenção contra avisos de funções não seguras no MSVC++ (fopen, sscanf, etc...)
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include "vc.h"
#include <math.h>
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Funções para Alocação e Liberação de Memória de Imagens
// =======================================================================

/**
 * @brief Aloca memória para uma nova imagem.
 * @author lugon
 * @param width largura da imagem.
 * @param height altura da imagem.
 * @param channels número de canais de cor da imagem.
 * @param levels níveis de intensidade de cor (normalmente 256 para imagens de 8 bits).
 * @return Retorna um ponteiro para a estrutura IVC que representa a nova imagem alocada na memória.
 */
IVC* vc_image_new(int width, int height, int channels, int levels)
{
	IVC* image = (IVC*)malloc(sizeof(IVC));

	if (image == NULL) return NULL;
	if ((levels <= 0) || (levels > 255)) return NULL;

	image->width = width;
	image->height = height;
	image->channels = channels;
	image->levels = levels;
	image->bytesperline = image->width * image->channels;
	image->data = (unsigned char*)malloc(image->width * image->height * image->channels * sizeof(char));

	if (image->data == NULL)
	{
		return vc_image_free(image);
	}

	return image;
}

/**
 * @brief Liberta a memória alocada para uma estrutura IVC e seus dados.
 * @author lugon
 * @param image ponteiro para a estrutura IVC a ser liberada.
 * @return Retorna NULL após liberar a memória.
 */
IVC* vc_image_free(IVC* image)
{
	// Verifica se o ponteiro da imagem não é nulo
	if (image != NULL)
	{
		// Verifica se o ponteiro dos dados da imagem não é nulo
		if (image->data != NULL)
		{
			// Libera a memória alocada para os dados da imagem
			free(image->data);
			image->data = NULL; // Define o ponteiro de dados como NULL para evitar referências inválidas
		}

		// Libera a memória alocada para a estrutura de imagem
		free(image);
		image = NULL; // Define o ponteiro de imagem como NULL para evitar referências inválidas
	}

	// Retorna NULL após liberar a memória
	return image;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/**
 * @brief Obtém um token do arquivo NetPBM, ignorando comentários e espaços em branco.
 * @param file ponteiro para o arquivo a ser lido.
 * @param tok ponteiro para onde o token será armazenado.
 * @param len tamanho máximo do token.
 * @return Retorna um ponteiro para o token.
 */
char* netpbm_get_token(FILE* file, char* tok, int len)
{
	char* t;
	int c;

	for (;;)
	{
		while (isspace(c = getc(file)));
		if (c != '#') break;
		do c = getc(file);
		while ((c != '\n') && (c != EOF));
		if (c == EOF) break;
	}

	t = tok;

	if (c != EOF)
	{
		do
		{
			*t++ = c;
			c = getc(file);
		} while ((!isspace(c)) && (c != '#') && (c != EOF) && (t - tok < len - 1));

		if (c == '#') ungetc(c, file);
	}

	*t = 0;

	return tok;
}

/**
 * @brief Converte dados de imagem de unsigned char para bits.
 * @param datauchar ponteiro para os dados da imagem em unsigned char.
 * @param databit ponteiro para os dados da imagem em bits.
 * @param width largura da imagem.
 * @param height altura da imagem.
 * @return Retorna o número total de bytes processados.
 */
long int unsigned_char_to_bit(unsigned char* datauchar, unsigned char* databit, int width, int height)
{
	int x, y;
	int countbits;
	long int pos, counttotalbytes;
	unsigned char* p = databit;

	*p = 0;
	countbits = 1;
	counttotalbytes = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//*p |= (datauchar[pos] != 0) << (8 - countbits);

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				*p |= (datauchar[pos] == 0) << (8 - countbits);

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				*p = 0;
				countbits = 1;
				counttotalbytes++;
			}
		}
	}

	return counttotalbytes;
}

/**
 * @brief Converte dados de imagem de bits para unsigned char.
 * @param databit ponteiro para os dados da imagem em bits.
 * @param datauchar ponteiro para os dados da imagem em unsigned char.
 * @param width largura da imagem.
 * @param height altura da imagem.
 */
void bit_to_unsigned_char(unsigned char* databit, unsigned char* datauchar, int width, int height)
{
	int x, y;
	int countbits;
	long int pos;
	unsigned char* p = databit;

	countbits = 1;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = width * y + x;

			if (countbits <= 8)
			{
				// Numa imagem PBM:
				// 1 = Preto
				// 0 = Branco
				//datauchar[pos] = (*p & (1 << (8 - countbits))) ? 1 : 0;

				// Na nossa imagem:
				// 1 = Branco
				// 0 = Preto
				datauchar[pos] = (*p & (1 << (8 - countbits))) ? 0 : 1;

				countbits++;
			}
			if ((countbits > 8) || (x == width - 1))
			{
				p++;
				countbits = 1;
			}
		}
	}
}

/**
 * @brief Lê uma imagem NetPBM (PBM, PGM, PPM) de um arquivo.
 * @param filename nome do arquivo a ser lido.
 * @return Retorna um ponteiro para a estrutura IVC que representa a imagem lida, ou NULL em caso de erro.
 */
IVC* vc_read_image(char* filename)
{
	FILE* file = NULL;
	IVC* image = NULL;
	unsigned char* tmp;
	char tok[20];
	long int size, sizeofbinarydata;
	int width, height, channels;
	int levels = 255;
	int v;

	// Abre o ficheiro
	if ((file = fopen(filename, "rb")) != NULL)
	{
		// Efectua a leitura do header
		netpbm_get_token(file, tok, sizeof(tok));

		if (strcmp(tok, "P4") == 0) { channels = 1; levels = 1; }	// Se PBM (Binary [0,1])
		else if (strcmp(tok, "P5") == 0) channels = 1;				// Se PGM (Gray [0,MAX(level,255)])
		else if (strcmp(tok, "P6") == 0) channels = 3;				// Se PPM (RGB [0,MAX(level,255)])
		else
		{
#ifdef VC_DEBUG
			printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM, PGM or PPM file.\n\tBad magic number!\n");
#endif

			fclose(file);
			return NULL;
		}

		if (levels == 1) // PBM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PBM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			if ((v = fread(tmp, sizeof(unsigned char), sizeofbinarydata, file)) != sizeofbinarydata)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				free(tmp);
				return NULL;
			}

			bit_to_unsigned_char(tmp, image->data, image->width, image->height);

			free(tmp);
		}
		else // PGM ou PPM
		{
			if (sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &width) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &height) != 1 ||
				sscanf(netpbm_get_token(file, tok, sizeof(tok)), "%d", &levels) != 1 || levels <= 0 || levels > 255)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tFile is not a valid PGM or PPM file.\n\tBad size!\n");
#endif

				fclose(file);
				return NULL;
			}

			// Aloca memória para imagem
			image = vc_image_new(width, height, channels, levels);
			if (image == NULL) return NULL;

#ifdef VC_DEBUG
			printf("\nchannels=%d w=%d h=%d levels=%d\n", image->channels, image->width, image->height, levels);
#endif

			size = image->width * image->height * image->channels;

			if ((v = fread(image->data, sizeof(unsigned char), size, file)) != size)
			{
#ifdef VC_DEBUG
				printf("ERROR -> vc_read_image():\n\tPremature EOF on file.\n");
#endif

				vc_image_free(image);
				fclose(file);
				return NULL;
			}
		}

		fclose(file);
	}
	else
	{
#ifdef VC_DEBUG
		printf("ERROR -> vc_read_image():\n\tFile not found.\n");
#endif
	}

	return image;
}

/**
 * @brief Escreve uma imagem NetPBM (PBM, PGM, PPM) em um arquivo.
 * @param filename nome do arquivo a ser escrito.
 * @param image ponteiro para a estrutura IVC que representa a imagem a ser escrita.
 * @return Retorna 1 em caso de sucesso, 0 em caso de erro.
 */
int vc_write_image(char* filename, IVC* image)
{
	FILE* file = NULL;
	unsigned char* tmp;
	long int totalbytes, sizeofbinarydata;

	if (image == NULL) return 0;

	if ((file = fopen(filename, "wb")) != NULL)
	{
		if (image->levels == 1)
		{
			sizeofbinarydata = (image->width / 8 + ((image->width % 8) ? 1 : 0)) * image->height + 1;
			tmp = (unsigned char*)malloc(sizeofbinarydata);
			if (tmp == NULL) return 0;

			fprintf(file, "%s %d %d\n", "P4", image->width, image->height);

			totalbytes = unsigned_char_to_bit(image->data, tmp, image->width, image->height);
			printf("Total = %ld\n", totalbytes);
			if (fwrite(tmp, sizeof(unsigned char), totalbytes, file) != totalbytes)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				free(tmp);
				return 0;
			}

			free(tmp);
		}
		else
		{
			fprintf(file, "%s %d %d 255\n", (image->channels == 1) ? "P5" : "P6", image->width, image->height);

			if (fwrite(image->data, image->bytesperline, image->height, file) != image->height)
			{
#ifdef VC_DEBUG
				fprintf(stderr, "ERROR -> vc_read_image():\n\tError writing PBM, PGM or PPM file.\n");
#endif

				fclose(file);
				return 0;
			}
		}

		fclose(file);

		return 1;
	}

	return 0;
}

/**
 * @brief Inverte os valores dos pixels de uma imagem em escala de cinza para produzir o negativo da imagem.
 * @author lugon
 * @param srcdst Ponteiro para a estrutura IVC que representa a imagem em escala de cinza.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a imagem foi invertida com sucesso, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_gray_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data; // Ponteiro para os dados da imagem
	int width = srcdst->width;  // Largura da imagem
	int height = srcdst->height; // Altura da imagem
	int bytesperline = srcdst->bytesperline; // Número de bytes por linha na imagem
	int channels = srcdst->channels; // Número de canais de cor na imagem, deve ser 1 para imagens em escala de cinza
	int x, y; // Variáveis para iterar sobre os pixels da imagem
	long int pos; // Posição do pixel na array de dados

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (srcdst->data == NULL))
		return 0; // Retorna 0 se a imagem tiver dimensões inválidas ou dados nulos

	if (channels != 1)
		return 0; // Retorna 0 se a imagem não for em escala de cinza

	// Processo de inverter a imagem em escala de cinza
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels; // Calcula a posição do pixel na array de dados
			data[pos] = 255 - data[pos]; // Inverte o pixel subtraindo seu valor de 255
		}
	}

	return 1; // Retorna 1 após a imagem ser invertida com sucesso
}

/**
 * @brief Segmenta uma imagem HSV com base em intervalos de Hue, Saturation e Value.
 * @author lugon
 * @param src ponteiro para a estrutura IVC que representa a imagem de entrada em HSV.
 * @param dst ponteiro para a estrutura IVC que representa a imagem de saída binarizada.
 * @param minHue valor mínimo de Hue.
 * @param maxHue valor máximo de Hue.
 * @param minSaturation valor mínimo de Saturation.
 * @param maxSaturation valor máximo de Saturation.
 * @param minValue valor mínimo de Value.
 * @param maxValue valor máximo de Value.
 * @return int Retorna 1 se a segmentação foi bem-sucedida, 0 caso contrário.
 */
int vc_hsv_segmentation(IVC* src, IVC* dst, unsigned char minHue, unsigned char maxHue, unsigned char minSaturation, unsigned char maxSaturation, unsigned char minValue, unsigned char maxValue)
{
	unsigned char* datadst = (unsigned char*)dst->data;
	int i, size;
	unsigned char h, s, v;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		h = src->data[i];
		s = src->data[i + 1];
		v = src->data[i + 2];

		if (h >= minHue && h <= maxHue && s >= minSaturation && s <= maxSaturation && v >= minValue && v <= maxValue)
		{
			datadst[i / channels] = 255;
		}
		else
		{
			datadst[i / channels] = 0;
		}
	}

	return 1;
}

/**
 * @brief Inverte os valores dos pixels de uma imagem RGB para produzir o negativo da imagem.
 * @author lugon
 * @param srcdst Ponteiro para a estrutura IVC que representa a imagem RGB.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a imagem foi invertida com sucesso, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_negative(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int with = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->bytesperline;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//inverter imagem RGB

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < with; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = 255 - data[pos];
			data[pos + 1] = 255 - data[pos + 1];
			data[pos + 2] = 255 - data[pos + 2];
		}
	}
	return 1;
}

/**
 * @brief Converte a componente vermelha (Red) de uma imagem RGB para escala de cinza.
 * @author lugon
 * @param srcdst Ponteiro para a estrutura IVC que representa a imagem RGB.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_get_red_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//Extrai a componente Red
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos + 1] = data[pos]; //green
			data[pos + 2] = data[pos]; //blue
		}
	}
	return 1;
}

/**
 * @brief Converte a componente verde (Green) de uma imagem RGB para escala de cinza.
 * @author lugon
 * @param srcdst Ponteiro para a estrutura IVC que representa a imagem RGB.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_get_green_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//Extrai a componente Green
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos + 1]; //red
			data[pos + 2] = data[pos + 1]; //blue
		}
	}

	return 1;
}

/**
 * @brief Converte a componente azul (Blue) de uma imagem RGB para escala de cinza.
 * @author lugon
 * @param srcdst Ponteiro para a estrutura IVC que representa a imagem RGB.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_get_blue_gray(IVC* srcdst)
{
	unsigned char* data = (unsigned char*)srcdst->data;
	int width = srcdst->width;
	int height = srcdst->height;
	int bytesperline = srcdst->width * srcdst->channels;
	int channels = srcdst->channels;
	int x, y;
	long int pos;

	//verificaçao de erros
	if ((srcdst->width <= 0) || (srcdst->height <= 0) || (srcdst->data == NULL)) return 0;
	if (channels != 3) return 0;

	//Extrai a componente Blue
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			data[pos] = data[pos + 2]; //red
			data[pos + 1] = data[pos + 2]; //green
		}
	}

	return 1;
}

/**
 * @brief Converte uma imagem RGB para escala de cinza.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem RGB de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem de saída em escala de cinza.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_to_gray(IVC* src, IVC* dst)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;
	int width = src->width;
	int height = src->height;
	int x, y;
	long int pos_src, pos_dst;
	float rf, gf, bf;

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 3) || (dst->channels != 1)) return 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;

			rf = (float)datasrc[pos_src];
			gf = (float)datasrc[pos_src + 1];
			bf = (float)datasrc[pos_src + 2];

			datadst[pos_dst] = (unsigned char)((rf * 0.299) + (gf * 0.587) + (bf * 0.114));
		}
	}
	return 1;
}

/**
 * @brief Converte uma imagem RGB para HSV.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem RGB de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem de saída em HSV.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_rgb_to_hsv(IVC* src, IVC* dst)
{
	unsigned char* data_src = (unsigned char*)src->data;
	unsigned char* data_dst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	float r, g, b, hue, saturation, value;
	float rgb_max, rgb_min;
	int i, size;

	// Verificação de erros
	if ((width <= 0) || (height <= 0) || (data_src == NULL) || (data_dst == NULL))
		return 0;
	if (channels != 3 || dst->channels != 3)
		return 0;

	size = width * height * channels;

	for (i = 0; i < size; i = i + channels)
	{
		r = (float)data_src[i];
		g = (float)data_src[i + 1];
		b = (float)data_src[i + 2];

		// Calcula valores máximo e mínimo dos canais de cor R, G e B
		rgb_max = (r > g ? (r > b ? r : b) : (g > b ? g : b));
		rgb_min = (r < g ? (r < b ? r : b) : (g < b ? g : b));

		// Value toma valores entre [0,255]
		value = rgb_max;
		if (value == 0.0f)
		{
			hue = 0.0f;
			saturation = 0.0f;
		}
		else
		{
			// Saturation toma valores entre [0,255]
			saturation = ((rgb_max - rgb_min) / rgb_max) * 255.0f;

			if (saturation == 0.0f)
			{
				hue = 0.0f;
			}
			else
			{
				// Hue toma valores entre [0,360]
				if ((rgb_max == r) && (g >= b))
				{
					hue = 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if ((rgb_max == r) && (b > g))
				{
					hue = 360.0f + 60.0f * (g - b) / (rgb_max - rgb_min);
				}
				else if (rgb_max == g)
				{
					hue = 120.0f + 60.0f * (b - r) / (rgb_max - rgb_min);
				}
				else /* rgb_max == b*/
				{
					hue = 240.0f + 60.0f * (r - g) / (rgb_max - rgb_min);
				}
			}
		}

		// Atribui valores entre [0,255] no destino
		data_dst[i] = (unsigned char)(hue / 360.0f * 255.0f);
		data_dst[i + 1] = (unsigned char)(saturation);
		data_dst[i + 2] = (unsigned char)(value);
	}

	return 1;
}

/**
 * @brief Converte uma imagem em escala de cinza para uma imagem RGB com coloração baseada em níveis de intensidade.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem em escala de cinza.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem RGB de saída.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_scale_gray_to_rgb(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y;
	long int pos_src, pos_dst;
	float gray;

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 3)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			gray = (float)datasrc[pos_src];

			if (gray < 64)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)(gray * 4);
				datadst[pos_dst + 2] = (unsigned char)255;
			}
			else if (gray < 128)
			{
				datadst[pos_dst] = (unsigned char)0;
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)(255 - (gray - 64) * 4);
			}
			else if (gray < 192)
			{
				datadst[pos_dst] = (unsigned char)((gray - 128) * 4);
				datadst[pos_dst + 1] = (unsigned char)255;
				datadst[pos_dst + 2] = (unsigned char)0;
			}
			else if (gray <= 255)
			{
				datadst[pos_dst] = (unsigned char)255;
				datadst[pos_dst + 1] = (unsigned char)(255 - (gray - 192) * 4);
				datadst[pos_dst + 2] = (unsigned char)0;
			}
		}
	}
	return 1;
}

/**
 * @brief Converte uma imagem em escala de cinza para binária usando um valor de threshold.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem em escala de cinza de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem binária de saída.
 * @param threshold Valor de threshold usado para a conversão.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_gray_to_binary(IVC* src, IVC* dst, int threshold)
{
	unsigned char* src_data = (unsigned char*)src->data;
	unsigned char* dst_data = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int src_bytesperline = src->bytesperline;
	int dst_bytesperline = dst->bytesperline;
	int src_channels = src->channels;
	int dst_channels = dst->channels;
	int x, y;
	long int src_pos, dst_pos;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (src_channels != 1 || dst_channels != 1) return 0;

	// Converter imagem Gray para Binária
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			src_pos = y * src_bytesperline + x * src_channels;
			dst_pos = y * dst_bytesperline + x * dst_channels;

			if (src_data[src_pos] < threshold) dst_data[dst_pos] = 0;
			else dst_data[dst_pos] = 255;
		}
	}

	// Definir o número de níveis da imagem de destino para 2 (binária)
	dst->levels = 2;

	return 1;
}

/**
 * @brief Obtém o negativo de uma imagem binarizada.
 * @author lugon
 * @param image Ponteiro para a estrutura IVC que representa a imagem binarizada.
 * Esta estrutura deve ser pré-alocada e conter dados válidos de imagem.
 */
void vc_binary_get_negative(IVC* image)
{
	// Obtém as informações da imagem binarizada
	unsigned char* data = (unsigned char*)image->data;
	int bytesperline = image->bytesperline;
	int channels = image->channels;
	int width = image->width;
	int height = image->height;
	long int pos;

	// Aplica o negativo invertendo os valores dos pixels
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			pos = y * bytesperline + x * channels;
			if (data[pos] == 0)
			{
				data[pos] = 255; // Troca pixel preto por branco
			}
			else
			{
				data[pos] = 0; // Troca pixel branco por preto
			}
		}
	}
}

/**
 * @brief Converte uma imagem em escala de cinza para binária usando a média global como valor de threshold.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem em escala de cinza de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem binária de saída.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst)
{
	// Informações da imagem de origem
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	// Informações da imagem de destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// Medidas
	int width = src->width;
	int height = src->height;

	// Auxiliares
	long int soma = 0;
	float media;
	int x, y, pos_src, pos_dst;

	// Verificações de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	// Calcula a média global
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			soma += datasrc[pos_src];
		}
	}
	media = (float)soma / (height * width);

	// Aplica a binarização
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;
			pos_dst = y * bytesperline_dst + x * channels_dst;
			if (datasrc[pos_src] < media)
			{
				datadst[pos_dst] = 0; // Preto
			}
			else
			{
				datadst[pos_dst] = 255; // Branco
			}
		}
	}
	return 1;
}

/**
 * @brief Converte uma imagem em escala de cinza para binária usando o método Midpoint com um kernel.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem em escala de cinza de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem binária de saída.
 * @param kernel Tamanho do kernel a ser usado para a conversão.
 * @return int Retorna 1 se a conversão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_gray_to_binary_kernel_midpoint(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize;
	long int pos_src, pos_dst, pos_src2;
	float treshold, min, max;

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			min = 255;
			max = 0;
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{
						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posicao da source
						//pos_dst = y2 * bytesperline_dst + x2 * channels_dst;//posicao destino
						if (max <= (float)datasrc[pos_src2])
						{
							max = (float)datasrc[pos_src2];
						}
						if (min >= (float)datasrc[pos_src2])
						{
							min = (float)datasrc[pos_src2];
						}
					}
					else datadst[pos_dst] = 0;
				}
			}
			treshold = (min + max) * (0.5);//mid point 
			if ((float)datasrc[pos_src] < treshold) datadst[pos_dst] = 0;
			else datadst[pos_dst] = 1;
		}
	}
	return 1;
}

/**
 * @brief Aplica a dilatação binária em uma imagem binária de entrada.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem binária de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem binária de saída.
 * @param kernel Tamanho do kernel a ser usado para a dilatação.
 * @return int Retorna 1 se a dilatação foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_binary_dilate(IVC* src, IVC* dst, int kernel)
{
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;

	int bytesperline_dst = dst->width * dst->channels;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((dst->width <= 0) || (dst->height <= 0))
		return 0;
	if ((src->channels != 1) || (dst->channels != 1))
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos_dst = y * bytesperline_dst + x * channels;
			int pixel = 0;

			for (int yy = -kernel / 2; yy <= kernel / 2; yy++)
			{
				for (int xx = -kernel / 2; xx <= kernel / 2; xx++)
				{
					if (y + yy >= 0 && y + yy < height && x + xx >= 0 && x + xx < width)
					{
						int pos_src = (y + yy) * bytesperline + (x + xx) * channels;
						if (src->data[pos_src] == 255)
						{
							pixel = 255;
						}
					}
				}
			}
			dst->data[pos_dst] = pixel;
		}
	}
	return 1;
}

/**
 * @brief Aplica a erosão binária em uma imagem binária de entrada.
 * @author lugon
 * @param src Ponteiro para a estrutura IVC que representa a imagem binária de entrada.
 * @param dst Ponteiro para a estrutura IVC que representa a imagem binária de saída.
 * @param kernel Tamanho do kernel a ser usado para a erosão.
 * @return int Retorna 1 se a erosão foi bem-sucedida, 0 se houve um erro, como dimensões inválidas ou formato de cor incorreto.
 */
int vc_binary_erode(IVC* src, IVC* dst, int kernel)
{
	int width = src->width;
	int height = src->height;
	int bytesperline = src->width * src->channels;
	int channels = src->channels;

	int bytesperline_dst = dst->width * dst->channels;

	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL))
		return 0;
	if ((dst->width <= 0) || (dst->height <= 0))
		return 0;
	if ((src->channels != 1) || (dst->channels != 1))
		return 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			int pos_dst = y * bytesperline_dst + x * channels;
			int pixel = 255;

			for (int yy = -kernel / 2; yy <= kernel / 2; yy++)
			{
				for (int xx = -kernel / 2; xx <= kernel / 2; xx++)
				{
					if (y + yy >= 0 && y + yy < height && x + xx >= 0 && x + xx < width)
					{
						int pos_src = (y + yy) * bytesperline + (x + xx) * channels;
						if (src->data[pos_src] == 0)
						{
							pixel = 0;
						}
					}
				}
			}
			dst->data[pos_dst] = pixel;
		}
	}
	return 1;
}

/**
 * @brief Calcula o histograma de uma imagem em escala de cinza e mostra o resultado em outra imagem.
 * @author lugon
 * @param src Imagem de origem em escala de cinza, de onde o histograma será calculado.
 * @param dst Imagem de destino onde o histograma será desenhado.
 * @return int Retorna 1 se o histograma foi processado e desenhado com sucesso, caso contrário retorna 0.
 */
int vc_gray_histogram_show(IVC* src, IVC* dst)
{
	// Informações da imagem de origem
	unsigned char* datasrc = (unsigned char*)src->data; // Dados da imagem de origem
	int bytesperline_src = src->width * src->channels;  // Bytes por linha na imagem de origem
	int channels_src = src->channels;                   // Número de canais de cor na imagem de origem

	// Informações da imagem de destino
	unsigned char* datadst = (unsigned char*)dst->data; // Dados da imagem de destino
	int bytesperline_dst = dst->width * dst->channels;  // Bytes por linha na imagem de destino
	int channels_dst = dst->channels;                   // Número de canais de cor na imagem de destino

	// Dimensões da imagem
	int width = src->width;
	int height = src->height;

	// Variáveis auxiliares
	int x, y, hist[256] = { 0 }, max = 0;
	long int pos_src, pos_dst;
	int total = height * width; // Total de pixels na imagem
	float pdf[256];             // Probability density function

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	// Calcula o histograma
	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src; // Calcula posição do pixel na imagem de origem
			hist[datasrc[pos_src]]++;                         // Incrementa o histograma no valor do pixel
		}
	}

	// Encontra o valor máximo no histograma
	for (int i = 0; i < 256; i++)
	{
		if (hist[i] > max) max = hist[i];
	}

	// Calcula o PDF normalizado para o máximo
	for (int i = 0; i < 256; i++)
	{
		pdf[i] = (float)hist[i] / max; // Normaliza cada valor de histograma
	}

	// Gera o gráfico do histograma na imagem de destino
	for (int i = 0, x = (width - 256) / 2; i < 256; i++, x++)
	{
		for (y = height - 1; y > height - 1 - pdf[i] * height; y--)
		{
			datadst[y * bytesperline_dst + x * channels_dst] = 255; // Desenha a barra do histograma
		}
	}

	// Desenha linhas de borda para o histograma
	for (y = 0; y < height - 1; y++)
	{
		datadst[y * bytesperline_dst + ((width - 256) / 2 - 1) * channels_dst] = 127; // Linha esquerda
		datadst[y * bytesperline_dst + ((width + 256) / 2 + 1) * channels_dst] = 127; // Linha direita
	}

	return 1; // Sucesso
}

/**
 * @brief Aplica a equalização de histograma em uma imagem em escala de cinza.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde o resultado da equalização será armazenado.
 * @return int Retorna 1 se a equalização foi processada com sucesso, caso contrário retorna 0.
 */
int vc_gray_histogram_equalization(IVC* src, IVC* dst)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, hist[256] = { 0 }, max = 0;
	long int pos_src, pos_dst;
	int total = height * width;//maximo de pixeis
	float pdf[256], min = 256, cdf[256];

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			//conta ocorrencias de cada valor de pixel
			hist[datasrc[pos_src]]++;
		}
	}
	for (int i = 0; i < 256; i++)
	{
		pdf[i] = (float)hist[i] / total;//total para equalização
		if ((pdf[i] < min) && (pdf[i] != 0)) min = pdf[i];//saber mínimo do pdf
	}

	for (int i = 0; i < 256; i++)
	{
		if (i != 0) cdf[i] = cdf[i - 1] + pdf[i];//acumulada
		else cdf[0] = pdf[0];
	}
	//--------------------------------------------------------
	//GERAR IMAGEM
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posicao da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posicao destino

			datadst[y * bytesperline_dst + x * channels_dst] = ((cdf[datasrc[pos_src]] - min) / (1 - min)) * (255 - 1);
		}
	}
	//---------------------------------------------------------
	return 1;
}

/**
 * @brief Etiquetagem de blobs em uma imagem binária.
 * @author lugon
 * @param src Imagem binária de entrada.
 * @param dst Imagem grayscale onde as etiquetas serão armazenadas.
 * @param nlabels Endereço de memória de uma variável onde será armazenado o número de etiquetas encontradas.
 * @return OVC* Retorna um array de estruturas de blobs (objetos), com respectivas etiquetas.
 */
OVC* vc_binary_blob_labelling(IVC* src, IVC* dst, int* nlabels)
{
	unsigned char* datasrc = (unsigned char*)src->data;
	unsigned char* datadst = (unsigned char*)dst->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, a, b;
	long int i, size;
	long int posX, posA, posB, posC, posD;
	int labeltable[256] = { 0 };
	int labelarea[256] = { 0 };
	int label = 1; // Etiqueta inicial.
	int num, tmplabel;
	OVC* blobs; // Apontador para array de blobs (objetos) que será retornado desta função.

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height) || (src->channels != dst->channels)) return NULL;
	if (channels != 1) return NULL;

	// Copia dados da imagem binária para imagem grayscale
	memcpy(datadst, datasrc, bytesperline * height);

	// Todos os pixels de plano de fundo devem obrigatoriamente ter valor 0
	// Todos os pixels de primeiro plano devem obrigatoriamente ter valor 255
	// Serão atribuídas etiquetas no intervalo [1,254]
	// Este algoritmo está assim limitado a 254 labels
	for (i = 0, size = bytesperline * height; i < size; i++)
	{
		if (datadst[i] != 0) datadst[i] = 255;
	}

	// Limpa os rebordos da imagem binária
	for (y = 0; y < height; y++)
	{
		datadst[y * bytesperline + 0 * channels] = 0;
		datadst[y * bytesperline + (width - 1) * channels] = 0;
	}
	for (x = 0; x < width; x++)
	{
		datadst[0 * bytesperline + x * channels] = 0;
		datadst[(height - 1) * bytesperline + x * channels] = 0;
	}

	// Efectua a etiquetagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			// Kernel:
			// A B C
			// D X

			posA = (y - 1) * bytesperline + (x - 1) * channels; // A
			posB = (y - 1) * bytesperline + x * channels; // B
			posC = (y - 1) * bytesperline + (x + 1) * channels; // C
			posD = y * bytesperline + (x - 1) * channels; // D
			posX = y * bytesperline + x * channels; // X

			// Se o pixel foi marcado
			if (datadst[posX] != 0)
			{
				if ((datadst[posA] == 0) && (datadst[posB] == 0) && (datadst[posC] == 0) && (datadst[posD] == 0))
				{
					datadst[posX] = label;
					labeltable[label] = label;
					label++;
				}
				else
				{
					num = 255;

					// Se A está marcado
					if (datadst[posA] != 0) num = labeltable[datadst[posA]];
					// Se B está marcado, e é menor que a etiqueta "num"
					if ((datadst[posB] != 0) && (labeltable[datadst[posB]] < num)) num = labeltable[datadst[posB]];
					// Se C está marcado, e é menor que a etiqueta "num"
					if ((datadst[posC] != 0) && (labeltable[datadst[posC]] < num)) num = labeltable[datadst[posC]];
					// Se D está marcado, e é menor que a etiqueta "num"
					if ((datadst[posD] != 0) && (labeltable[datadst[posD]] < num)) num = labeltable[datadst[posD]];

					// Atribui a etiqueta ao pixel
					datadst[posX] = num;
					labeltable[num] = num;

					// Actualiza a tabela de etiquetas
					if (datadst[posA] != 0)
					{
						if (labeltable[datadst[posA]] != num)
						{
							for (tmplabel = labeltable[datadst[posA]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posB] != 0)
					{
						if (labeltable[datadst[posB]] != num)
						{
							for (tmplabel = labeltable[datadst[posB]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posC] != 0)
					{
						if (labeltable[datadst[posC]] != num)
						{
							for (tmplabel = labeltable[datadst[posC]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
					if (datadst[posD] != 0)
					{
						if (labeltable[datadst[posD]] != num)
						{
							for (tmplabel = labeltable[datadst[posD]], a = 1; a < label; a++)
							{
								if (labeltable[a] == tmplabel)
								{
									labeltable[a] = num;
								}
							}
						}
					}
				}
			}
		}
	}

	// Volta a etiquetar a imagem
	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			posX = y * bytesperline + x * channels; // X

			if (datadst[posX] != 0)
			{
				datadst[posX] = labeltable[datadst[posX]];
			}
		}
	}

	// Contagem do número de blobs
	// Passo 1: Eliminar, da tabela, etiquetas repetidas
	for (a = 1; a < label - 1; a++)
	{
		for (b = a + 1; b < label; b++)
		{
			if (labeltable[a] == labeltable[b]) labeltable[b] = 0;
		}
	}
	// Passo 2: Conta etiquetas e organiza a tabela de etiquetas, para que não hajam valores vazios (zero) entre etiquetas
	*nlabels = 0;
	for (a = 1; a < label; a++)
	{
		if (labeltable[a] != 0)
		{
			labeltable[*nlabels] = labeltable[a]; // Organiza tabela de etiquetas
			(*nlabels)++; // Conta etiquetas
		}
	}

	// Se não há blobs
	if (*nlabels == 0) return NULL;

	// Cria lista de blobs (objetos) e preenche a etiqueta
	blobs = (OVC*)calloc((*nlabels), sizeof(OVC));
	if (blobs != NULL)
	{
		for (a = 0; a < (*nlabels); a++) blobs[a].label = labeltable[a];
	}
	else return NULL;

	return blobs;
}

/**
 * @brief Calcula as informações dos blobs etiquetados em uma imagem binária.
 * @author lugon
 * @param src Imagem binária com blobs etiquetados.
 * @param blobs Array de estruturas de blobs (objetos) contendo informações dos blobs.
 * @param nblobs Número de blobs etiquetados.
 * @return int Retorna 1 se as informações foram calculadas com sucesso, caso contrário retorna 0.
 */
int vc_binary_blob_info(IVC* src, OVC* blobs, int nblobs)
{
	unsigned char* data = (unsigned char*)src->data;
	int width = src->width;
	int height = src->height;
	int bytesperline = src->bytesperline;
	int channels = src->channels;
	int x, y, i;
	long int pos;
	int xmin, ymin, xmax, ymax;
	long int sumx, sumy;

	// Verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if (channels != 1) return 0;

	// Conta área de cada blob
	for (i = 0; i < nblobs; i++)
	{
		xmin = width - 1;
		ymin = height - 1;
		xmax = 0;
		ymax = 0;

		sumx = 0;
		sumy = 0;

		blobs[i].area = 0;

		for (y = 1; y < height - 1; y++)
		{
			for (x = 1; x < width - 1; x++)
			{
				pos = y * bytesperline + x * channels;

				if (data[pos] == blobs[i].label)
				{
					// Área
					blobs[i].area++;

					// Centro de Gravidade
					sumx += x;
					sumy += y;

					// Bounding Box
					if (xmin > x) xmin = x;
					if (ymin > y) ymin = y;
					if (xmax < x) xmax = x;
					if (ymax < y) ymax = y;

					// Perímetro
					// Se pelo menos um dos quatro vizinhos não pertence ao mesmo label, então é um pixel de contorno
					if ((data[pos - 1] != blobs[i].label) || (data[pos + 1] != blobs[i].label) || (data[pos - bytesperline] != blobs[i].label) || (data[pos + bytesperline] != blobs[i].label))
					{
						blobs[i].perimeter++;
					}
				}
			}
		}

		// Bounding Box
		blobs[i].x = xmin;
		blobs[i].y = ymin;
		blobs[i].width = (xmax - xmin) + 1;
		blobs[i].height = (ymax - ymin) + 1;

		// Centro de Gravidade
		blobs[i].xc = sumx / MAX(blobs[i].area, 1);
		blobs[i].yc = sumy / MAX(blobs[i].area, 1);
	}

	return 1;
}

/**
 * @brief Detecção de contornos utilizando o filtro Prewitt.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde os contornos serão armazenados.
 * @param th Valor de threshold para a detecção de contornos.
 * @return int Retorna 1 se a detecção foi processada com sucesso, caso contrário retorna 0.
 */
int vc_gray_edge_prewitt(IVC* src, IVC* dst, float th)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, fdX, fdY;
	long int pos_src, pos_dst, posA, posB, posC, posD, posE, posF, posG, posH, posX;
	float magn;

	/*Posições		SOBEL X				PREWITT X
	* [A	B	C]		[-1		0	1]		[-1		0	1]
	* [D	X	E]		[-2		0	2]		[-1		0	1]
	* [F	G	H]		[-1		0	1]		[-1		0	1]
	*					SOBEL Y				PREWITT Y
	*					[-1		-2	1]		[-1		-1	-1]
	*					[0		0	0]		[0		0	 0]
	*					[1		2	1]		[1		1	 1]
	*/

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			//POSIÇÕES DO KERNEL / VIZINHOS
			posA = (y - 1) * bytesperline_src + (x - 1) * channels_src;
			posB = (y - 1) * bytesperline_src + x * channels_src;
			posC = (y - 1) * bytesperline_src + (x + 1) * channels_src;
			posD = y * bytesperline_src + (x - 1) * channels_src;
			posX = y * bytesperline_src + x * channels_src;	//POSIÇÃO X
			posE = y * bytesperline_src + (x + 1) * channels_src;
			posF = (y + 1) * bytesperline_src + (x - 1) * channels_src;
			posG = (y + 1) * bytesperline_src + x * channels_src;
			posH = (y + 1) * bytesperline_src + (x + 1) * channels_src;

			// Calcular derivadas
			fdX = datasrc[posA] * -1 + datasrc[posC] * 1 + datasrc[posD] * -1 + datasrc[posE] * 1 + datasrc[posF] * -1 + datasrc[posH] * 1;
			fdY = datasrc[posA] * -1 + datasrc[posB] * -1 + datasrc[posC] * -1 + datasrc[posF] * 1 + datasrc[posG] * 1 + datasrc[posH] * 1;

			// Calcular magnitude
			magn = sqrt(fdX * fdX + fdY * fdY);
			if (magn > th)
			{
				datadst[posX] = 255;
			}
			else datadst[posX] = 0;
		}
	}
	return 1;
}

/**
 * @brief Detecção de contornos utilizando o filtro Sobel.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde os contornos serão armazenados.
 * @param th Valor de threshold para a detecção de contornos.
 * @return int Retorna 1 se a detecção foi processada com sucesso, caso contrário retorna 0.
 */
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, fdX, fdY;
	long int pos_src, pos_dst, posA, posB, posC, posD, posE, posF, posG, posH, posX;
	float magn;

	/*Posições		SOBEL X				PREWITT X
	* [A	B	C]		[-1		0	1]		[-1		0	1]
	* [D	X	E]		[-2		0	2]		[-1		0	1]
	* [F	G	H]		[-1		0	1]		[-1		0	1]
	*					SOBEL Y				PREWITT Y
	*					[-1		-2	1]		[-1		-1	-1]
	*					[0		0	0]		[0		0	 0]
	*					[1		2	1]		[1		1	 1]
	*/

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	for (y = 1; y < height - 1; y++)
	{
		for (x = 1; x < width - 1; x++)
		{
			//POSIÇÕES DO KERNEL / VIZINHOS
			posA = (y - 1) * bytesperline_src + (x - 1) * channels_src;
			posB = (y - 1) * bytesperline_src + x * channels_src;
			posC = (y - 1) * bytesperline_src + (x + 1) * channels_src;
			posD = y * bytesperline_src + (x - 1) * channels_src;
			posX = y * bytesperline_src + x * channels_src;	//POSIÇÃO X
			posE = y * bytesperline_src + (x + 1) * channels_src;
			posF = (y + 1) * bytesperline_src + (x - 1) * channels_src;
			posG = (y + 1) * bytesperline_src + x * channels_src;
			posH = (y + 1) * bytesperline_src + (x + 1) * channels_src;

			// Calcular derivadas
			fdX = datasrc[posA] * -1 + datasrc[posC] * 1 + datasrc[posD] * -2 + datasrc[posE] * 2 + datasrc[posF] * -1 + datasrc[posH] * 1;
			fdY = datasrc[posA] * -1 + datasrc[posB] * -2 + datasrc[posC] * -1 + datasrc[posF] * 1 + datasrc[posG] * 2 + datasrc[posH] * 1;

			// Calcular magnitude
			magn = sqrt(fdX * fdX + fdY * fdY);
			if (magn > th)
			{
				datadst[posX] = 255;
			}
			else datadst[posX] = 0;
		}
	}
	return 1;
}

/**
 * @brief Aplica um filtro de média em uma imagem em escala de cinza.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde o resultado do filtro será armazenado.
 * @param kernel Tamanho do kernel a ser usado para o filtro.
 * @return int Retorna 1 se o filtro foi processado com sucesso, caso contrário retorna 0.
 */
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize;
	long int pos_src, pos_dst, pos_src2, soma = 0;
	float media;

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	/*Posições
	* [A	B	C]
	* [D	X	E]
	* [F	G	H]
	*/
	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posição da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posição destino

			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{
						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posição da source
						soma += datasrc[pos_src2];
					}
					else datadst[pos_dst] = datasrc[pos_src];//se o kernel estiver fora da imagem
				}
			}
			media = soma / (float)(kernel * kernel);//exemplo 5 por 5 = kernel x kernel
			datadst[pos_dst] = media;
			soma = 0;
		}
	}
	return 1;
}

/**
 * @brief Aplica um filtro de mediana em uma imagem em escala de cinza.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde o resultado do filtro será armazenado.
 * @param kernel Tamanho do kernel a ser usado para o filtro.
 * @return int Retorna 1 se o filtro foi processado com sucesso, caso contrário retorna 0.
 */
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	//info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	//auxiliares gerais
	int x, y, x2, y2, ksize, npixkernel, i;
	npixkernel = kernel * kernel;
	long int pos_src, pos_dst, pos_src2, mediana[1000];

	//verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	/*Posições
	* [A	B	C]
	* [D	X	E]
	* [F	G	H]
	*/
	ksize = (kernel - 1) / 2;
	for (y = 0;y < height;y++)
	{
		for (x = 0;x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src;//posição da source
			pos_dst = y * bytesperline_dst + x * channels_dst;//posição destino

			i = 0;
			for (y2 = (y - ksize); y2 <= (y + ksize);y2++)
			{
				for (x2 = (x - ksize); x2 <= (x + ksize);x2++)
				{
					if ((y2 >= 0) && (y2 < height) && (x2 >= 0) && (x2 < width))
					{
						pos_src2 = y2 * bytesperline_src + x2 * channels_src;//posição da source
						mediana[i] = datasrc[pos_src2];
						i++;
					}
				}
			}
			// Bubble sort
			for (i = 0; i < npixkernel - 1; i++)
			{
				for (int j = 0; j < npixkernel - i - 1; j++)
				{
					if (mediana[j] > mediana[j + 1])
					{
						int temp = mediana[j];
						mediana[j] = mediana[j + 1];
						mediana[j + 1] = temp;
					}
				}
			}
			datadst[pos_dst] = mediana[(npixkernel - 1) / 2];
		}
	}
	return 1;
}

/**
 * @brief Aplica um filtro de Gauss em uma imagem em escala de cinza.
 * @author lugon
 * @param src Imagem de origem em escala de cinza.
 * @param dst Imagem de destino onde o resultado do filtro será armazenado.
 * @param kernel Tamanho do kernel a ser usado para o filtro.
 * @return int Retorna 1 se o filtro foi processado com sucesso, caso contrário retorna 0.
 */
int vc_gray_lowpass_gaussian_filter(IVC* src, IVC* dst, int kernel)
{
	// info source
	unsigned char* datasrc = (unsigned char*)src->data;
	int bytesperline_src = src->width * src->channels;
	int channels_src = src->channels;

	// info destino
	unsigned char* datadst = (unsigned char*)dst->data;
	int bytesperline_dst = dst->width * dst->channels;
	int channels_dst = dst->channels;

	// medidas
	int width = src->width;
	int height = src->height;

	// auxiliares gerais
	int x, y, x2, y2, ksize, npixkernel;
	float sum, weight_sum;
	long int pos_src, pos_dst, pos_src2;

	// verificação de erros
	if ((src->width <= 0) || (src->height <= 0) || (src->data == NULL)) return 0;
	if ((src->width != dst->width) || (src->height != dst->height)) return 0;
	if ((src->channels != 1) || (dst->channels != 1)) return 0;

	// Kernel Gaussiano 3x3 (sigma = 1)
	float kernel_gauss[3][3] = {
		{1 / 16.0, 2 / 16.0, 1 / 16.0},
		{2 / 16.0, 4 / 16.0, 2 / 16.0},
		{1 / 16.0, 2 / 16.0, 1 / 16.0}
	};

	ksize = 1; // 3x3 kernel has a radius of 1
	npixkernel = (ksize * 2 + 1) * (ksize * 2 + 1);

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			pos_src = y * bytesperline_src + x * channels_src; // posição da source
			pos_dst = y * bytesperline_dst + x * channels_dst; // posição destino

			sum = 0.0;
			weight_sum = 0.0;

			for (y2 = -ksize; y2 <= ksize; y2++)
			{
				for (x2 = -ksize; x2 <= ksize; x2++)
				{
					int yk = y + y2;
					int xk = x + x2;

					if (yk >= 0 && yk < height && xk >= 0 && xk < width)
					{
						pos_src2 = yk * bytesperline_src + xk * channels_src; // posição da source
						sum += datasrc[pos_src2] * kernel_gauss[y2 + ksize][x2 + ksize];
						weight_sum += kernel_gauss[y2 + ksize][x2 + ksize];
					}
				}
			}
			datadst[pos_dst] = (unsigned char)(sum / weight_sum);
		}
	}
	return 1;
}

/**
 * @brief Desenha a caixa delimitadora de um objeto.
 * @author lugon
 * @param srcdst Imagem de entrada e saída onde a caixa delimitadora será desenhada.
 * @param blob Estrutura que contém as informações do objeto.
 * @return int Retorna 1 se a caixa foi desenhada com sucesso, caso contrário retorna 0.
 */
int vc_draw_boundingbox(IVC* srcdst, OVC* blob)
{
	int c;
	int x, y;

	for (y = blob->y; y < blob->y + blob->height; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->x * srcdst->channels] = 255;
			srcdst->data[y * srcdst->bytesperline + (blob->x + blob->width - 1) * srcdst->channels] = 255;
		}
	}

	for (x = blob->x; x < blob->x + blob->width; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->y * srcdst->bytesperline + x * srcdst->channels] = 255;
			srcdst->data[(blob->y + blob->height - 1) * srcdst->bytesperline + x * srcdst->channels] = 255;
		}
	}

	return 1;
}

/**
 * @brief Desenha o centro de gravidade de um objeto.
 * @author lugon
 * @param srcdst Imagem de entrada e saída onde o centro de gravidade será desenhado.
 * @param blob Estrutura que contém as informações do objeto.
 * @return int Retorna 1 se o centro de gravidade foi desenhado com sucesso, caso contrário retorna 0.
 */
int vc_draw_centerofgravity(IVC* srcdst, OVC* blob)
{
	int c;
	int x, y;
	int xmin, xmax, ymin, ymax;
	int s = 3;

	xmin = blob->xc - s;
	ymin = blob->yc - s;
	xmax = blob->xc + s;
	ymax = blob->yc + s;

	if (xmin < blob->x) xmin = blob->x;
	if (ymin < blob->y) ymin = blob->y;
	if (xmax > blob->x + blob->width - 1) xmax = blob->x + blob->width - 1;
	if (ymax > blob->y + blob->height - 1) ymax = blob->y + blob->height - 1;

	for (y = ymin; y <= ymax; y++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[y * srcdst->bytesperline + blob->xc * srcdst->channels] = 255;
		}
	}

	for (x = xmin; x <= xmax; x++)
	{
		for (c = 0; c < srcdst->channels; c++)
		{
			srcdst->data[blob->yc * srcdst->bytesperline + x * srcdst->channels] = 255;
		}
	}

	return 1;
}
