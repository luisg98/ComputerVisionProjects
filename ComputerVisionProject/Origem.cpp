#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

extern "C" {
#include "vc.h"
}

void vc_timer(void) {
    static bool running = false;
    static std::chrono::steady_clock::time_point previousTime = std::chrono::steady_clock::now();

    if (!running) {
        running = true;
    }
    else {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::steady_clock::duration elapsedTime = currentTime - previousTime;

        // Tempo em segundos.
        std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(elapsedTime);
        double nseconds = time_span.count();

        std::cout << "Tempo decorrido: " << nseconds << " segundos" << std::endl;
        std::cout << "Pressione qualquer tecla para continuar...\n";
        std::cin.get();
    }
}



/**
 * @brief Converte uma imagem de BGR para RGB
 *
 * @author lugon
 * @param src Imagem de entrada no formato BGR
 * @return IVC* Imagem convertida no formato RGB
 */
IVC* convertBGRToRGB(IVC* src) {
    IVC* dst = vc_image_new(src->width, src->height, src->channels, src->levels);
    int size = src->width * src->height * src->channels;
    for (int i = 0; i < size; i += src->channels) {
        dst->data[i] = src->data[i + 2];
        dst->data[i + 1] = src->data[i + 1];
        dst->data[i + 2] = src->data[i];
    }
    return dst;
}


/**
 * @brief Calcula a distância euclidiana entre dois pontos
 *
 * @author lugon
 * @param pt1 Primeiro ponto
 * @param pt2 Segundo ponto
 * @return double Distância euclidiana
 */
double euclideanDistance(const cv::Point& pt1, const cv::Point& pt2) {
    return std::sqrt(std::pow(pt1.x - pt2.x, 2) + std::pow(pt1.y - pt2.y, 2));
}


/**
 * @brief Segmenta a cor utilizando a biblioteca vc.c
 *
 * @author lugon
 * @param src Imagem de entrada no formato HSV
 * @param minHue Valor mínimo do matiz
 * @param maxHue Valor máximo do matiz
 * @param minSaturation Valor mínimo da saturação
 * @param maxSaturation Valor máximo da saturação
 * @param minValue Valor mínimo do valor
 * @param maxValue Valor máximo do valor
 * @return IVC* Imagem segmentada
 */
IVC* segmentColor(IVC* src, unsigned char minHue, unsigned char maxHue, unsigned char minSaturation, unsigned char maxSaturation, unsigned char minValue, unsigned char maxValue) {
    IVC* dst = vc_image_new(src->width, src->height, 1, 255);
    vc_hsv_segmentation(src, dst, minHue, maxHue, minSaturation, maxSaturation, minValue, maxValue);
    return dst;
}

/**
 * @brief Converte uma imagem de OpenCV Mat para IVC
 *
 * @author lugon
 * @param mat Imagem de entrada no formato OpenCV Mat
 * @return IVC* Imagem convertida no formato IVC
 */
// Função para converter de OpenCV Mat para IVC
IVC* convertMatToIVC(cv::Mat& mat) {
    IVC* image = vc_image_new(mat.cols, mat.rows, mat.channels(), 255);
    memcpy(image->data, mat.data, mat.cols * mat.rows * mat.channels());
    return image;
}



/**
 * @brief Converte uma imagem de IVC para OpenCV Mat
 *
 * @author lugon
 * @param image Imagem de entrada no formato IVC
 * @return cv::Mat Imagem convertida no formato OpenCV Mat
 */
cv::Mat convertIVCToMat(IVC* image) {
    cv::Mat mat(image->height, image->width, image->channels == 3 ? CV_8UC3 : CV_8UC1, image->data);
    return mat.clone(); // Adicionado clone() para garantir a integridade dos dados
}

// Estrutura para armazenar intervalos de cor e seus valores correspondentes
struct ColorRange {
    cv::Scalar lower;
    cv::Scalar upper;
    std::string colorName;
    int value;
};

