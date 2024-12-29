#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <random>
#include <numeric>
#include <iostream>


cv::Mat read_binary(const std::string& path) {
    cv::Mat image = cv::imread(path);
    if (image.empty()) {
        throw std::invalid_argument("Path does not exists or is not an image");
    }
    cv::Mat gray_image;
    cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray_image, gray_image, cv::Size(5, 5), 0);
    cv::adaptiveThreshold(gray_image, gray_image, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 11, 2);
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


std::vector<std::vector<size_t>> create_boolean_matrix(size_t n, size_t i, size_t c) {
    std::vector<size_t> row(n, 0);
    std::fill(row.end() - i, row.end(), 255);

    std::vector<std::vector<size_t>> matrix;
    do {
        for (size_t i = 0; i < c; ++i)
            matrix.push_back(row);
    } while (std::next_permutation(row.begin(), row.end()));
    return matrix;
}


bool make_folder(const std::string& folder) {
    if (!std::filesystem::exists(folder)) {
        try {
            std::filesystem::create_directory(folder);
            std::cout << "Folder created: " << folder << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            throw std::invalid_argument(" Error when creating a folder: " + folder);
        }
    }

    return false;
}


void encrypt_and_save_images(const cv::Mat& image,
    std::vector<std::vector<size_t>>& S0,
    std::vector<std::vector<size_t>>& S1,
    const std::string& folder,
    size_t n,
    const std::string& name_files
) {
    cv::imwrite("D:\\Course_paper\\nnnn.png", image);
    make_folder(folder);
    size_t rows = image.rows;
    size_t cols = image.cols;
    size_t string_len = S0.size();
    size_t new_width = cols * string_len;
    std::vector<size_t> indices_s(S0.size());
    std::iota(indices_s.begin(), indices_s.end(), 0);
    std::mt19937 gen(1444);
    std::vector<cv::Mat> encrypted_images;
    size_t c = 0;
    for (size_t i = 0; i < n; ++i)
        encrypted_images.push_back(cv::Mat::zeros(rows, new_width, CV_8U));
    for (size_t row = 0; row < rows; ++row) {
        for (size_t col = 0; col < cols; ++col) {
            std::shuffle(indices_s.begin(), indices_s.end(), gen);
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < string_len; ++j) {
                    encrypted_images[i].at<uchar>(row, col * string_len + j) =
                        static_cast<uchar>((image.at<uchar>(row, col) == 255)
                            ? S0[indices_s[j]][i]
                            : S1[indices_s[j]][i]);
                }
            }

        }
    }
    std::cout << std::endl;
    std::cout << string_len;
    std::cout << std::endl;
    for (size_t i = 0; i < n; ++i) {
        std::string filename = folder + "\\" + name_files + std::to_string(i) + ".png";

        if (!cv::imwrite(filename, encrypted_images[i])) {
            std::cerr << "Error: can't save image " << filename << "!" << std::endl;
        }
        else {
            std::cout << "Image saved: " << filename << std::endl;
        }
    }
}


void encrypt_image(size_t n,
    size_t k,
    const std::string& image_path,
    const std::string& folder,
    const std::string& files_names
) {

    cv::Mat gray_image = read_binary(image_path);
    size_t r = k / 2;
    bool is_odd = k % 2;
    if (n < k || n - r - 1 < 1)
        return;
    std::vector<size_t> c(r + 1);
    for (size_t j = 0; j <= r; ++j) {
        c[j] = binomial(n - r - is_odd - j, r - j);
    }
    std::vector<std::vector<size_t>> S0, S1;
        if (is_odd) {
        for (int j = 0; j < n; j += 2) {
            if (r % 2 && j == r + 1)
                j = n - r;
            auto Tmp1 = create_boolean_matrix(n, j, c[j > r ? n - j : j]);
            S0.insert(S0.end(), Tmp1.begin(), Tmp1.end());
            if (r % 2 == 0 && j == r)
                j = n - r - 1;
            if (j >= n)
                break;
            auto Tmp2 = create_boolean_matrix(n, j + 1, c[j > r ? n - j - 1 : (j + 1)%c.size()]);
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
            if (j >= n)
                break;
            auto Tmp2 = create_boolean_matrix(n, n < j + 1 ? n : j + 1, c[j > r ? n - j : j + 1]);
            S1.insert(S1.end(), Tmp2.begin(), Tmp2.end());
        }
    }
    
    std::cout << std::endl;
    for (size_t i = 0; i < S0[0].size(); ++i) {
        for (size_t j = 0; j < S0.size(); ++j)
            std::cout << S0[j][i]<< " ";
        std::cout << std::endl;
    }
    encrypt_and_save_images(gray_image, S0, S1, folder, n, files_names);
}