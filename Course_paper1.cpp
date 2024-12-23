//! [includes]
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <vector>
#include <deque>
#include <random>

#include <iostream>


bool is_black_white(const cv::Mat& image) {
    for (size_t y = 0; y < image.rows; y++) {
        for (size_t x = 0; x < image.cols; x++) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(y, x);
            if (pixel[0] != pixel[1] || pixel[1] != pixel[2] || (pixel[0] != 0 && pixel[0] != 255))
                return false;
        }
    }
    return true;
}

cv::Mat visual_cypher(char* path) {
    cv::Mat image = cv::imread(path);
    if (image.empty()) {
        throw std::invalid_argument("Empty!");
    }
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    cv::threshold(grayImage, grayImage, 127, 255, cv::THRESH_BINARY);
    return grayImage;
}

size_t binomial(size_t n, size_t k)
{
    size_t l = k;
    if (n < k) throw std::invalid_argument("n < k");
    if (k > n / 2) l = n - k;
    if (k == 1)  return n;
    if (k == 0)  return 1;
    size_t r = 1;
    for (size_t i = 1; i <= l; ++i) {
        r *= n - l + i;
        r /= i;
    }
    return r;
}


std::vector<std::vector<size_t>> createBooleanMatrix(size_t n, size_t i, size_t c) {
    std::vector<size_t> col(n, 255);
    std::fill(col.begin(), col.begin() + i, 0);

    std::vector<std::vector<size_t>> matrix;
    do {
        for(size_t i = 1; i < c; ++i)
            matrix.push_back(col);
    } while (std::prev_permutation(col.begin(), col.end()));
    return matrix;
}


void encodeAndSaveImages(const cv::Mat& image,
    const std::vector<std::vector<size_t>>& S0,
    const std::vector<std::vector<size_t>>& S1,
    const std::string& folder_path,
    int n) {
    int rows = image.rows;
    int cols = image.cols;
    size_t columnLength = S0[0].size();
    int newWidth = cols * columnLength;

    // Генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distS0(0, S0.size() - 1);
    std::uniform_int_distribution<> distS1(0, S1.size() - 1);

    // Сохранение n зашифрованных изображений
    for (int i = 0; i < n; ++i) {
        // Результирующее изображение
        cv::Mat encodedImage(rows, newWidth, CV_8U, cv::Scalar(0));

        // Обход каждого пикселя
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const std::vector<size_t>& selectedColumn =
                    (image.at<uchar>(row, col) == 255)
                    ? S0[distS0(gen)]
                    : S1[distS1(gen)];

                // Копирование столбца в результирующую матрицу
                for (size_t i = 0; i < selectedColumn.size(); ++i) {
                    encodedImage.at<uchar>(row, col * columnLength + i) =
                        static_cast<uchar>(selectedColumn[i]);
                }
            }
        }

        // Генерация имени файла
        std::string filename = folder_path + "\\encoded_image_" + std::to_string(i) + ".png";

        // Сохранение изображения
        if (!cv::imwrite(filename, encodedImage)) {
            std::cerr << "Error: can't save image " << filename << "!" << std::endl;
        }
        else {
            std::cout << "Image saved: " << filename << std::endl;
        }
    }
}

cv::Mat decodeImages(const std::string& folder_path, int n, int k) {
    // Загрузка n зашифрованных изображений
    std::vector<cv::Mat> encodedImages;
    for (int i = 0; i < k; ++i) {
        std::string filename = folder_path + "\\encoded_image_" + std::to_string(i) + ".png";
        cv::Mat encodedImage = cv::imread(filename, cv::IMREAD_GRAYSCALE);
        if (encodedImage.empty()) {
            std::cerr << "Error with " << filename << "!" << std::endl;
            return cv::Mat();
        }
        encodedImages.push_back(encodedImage);
    }

    // Получение размеров изображений
    int rows = encodedImages[0].rows;
    int cols = encodedImages[0].cols;

    // Создание матрицы для расшифрованного изображения
    cv::Mat decodedImage(rows, cols / n, CV_8UC1, cv::Scalar(0));

    // Применение "простого усреднения" к строкам изображений
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; x+=n) {
            int sum = 0;
            for (int xi = x; xi < x + n; ++xi) {
                for (int i = 0; i < k; ++i) {
                    sum += encodedImages[i].at<uchar>(y, xi);
                }
            }
            decodedImage.at<uchar>(y, x / n) = static_cast<uchar>(sum / n / k >= 255/2 ? 255 : 0);
        }
    }
    if (!cv::imwrite( "D:\\Course_paper\\decrypted\\Map_25_18_1.png", decodedImage)) {
        std::cerr << "Error: can't write " << folder_path + "\\encoded.png" << "!" << std::endl;
    }
    else {
        std::cout << "Complete: " << folder_path + "\\encoded.png" << std::endl;
    }
    return decodedImage;
}


void constructVCS(size_t n, size_t k, cv::Mat image) {
    size_t r = k / 2;
    if (n <= k || n - r - 1 < 1)
        return;
    std::vector<size_t> c(r + 1);
    for (size_t j = 0; j <= r; ++j) {
        c[j] = binomial(n - r - 1 - j, r - j);
    }
    // Построение матриц S^0 и S^1
    std::vector <std::vector<size_t>> S0, S1;
    if (k % 2) {
        for (int j = 0; j < n; j += 2) {
            if (r%2 && j == r + 1)
                j = n - r;
            auto Tmp = createBooleanMatrix(n, j, c[j > r ? n - j : j]);
            S0.insert(S0.end(), Tmp.begin(), Tmp.end());
            if (r % 2 == 0 && j == r)
                j = n - r - 1;
            Tmp = createBooleanMatrix(n, j + 1, c[j > r ? n - j - 1 : j + 1]);
            S1.insert(S1.end(), Tmp.begin(), Tmp.end());
        }
    }
    else {
        for (int j = 0; j < n; j += 2) {
            if (r % 2 && j == r + 1)
                j = n - r + 1;
            auto Tmp = createBooleanMatrix(n, j, c[j > r ? n - j + 1 : j]);
            S0.insert(S0.end(), Tmp.begin(), Tmp.end());
            if (r % 2 == 0 && j == r)
                j = n - r;
            Tmp = createBooleanMatrix(n, j + 1, c[j > r ? n - j: j + 1]);
            S1.insert(S1.end(), Tmp.begin(), Tmp.end());
        }
        auto Tmp = createBooleanMatrix(n, n, c[1]);
        S0.insert(S0.end(), Tmp.begin(), Tmp.end());
    }
    encodeAndSaveImages(image, S0, S1, "D:\\Course_paper\\encrypted_imgs\\Map_25_13", n);
    
}


int main() {
    size_t n = 25;
    size_t k = 13;
    //constructVCS(n, k, visual_cypher("D:\\Course_paper\\map.jpg"));
    decodeImages("D:\\Course_paper\\encrypted_imgs\\Map_25_18", n, 1);
    return 0;
}