// Mapa de cores possíveis para uma resistência
const std::vector<ColorRange> colorRanges = {
    {cv::Scalar(0, 0, 0), cv::Scalar(50, 90, 80), "Preto", 0},
    {cv::Scalar(0, 90, 80), cv::Scalar(30, 150, 150), "Castanho", 1},
    {cv::Scalar(0, 90, 150), cv::Scalar(10, 255, 200), "Vermelho", 2},
    {cv::Scalar(5, 150, 150), cv::Scalar(30, 255, 255), "Laranja", 3},
    {cv::Scalar(20, 100, 150), cv::Scalar(40, 255, 255), "Amarelo", 4},
    {cv::Scalar(45, 50, 50), cv::Scalar(60, 255, 200), "Verde", 5},
    {cv::Scalar(60, 50, 50), cv::Scalar(140, 255, 255), "Azul", 6},
    {cv::Scalar(125, 50, 100), cv::Scalar(150, 255, 255), "Violeta", 7},
    {cv::Scalar(50, 50, 50), cv::Scalar(180, 100, 100), "Cinza", 8},
    {cv::Scalar(0, 0, 200), cv::Scalar(180, 50, 255), "Branco", 9},
};

/**
 * @brief Converte uma imagem de BGR para HSV
 *
 * @author lugon
 * @param bgr Imagem de entrada no formato BGR
 * @param hsv Imagem de saída no formato HSV
 */
void convertBGRToHSV(cv::Mat& bgr, cv::Mat& hsv) {
    hsv.create(bgr.size(), bgr.type());
    for (int y = 0; y < bgr.rows; ++y) {
        for (int x = 0; x < bgr.cols; ++x) {
            cv::Vec3b bgrPixel = bgr.at<cv::Vec3b>(y, x);
            float blue = bgrPixel[0] / 255.0;
            float green = bgrPixel[1] / 255.0;
            float red = bgrPixel[2] / 255.0;

            float cmax = std::max({ red, green, blue });
            float cmin = std::min({ red, green, blue });
            float delta = cmax - cmin;

            float h = 0, s = 0, v = 0;
            v = cmax;

            if (cmax != 0)
                s = delta / cmax;
            else
                s = 0;

            if (delta != 0) {
                if (cmax == red)
                    h = 60 * (fmod(((green - blue) / delta), 6));
                else if (cmax == green)
                    h = 60 * (((blue - red) / delta) + 2);
                else if (cmax == blue)
                    h = 60 * (((red - green) / delta) + 4);
            }
            else
                h = 0;

            if (h < 0)
                h += 360;

            hsv.at<cv::Vec3b>(y, x)[0] = static_cast<unsigned char>((h / 2));
            hsv.at<cv::Vec3b>(y, x)[1] = static_cast<unsigned char>((s * 255));
            hsv.at<cv::Vec3b>(y, x)[2] = static_cast<unsigned char>((v * 255));
        }
    }
}

/**
 * @brief Calcula a média da cor em uma região da imagem
 *
 * @author lugon
 * @param img Imagem de entrada no formato HSV
 * @param rect Retângulo que define a região de interesse
 * @return cv::Scalar Média da cor na região especificada
 */
cv::Scalar getCenterColorHSV(cv::Mat& img, cv::Rect rect) {
    // Extrair o ponto central do retângulo
    cv::Point center(rect.x + rect.width / 2, rect.y + rect.height / 2);

    // Obter a cor no ponto central
    cv::Vec3b color = img.at<cv::Vec3b>(center);
    return cv::Scalar(color[0], color[1], color[2]);
}

/**
 * @brief Mapeia a cor para o valor da resistência
 *
 * @author lugon
 * @param color Cor no formato HSV
 * @param colorName Nome da cor correspondente
 * @return int Valor da resistência correspondente à cor
 */
int mapColorToValue(cv::Scalar color, std::string& colorName) {
    for (const auto& range : colorRanges) {
        if (color[0] >= range.lower[0] && color[0] <= range.upper[0] &&
            color[1] >= range.lower[1] && color[1] <= range.upper[1] &&
            color[2] >= range.lower[2] && color[2] <= range.upper[2]) {
            colorName = range.colorName;
            return range.value;
        }
    }
    colorName = "unknown";
    return -1; // Valor de erro caso a cor não seja encontrada
}

