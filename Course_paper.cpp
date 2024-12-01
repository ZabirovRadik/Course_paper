//! [includes]
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <vector>
#include <deque>
#include <random>
#include <ctime>

#include <iostream>

//! [includes]

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
    //cv::namedWindow("Image", cv::WINDOW_NORMAL);
    //cv::imshow("Image", grayImage);
    //cv::waitKey(0);
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


std::vector<size_t> createBooleanMatrix(size_t n, size_t i, size_t c) {
    size_t num_columns = binomial(n, i);
    std::vector<size_t> matrix;

    std::vector<int> vec(n, 0);
    std::fill(vec.begin(), vec.begin() + i, 1);

    do {
        matrix.insert(matrix.end(), vec.begin(), vec.end());
    } while (std::prev_permutation(vec.begin(), vec.end()));

    return matrix;
}


std::vector < std::vector < std::vector<size_t>>> encryptImage(
    const cv::Mat& image,
    const std::deque <std::vector < size_t>>& S0,
    const std::deque <std::vector < size_t>>& S1,
    size_t n, size_t k
)
{
    size_t rows = image.rows;
    size_t cols = image.cols;

    // Создаём n долей
    std::vector < std::vector < std::vector<size_t>>> shares(n, std::vector < std::vector < size_t>>(rows, std::vector<size_t>(cols)));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            // Выбор матрицы S0 или S1 в зависимости от пикселя
            std::vector < std::vector <size_t>> S;
            if (image.at<uchar>(i, j))
                for (size_t share = 0; share < n; ++share) {
                    shares[share][i][j] = S1[share][j];
                }
            else
                for (size_t share = 0; share < n; ++share) {
                    shares[share][i][j] = S0[share][j];
                }


            // Распределяем бит из столбца между долями

        }
    }
    return shares;
}

void k_odd(size_t n, size_t k, cv::Mat image) {

}

void constructVCS(size_t n, size_t k, cv::Mat image) {
    size_t r = k / 2;
    std::vector<size_t> c;
    std::deque<size_t> is0, is1;
    for (size_t j = 0; j <= r; ++j)
        c.push_back(binomial(n - r - 1 - j, r - j));
    std::deque < std::vector<size_t>> S0, S1;      // Построение матриц S^0 и S^1
    if (k % 2) { 
        for (int j = 0; j < n; j += 2) {
            if (r%2 && j == r + 1) {
                if (n - r + 1 < n)
                    j = n - r + 1;
                else
                    break;
            }
            S0.push_back(createBooleanMatrix(n, j, c[j > r ? r - (j - 2) % r : j]));
            is0.push_back(j > r ? r - (j + 2) % r : j);
            if (r % 2 == 0 && j == r) {
                if (n - r - 1 < n)
                    j = n - r - 1;
                else
                    break;
            }
            S1.push_back(createBooleanMatrix(n, j + 1, c[j > r ? r - (j - 1) % r : j + 1]));
            is1.push_back(j > r ? r - (j + 1) % r : j + 1);
        }
        if (r == 1) {
            S0.push_back(createBooleanMatrix(n, n, c[1]));
            is0.push_back(1);
        }
    }
    else {
        for (int j = 0; j < n; j += 2) {
            if (r % 2 && j == r + 1) {
                if (n - r + 1 < n)
                    j = n - r + 1;
                else
                    break;
            }
            S0.push_back(createBooleanMatrix(n, j, c[j > r ? r - (j + 2) % r : j]));
            is0.push_back(j > r ? r - (n + 1 - j) % r : j);
            if (r % 2 == 0 && j == r) {
                if (n - r < n)
                    j = n - r;
                else
                    break;
            }
            S1.push_back(createBooleanMatrix(n, j + 1, c[j > r ? r - (j + 1) % r : j + 1]));
            is1.push_back(j > r ? r - (n + 2 - j) % r : j + 1);
        }
        S0.push_back(createBooleanMatrix(n, n, c[1]));
        is0.push_back(1);   
    }
    /*for (size_t i = 0; i < S0.size(); ++i) {
        for (size_t j = 0; j < S0[i].size(); ++j) {
            std::cout << S0[i][j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "S1" << std::endl;
    for (size_t i = 0; i < S1.size(); ++i) {
        for (size_t j = 0; j < S1[i].size(); ++j) {
            std::cout << S1[i][j] << " ";
        }
        std::cout << std::endl;
    }*/
    for (auto lo : is0)
        std::cout << lo << " ";
    std::cout << std::endl;
    for (auto lo : is1)
        std::cout << lo << " ";
    encryptImage(image, S0, S1, n, k);
}




int main(){
    size_t n = 17;
    size_t k = 10;
    size_t r = k / 2;
    constructVCS(n, k, visual_cypher("D:\\Course_paper\\table.png"));
    return 0;
}