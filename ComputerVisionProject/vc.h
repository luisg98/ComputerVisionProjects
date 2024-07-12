
#define VC_DEBUG


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                   ESTRUTURA DE UMA IMAGEM
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
typedef struct {
	unsigned char *data;
	int width, height;
	int channels;			// Binário/Cinzentos=1; RGB=3
	int levels;				// Binário=1; Cinzentos [1,255]; RGB [1,255]
	int bytesperline;		// width * channels
} IVC;


typedef struct {
	int x, y, width, height;	// Caixa Delimitadora (Bounding Box)
	int area;					// �rea
	int xc, yc;					// Centro-de-massa
	int perimeter;				// Per�metro
	int label;					// Etiqueta
} OVC;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                    PROTÓTIPOS DE FUNÇÕES
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// FUNÇÕES: ALOCAR E LIBERTAR UMA IMAGEM
IVC *vc_image_new(int width, int height, int channels, int levels);
IVC *vc_image_free(IVC *image);
// FUNÇÕES: LEITURA E ESCRITA DE IMAGENS (PBM, PGM E PPM)
IVC *vc_read_image(char *filename);
int vc_write_image(char *filename, IVC *image);
int vc_gray_negative(IVC* srcdst);
int vc_rgb_negative(IVC* srcdst);
int vc_rgb_get_red_gray(IVC* srcdst);
int vc_rgb_get_green_gray(IVC* srcdst);
int vc_rgb_get_blue_gray(IVC* srcdst);
int vc_rgb_to_hsv(IVC* src, IVC* dst);
int vc_rgb_to_gray(IVC* src, IVC* dst);
int vc_scale_gray_to_rgb(IVC* src, IVC* dst);
int vc_gray_to_binary(IVC *src, IVC *dst, int threshold);
int vc_gray_to_binary_global_mean(IVC* src, IVC* dst);
int vc_binary_dilate(IVC* src, IVC* dst, int kernel);
int vc_binary_erode(IVC* src, IVC* dst, int kernel);
OVC* vc_binary_blob_labelling(IVC *src, IVC *dst, int *nlabels);
int vc_binary_blob_info(IVC *src, OVC *blobs, int nblobs);
int vc_gray_edge_prewitt(IVC *src, IVC *dst, float th);
int vc_gray_edge_sobel(IVC* src, IVC* dst, float th);
int vc_gray_lowpass_gaussian_filter(IVC* src, IVC* dst, int kernel);
int vc_gray_to_binary_kernel_midpoint(IVC* src, IVC* dst, int kernel);
int vc_gray_histogram_show(IVC* src, IVC* dst);
int vc_gray_histogram_equalization(IVC* src, IVC* dst);
int vc_gray_lowpass_mean_filter(IVC* src, IVC* dst, int kernel);
int vc_gray_lowpass_median_filter(IVC* src, IVC* dst, int kernel);
int vc_draw_boundingbox(IVC *srcdst, OVC *blob);
int vc_draw_centerofgravity(IVC *srcdst, OVC *blob);
int vc_hsv_segmentation(IVC* src, IVC* dst, unsigned char minHue, unsigned char maxHue, unsigned char minSaturation, unsigned char maxSaturation, unsigned char minValue, unsigned char maxValue);