/**
 * @brief Calcula a distância mínima entre dois contornos
 *
 * @author lugon
 * @param contour1 Primeiro contorno
 * @param contour2 Segundo contorno
 * @return double Distância mínima entre os contornos
 */
double minDistance(const std::vector<cv::Point>& contour1, const std::vector<cv::Point>& contour2) {
    double minDist = std::numeric_limits<double>::max();
    for (const auto& pt1 : contour1) {
        for (const auto& pt2 : contour2) {
            double dist = euclideanDistance(pt1, pt2);
            if (dist < minDist) {
                minDist = dist;
            }
        }
    }
    return minDist;
}


void mergeCloseContours(std::vector<std::vector<cv::Point>>& contours, double maxDist) {
    bool merged;
    do {
        merged = false;
        for (size_t i = 0; i < contours.size(); i++) {
            for (size_t j = i + 1; j < contours.size(); j++) {
                double dist = minDistance(contours[i], contours[j]);
                if (dist < maxDist) {
                    contours[i].insert(contours[i].end(), contours[j].begin(), contours[j].end());
                    contours.erase(contours.begin() + j);
                    merged = true;
                    break;
                }
            }
            if (merged) break;
        }
    } while (merged);
}

/**
 * @brief Desenha um retângulo na imagem
 * 
 * @author lugon
 * @param img Imagem de entrada
 * @param rect Retângulo a ser desenhado
 * @param color Cor do retângulo
 * @param thickness Espessura da linha
 */
void drawRectangle(cv::Mat& img, cv::Rect rect, const cv::Scalar& color, int thickness = 1) {
    for (int i = 0; i < thickness; ++i) {
        // Top edge
        for (int x = rect.x; x < rect.x + rect.width; ++x) {
            img.at<cv::Vec3b>(rect.y + i, x) = cv::Vec3b(color[0], color[1], color[2]);
            img.at<cv::Vec3b>(rect.y + rect.height - i - 1, x) = cv::Vec3b(color[0], color[1], color[2]);
        }
        // Bottom edge
        for (int x = rect.x; x < rect.x + rect.width; ++x) {
            img.at<cv::Vec3b>(rect.y + rect.height - i - 1, x) = cv::Vec3b(color[0], color[1], color[2]);
        }
        // Left edge
        for (int y = rect.y; y < rect.y + rect.height; ++y) {
            img.at<cv::Vec3b>(y, rect.x + i) = cv::Vec3b(color[0], color[1], color[2]);
            img.at<cv::Vec3b>(y, rect.x + rect.width - i - 1) = cv::Vec3b(color[0], color[1], color[2]);
        }
        // Right edge
        for (int y = rect.y; y < rect.y + rect.height; ++y) {
            img.at<cv::Vec3b>(y, rect.x + rect.width - i - 1) = cv::Vec3b(color[0], color[1], color[2]);
        }
    }
}

/**
 * @brief Desenha uma linha na imagem
 *
 * @author lugon
 * @param img Imagem de entrada
 * @param pt1 Ponto inicial
 * @param pt2 Ponto final
 * @param color Cor da linha
 * @param thickness Espessura da linha
 */
void drawLine(cv::Mat& img, cv::Point pt1, cv::Point pt2, const cv::Scalar& color, int thickness = 1) {
    int dx = abs(pt2.x - pt1.x), sx = pt1.x < pt2.x ? 1 : -1;
    int dy = -abs(pt2.y - pt1.y), sy = pt1.y < pt2.y ? 1 : -1;
    int err = dx + dy, e2;

    while (true) {
        for (int i = -thickness / 2; i <= thickness / 2; ++i) {
            for (int j = -thickness / 2; j <= thickness / 2; ++j) {
                if (pt1.y + i >= 0 && pt1.y + i < img.rows && pt1.x + j >= 0 && pt1.x + j < img.cols) {
                    img.at<cv::Vec3b>(pt1.y + i, pt1.x + j) = cv::Vec3b(color[0], color[1], color[2]);
                }
            }
        }
        if (pt1.x == pt2.x && pt1.y == pt2.y) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; pt1.x += sx; }
        if (e2 <= dx) { err += dx; pt1.y += sy; }
    }
}


