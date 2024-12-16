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
    std::fill(row.begin(), row.begin() + i, 255);

    std::vector<std::vector<size_t>> matrix;
    do {
        matrix.push_back(row);
    } while (std::prev_permutation(row.begin(), row.end()));

    return matrix;
}


cv::Mat encodeImageOnce(const cv::Mat& image,
    const std::vector < std::vector < size_t>>& S0,
    const std::vector < std::vector < size_t>>& S1) {
    int rows = image.rows;
    int cols = image.cols;
    size_t columnLength = S0[0].size();
    int newWidth = cols * columnLength;

    // Результирующее изображение
    cv::Mat encodedImage(rows, newWidth, CV_8U, cv::Scalar(0));

    // Генератор случайных чисел
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distS0(0, S0.size() - 1);
    std::uniform_int_distribution<> distS1(0, S1.size() - 1);

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

    return encodedImage;
}

void saveEncodedImages(const cv::Mat& image,
    const std::vector < std::vector < size_t>>& S0,
    const std::vector < std::vector < size_t>>& S1,
    const std::string& folder_path,
    int n) {
    // Сохранение n зашифрованных изображений
    for (int i = 0; i < n; ++i) {
        cv::Mat encodedImage = encodeImageOnce(image, S0, S1);

        // Генерация имени файла
        std::string filename = folder_path + "\\encoded_image_" + std::to_string(i) + ".png";

        // Сохранение изображения
        if (!cv::imwrite(filename, encodedImage)) {
            std::cerr << "Ошибка: не удалось сохранить изображение " << filename << "!" << std::endl;
        }
        else {
            std::cout << "Изображение сохранено: " << filename << std::endl;
        }
    }
}
//std::vector<cv::Mat> getImages(const std::string& folder_path) {
//    std::vector<cv::Mat> images;
//    for (const auto& entry : std::experimental::filesystem::directory_iterator(folder_path)) {
//        if (std::experimental::filesystem::is_regular_file(entry)) {
//            // Получаем путь к файлу
//            std::string file_path = entry.path().string();
//
//            // Загружаем изображение с помощью OpenCV
//            cv::Mat image = cv::imread(file_path, cv::IMREAD_GRAYSCALE);
//
//            // Проверяем, успешно ли загрузилось изображение
//            if (!image.empty()) {
//                images.push_back(image);
//            }
//            else {
//                std::cerr << "Failed to load image: " << file_path << std::endl;
//            }
//        }
//    }
//    return images;
//}

//cv::Mat restoreImageFromEncoded(const std::string& folder_path,
//    const std::vector < std::vector < size_t>>& S0,
//    const std::vector < std::vector < size_t>>& S1,
//    int n) {
//    std::vector<cv::Mat> images = getImages(folder_path);
//    int rows = originalImage.rows;
//    int cols = originalImage.cols;
//    size_t columnLength = S0[0].size(); // Длина столбца (предполагается, что все столбцы одинаковы)
//
//    cv::Mat restoredImage = cv::Mat::zeros(rows, cols, CV_8U);
//
//    for (int i = 0; i < n; ++i) {
//        // Чтение зашифрованного изображения
//        std::string filename = folder_path + "/encoded_image_" + std::to_string(i) + ".png";
//        cv::Mat encodedImage = cv::imread(filename, cv::IMREAD_GRAYSCALE);
//
//        if (encodedImage.empty()) {
//            std::cerr << "Ошибка: не удалось прочитать изображение " << filename << std::endl;
//            continue;
//        }
//
//        // Восстановление данных
//        for (int row = 0; row < rows; ++row) {
//            for (int col = 0; col < cols; ++col) {
//                size_t columnStart = col * columnLength;
//                // Выбираем столбец в зависимости от того, черный или белый пиксель
//                bool isWhitePixel = originalImage.at<uchar>(row, col) == 255;
//                const std::vector<size_t>& selectedColumn =
//                    isWhitePixel ? S0[i] : S1[i];
//
//                // Восстановление пикселя
//                for (size_t j = 0; j < columnLength; ++j) {
//                    restoredImage.at<uchar>(row, col) = static_cast<uchar>(encodedImage.at<uchar>(row, columnStart + j));
//                }
//            }
//        }
//    }
//
//    return restoredImage;
//}

//int main() {
//    // Пример использования
//    cv::Mat image= cv::imread("input_image.png", cv::IMREAD_GRAYSCALE);
//
//    // Пример данных S0 и S1
//    std::vector < std::vector < size_t>> S0 = { {1, 2, 3}, {4, 5, 6} };
//    std::vector < std::vector < size_t>> S1 = { {7, 8, 9}, {10, 11, 12} };
//    std::string folder_path = "encoded_images"; // Папка для сохранения зашифрованных изображений
//
//    // Сохранить зашифрованные изображения
//    saveEncodedImages(image, S0, S1, folder_path, 5); // Кодируем 5 раз
//
//    // Восстановить изображение
//    cv::Mat restoredImage = restoreImageFromEncoded(folder_path, S0, S1, 5, image);
//
//    // Сохранить восстановленное изображение
//    cv::imwrite("restored_image.png", restoredImage);
//
//    return 0;
//}

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
    saveEncodedImages(image, S0, S1, "D:\\encrypted_imgs", n);
}


int main() {
    size_t n = 17;
    size_t k = 10;
    size_t r = k / 2;
    constructVCS(n, k, visual_cypher("D:\\Course_paper\\table.png"));
    return 0;
}
