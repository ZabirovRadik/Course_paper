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
    if (image.empty() || !is_black_white(image)) {
        throw std::invalid_argument("Not black and white!");
    }
    cv::Mat grayImage;
    cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
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
    std::vector<size_t> row(n, 0);
    std::fill(row.begin(), row.begin() + i, c);

    std::vector<std::vector<size_t>> matrix;
    do {
        matrix.push_back(row);
    } while (std::prev_permutation(row.begin(), row.end()));

    return matrix;
}




std::vector<std::vector<std::vector<size_t>>> encryptImage(
    const cv::Mat& image,
    std::vector< std::vector<size_t>>& S0,
    std::vector< std::vector<size_t>>& S1,
    size_t n, size_t k)
{
    // Получаем размеры изображения
    size_t rows = image.rows;
    size_t cols = image.cols;

    // Разбиваем изображение на n подизображений
    std::vector<std::vector<std::vector<size_t>>> encrypted_image(n);

    // Итерируемся по каждому подизображению
    for (size_t i = 0; i < n; ++i) {
        encrypted_image[i].resize(k);
        for (size_t j = 0; j < k; ++j) {
            // Получаем размеры подизображения
            size_t sub_rows = rows / n;
            size_t sub_cols = cols / n;

            // Извлекаем подизображение
            cv::Mat sub_image = image(cv::Rect(i * sub_cols, j * sub_rows, sub_cols, sub_rows));

            // Преобразуем подизображение в вектор пикселей
            std::vector<size_t> pixels(sub_rows * sub_cols);
            for (size_t y = 0; y < sub_rows; ++y) {
                for (size_t x = 0; x < sub_cols; ++x) {
                    pixels[y * sub_cols + x] = sub_image.at<uchar>(y, x) > 0 ? 1 : 0;
                }
            }

            // Кодируем подизображение с помощью матриц S0 и S1
            if (pixels.size() == S0[j].size()) {
                encrypted_image[i][j].resize(S0[j].size());
                for (size_t p = 0; p < pixels.size(); ++p) {
                    encrypted_image[i][j][p] = pixels[p] * S0[j][p] + (1 - pixels[p]) * S1[j][p];
                }
            }
        }
    }

    return encrypted_image;
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
    encryptImage(image, S0, S1, n, k);
}




int main(){
    size_t n = 17;
    size_t k = 10;
    size_t r = k / 2;
    constructVCS(n, k, visual_cypher("D:\\Course_paper\\table.png"));
    return 0;
}