/**
 * @brief Desenha um círculo na imagem
 *
 * @author lugon
 * @param img Imagem de entrada
 * @param center Centro do círculo
 * @param radius Raio do círculo
 * @param color Cor do círculo
 * @param thickness Espessura da linha
 */
void drawCircle(cv::Mat& img, cv::Point center, int radius, const cv::Scalar& color, int thickness = 1) {
    int x = radius, y = 0;
    int err = 1 - x;

    while (x >= y) {
        for (int i = -thickness / 2; i <= thickness / 2; ++i) {
            for (int j = -thickness / 2; j <= thickness / 2; ++j) {
                if (center.y + y + i >= 0 && center.y + y + i < img.rows && center.x + x + j >= 0 && center.x + x + j < img.cols)
                    img.at<cv::Vec3b>(center.y + y + i, center.x + x + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y + x + i >= 0 && center.y + x + i < img.rows && center.x + y + j >= 0 && center.x + y + j < img.cols)
                    img.at<cv::Vec3b>(center.y + x + i, center.x + y + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y + y + i >= 0 && center.y + y + i < img.rows && center.x - x + j >= 0 && center.x - x + j < img.cols)
                    img.at<cv::Vec3b>(center.y + y + i, center.x - x + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y + x + i >= 0 && center.y + x + i < img.rows && center.x - y + j >= 0 && center.x - y + j < img.cols)
                    img.at<cv::Vec3b>(center.y + x + i, center.x - y + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y - y + i >= 0 && center.y - y + i < img.rows && center.x + x + j >= 0 && center.x + x + j < img.cols)
                    img.at<cv::Vec3b>(center.y - y + i, center.x + x + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y - x + i >= 0 && center.y - x + i < img.rows && center.x + y + j >= 0 && center.x + y + j < img.cols)
                    img.at<cv::Vec3b>(center.y - x + i, center.x + y + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y - y + i >= 0 && center.y - y + i < img.rows && center.x - x + j >= 0 && center.x - x + j < img.cols)
                    img.at<cv::Vec3b>(center.y - y + i, center.x - x + j) = cv::Vec3b(color[0], color[1], color[2]);
                if (center.y - x + i >= 0 && center.y - x + i < img.rows && center.x - y + j >= 0 && center.x - y + j < img.cols)
                    img.at<cv::Vec3b>(center.y - x + i, center.x - y + j) = cv::Vec3b(color[0], color[1], color[2]);
            }
        }
        y++;
        if (err <= 0) {
            err += 2 * y + 1;
        }
        if (err > 0) {
            x--;
            err -= 2 * x + 1;
        }
    }
}


