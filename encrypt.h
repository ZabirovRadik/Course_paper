#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <vector>
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


cv::Mat visual_cypher(const std::string& path) {
    cv::Mat image = cv::imread(path);
    if (image.empty()) {
        throw std::invalid_argument("Empty!");
    }
    cv::Mat gray_image;
    cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
    cv::threshold(gray_image, gray_image, 127, 255, cv::THRESH_BINARY);
    return gray_image;
}


size_t binomial(size_t n,
    size_t k)
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


std::vector<std::vector<size_t>> create_boolean_matrix(size_t n,
    size_t i,
    size_t c) {
    std::vector<size_t> col(n, 255);
    std::fill(col.begin(), col.begin() + i, 0);

    std::vector<std::vector<size_t>> matrix;
    do {
        for (size_t i = 0; i < c; ++i)
            matrix.push_back(col);
    } while (std::prev_permutation(col.begin(), col.end()));
    return matrix;
}


void encode_and_save_images(const cv::Mat& image,
    const std::vector<std::vector<size_t>>& S0,
    const std::vector<std::vector<size_t>>& S1,
    const std::string& path_to_encoded_imgs,
    int n) {
    int rows = image.rows;
    int cols = image.cols;
    size_t column_length = S0[0].size();
    int newWidth = cols * column_length;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distS0(0, S0.size() - 1);
    std::uniform_int_distribution<> distS1(0, S1.size() - 1);

    for (int i = 0; i < n; ++i) {
        cv::Mat encodedImage(rows, newWidth, CV_8U, cv::Scalar(0));
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                const std::vector<size_t>& selected_ñolumn =
                    (image.at<uchar>(row, col) == 255)
                    ? S0[distS0(gen)]
                    : S1[distS1(gen)];

                for (size_t i = 0; i < selected_ñolumn.size(); ++i) {
                    encodedImage.at<uchar>(row, col * column_length + i) =
                        static_cast<uchar>(selected_ñolumn[i]);
                }
            }
        }
        std::string filename = path_to_encoded_imgs + std::to_string(i) + ".jpg";

        if (!cv::imwrite(filename, encodedImage)) {
            std::cerr << "Error: can't save image " << filename << "!" << std::endl;
        }
        else {
            std::cout << "Image saved: " << filename << std::endl;
        }
    }
}


void encrypt_image(size_t n,
    size_t k,
    const std::string& path_to_image,
    const std::string& path_to_encoded_imgs) {

    cv::Mat gray_image = visual_cypher(path_to_image);
    size_t r = k / 2;
    bool is_odd = k % 2;
    if (n <= k || n - r - 1 < 1)
        return;
    std::vector<size_t> c(r + 1);
    for (size_t j = 0; j <= r; ++j) {
        c[j] = binomial(n - r - is_odd - j, r - j);
    }

    std::vector <std::vector<size_t>> S0, S1;
    if (is_odd) {
        for (int j = 0; j < n; j += 2) {
            if (r % 2 && j == r + 1)
                j = n - r;
            auto Tmp1 = create_boolean_matrix(n, j, c[j > r ? n - j : j]);
            S0.insert(S0.end(), Tmp1.begin(), Tmp1.end());
            if (r % 2 == 0 && j == r)
                j = n - r - 1;
            auto Tmp2 = create_boolean_matrix(n, j + 1, c[j > r ? n - j - 1 : j + 1]);
            S1.insert(S1.end(), Tmp2.begin(), Tmp2.end());
        }
    }
    else {
        for (int j = 0; j < n; j += 2) {
            if (r % 2 && j == r + 1)
                j = n - r + 1;
            auto Tmp1 = create_boolean_matrix(n, j, c[j > r ? n - j + 1 : j]);
            S0.insert(S0.end(), Tmp1.begin(), Tmp1.end());
            if (r % 2 == 0 && j == r)
                j = n - r;
            auto Tmp2 = create_boolean_matrix(n, j + 1, c[j > r ? n - j : j + 1]);
            S1.insert(S1.end(), Tmp2.begin(), Tmp2.end());
        }
        auto Tmp = create_boolean_matrix(n, n, c[1]);
        S0.insert(S0.end(), Tmp.begin(), Tmp.end());
    }
    encode_and_save_images(gray_image, S0, S1, path_to_encoded_imgs, n);
}