int main(void) {
    // Nome do arquivo de vídeo
    char videofile[20] = "video_resistors.mp4";
    cv::VideoCapture capture;
    struct {
        int width, height;
        int ntotalframes;
        int fps;
        int nframe;
    } video;
    // Outros
    std::string str;
    int key = 0;

    // Variáveis para armazenar os valores anteriores
    std::string prevColorName1 = "unknown";
    std::string prevColorName2 = "unknown";
    std::string prevColorName3 = "unknown";
    double prevResistorValue = -1;

    capture.open(videofile);

    // Verifica se foi possível abrir o ficheiro de vídeo
    if (!capture.isOpened()) {
        std::cerr << "Erro ao abrir o ficheiro de vídeo!\n";
        return 1;
    }

    // Número total de frames no vídeo
    video.ntotalframes = (int)capture.get(cv::CAP_PROP_FRAME_COUNT);
    // Frame rate do vídeo
    video.fps = (int)capture.get(cv::CAP_PROP_FPS);
    // Resolução do vídeo
    video.width = (int)capture.get(cv::CAP_PROP_FRAME_WIDTH);
    video.height = (int)capture.get(cv::CAP_PROP_FRAME_HEIGHT);

    // Criar janelas para exibir o vídeo original e processado
    cv::namedWindow("VC - VIDEO ORIGINAL", cv::WINDOW_AUTOSIZE);
    cv::namedWindow("VC - VIDEO PROCESSADO", cv::WINDOW_AUTOSIZE);

    // Inicia o timer
    vc_timer();

    // Definir intervalos de cor para segmentação da cor amarela
    unsigned char minHue = 25, maxHue = 30; // Intervalo para a cor amarela
    unsigned char minSaturation = 105, maxSaturation = 255;
    unsigned char minValue = 100, maxValue = 255;

    cv::Mat frame;
    while (key != 'q') {
        // Leitura de uma frame do vídeo
        capture.read(frame);

        // Verifica se conseguiu ler a frame
        if (frame.empty()) break;

        // Número da frame a processar
        video.nframe = (int)capture.get(cv::CAP_PROP_POS_FRAMES);

        // Converter o frame para a estrutura IVC
        IVC* ivc_frame_bgr = convertMatToIVC(frame);
        IVC* ivc_frame = convertBGRToRGB(ivc_frame_bgr); // Converter BGR para RGB

        // Converter o frame para HSV
        IVC* ivc_hsv = vc_image_new(ivc_frame->width, ivc_frame->height, ivc_frame->channels, ivc_frame->levels);
        vc_rgb_to_hsv(ivc_frame, ivc_hsv);

        // Segmentar a cor amarela
        IVC* maskYellow = segmentColor(ivc_hsv, minHue, maxHue, minSaturation, maxSaturation, minValue, maxValue);

        // Criar imagens para operações morfológicas
        IVC* dilatedMask = vc_image_new(maskYellow->width, maskYellow->height, maskYellow->channels, maskYellow->levels);
        IVC* erodedMask = vc_image_new(dilatedMask->width, dilatedMask->height, dilatedMask->channels, dilatedMask->levels);

        // Aplicar erosão
        if (!vc_binary_erode(maskYellow, erodedMask, 3)) {
            std::cerr << "Erro ao aplicar erosão!" << std::endl;
            // Liberar memória e sair
            vc_image_free(dilatedMask);
            vc_image_free(erodedMask);
            return 1;
        }

        // Aplicar dilatação
        if (!vc_binary_dilate(erodedMask, dilatedMask, 5)) {
            std::cerr << "Erro ao aplicar dilatação!" << std::endl;
            // Liberar memória e sair
            vc_image_free(dilatedMask);
            vc_image_free(erodedMask);
            return 1;
        }

        // Converter a imagem processada de volta para cv::Mat para exibir
        cv::Mat morphFrame = convertIVCToMat(dilatedMask);

        // Encontrar blobs na máscara segmentada usando OpenCV
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(morphFrame, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Unir contornos próximos para formar um único contorno por resistência
        const double maxDist = 200; // Distância máxima para agrupar contornos
        mergeCloseContours(contours, maxDist);

        // Processar cada contorno agrupado
        for (const auto& contour : contours) {
            cv::Rect boundingBox = cv::boundingRect(contour);

            drawRectangle(frame, boundingBox, cv::Scalar(0, 0, 255), 2);

            // Dividir o retângulo delimitador em 10 partes iguais e desenhar apenas as linhas 2, 3, 4, 5, 6 e 7
            int partWidth = boundingBox.width / 10;
            for (int i = 2; i <= 7; i++) {
                int x = boundingBox.x + i * partWidth;
                drawLine(frame, cv::Point(x, boundingBox.y), cv::Point(x, boundingBox.y + boundingBox.height), cv::Scalar(0, 255, 0), 2);
            }

            // Converter o frame para HSV
            cv::Mat hsvFrame;
            convertBGRToHSV(frame, hsvFrame);

            // Extrair as cores das partes 2, 3 e 4
            cv::Scalar color1 = getCenterColorHSV(hsvFrame, cv::Rect(boundingBox.x + 2 * partWidth, boundingBox.y, partWidth, boundingBox.height));
            cv::Scalar color2 = getCenterColorHSV(hsvFrame, cv::Rect(boundingBox.x + 4 * partWidth, boundingBox.y, partWidth, boundingBox.height));
            cv::Scalar color3 = getCenterColorHSV(hsvFrame, cv::Rect(boundingBox.x + 6 * partWidth, boundingBox.y, partWidth, boundingBox.height));

            // Adicionar pontos centrais na imagem para visualização
            cv::Point center1(boundingBox.x + 2 * partWidth + partWidth / 2, boundingBox.y + boundingBox.height / 2);
            cv::Point center2(boundingBox.x + 4 * partWidth + partWidth / 2, boundingBox.y + boundingBox.height / 2);
            cv::Point center3(boundingBox.x + 6 * partWidth + partWidth / 2, boundingBox.y + boundingBox.height / 2);

            drawCircle(frame, center1, 3, cv::Scalar(0, 0, 0), -1);
            drawCircle(frame, center2, 3, cv::Scalar(0, 0, 0), -1);
            drawCircle(frame, center3, 3, cv::Scalar(0, 0, 0), -1);

            // Imprimir valores dos blobs no terminal
            std::cout << "Blob 1 HSV: (" << color1[0] << ", " << color1[1] << ", " << color1[2] << ")\n";
            std::cout << "Blob 2 HSV: (" << color2[0] << ", " << color2[1] << ", " << color2[2] << ")\n";
            std::cout << "Blob 3 HSV: (" << color3[0] << ", " << color3[1] << ", " << color3[2] << ")\n";

            std::string colorName1, colorName2, colorName3;

            // Mapear as cores para os valores da resistência
            int digit1 = mapColorToValue(color1, colorName1);
            int digit2 = mapColorToValue(color2, colorName2);
            int multiplier = mapColorToValue(color3, colorName3);

            // Apenas atualizar se nenhum dos valores for "unknown"
            if (colorName1 != "unknown" && colorName2 != "unknown" && colorName3 != "unknown") {
                // Calcular o valor da resistência
                double resistorValue = (digit1 * 10 + digit2) * std::pow(10, multiplier);

                // Atualizar os valores anteriores
                prevColorName1 = colorName1;
                prevColorName2 = colorName2;
                prevColorName3 = colorName3;
                prevResistorValue = resistorValue;
            }

            // Exibir o valor da resistência na imagem
            std::string resistorText = "VALOR DA RESISTENCIA: " + std::to_string(prevResistorValue) + " ohms";

            // Exibir o nome das cores na imagem
            cv::putText(frame, "BANDA 1: " + prevColorName1, cv::Point(boundingBox.x, boundingBox.y - 40), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, "BANDA 2: " + prevColorName2, cv::Point(boundingBox.x, boundingBox.y - 25), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, "MULTIPLICADOR: " + prevColorName3, cv::Point(boundingBox.x, boundingBox.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
            cv::putText(frame, resistorText, cv::Point(boundingBox.x, boundingBox.y + boundingBox.height + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
        }

        // Exibir o frame original com bounding boxes e texto
        cv::imshow("VC - VIDEO ORIGINAL", frame);

        // Exibir o frame processado
        cv::imshow("VC - VIDEO PROCESSADO", morphFrame);

        // Liberar memória das estruturas IVC
        vc_image_free(ivc_frame_bgr);
        vc_image_free(ivc_frame);
        vc_image_free(ivc_hsv);
        vc_image_free(maskYellow);
        vc_image_free(dilatedMask);
        vc_image_free(erodedMask);

        // Sair do loop se a tecla 'q' for pressionada
        key = cv::waitKey(1);
    }

    // Para o timer e exibe o tempo decorrido
    vc_timer();

    // Fecha as janelas
    cv::destroyWindow("VC - VIDEO ORIGINAL");
    cv::destroyWindow("VC - VIDEO PROCESSADO");

    // Fecha o ficheiro de vídeo
    capture.release();

    return 0;
}
