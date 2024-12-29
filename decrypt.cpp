#include <iostream>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

const std::vector<cv::Mat>& read_images_from_folder(std::vector<cv::Mat>& encrypted_images, const std::filesystem::path& folder) {
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        cv::Mat encrypted_image = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);
        cv::threshold(encrypted_image, encrypted_image, 127, 255, cv::THRESH_BINARY);
        if (encrypted_image.empty()) {
            throw std::invalid_argument(" Failed to read image from " + entry.path().string());
            continue;
        }
        encrypted_images.push_back(encrypted_image);
    }
    if (encrypted_images.empty()) {
        throw std::invalid_argument(" No images were read");
        exit(-1);
    }
    cv::Size firstSize = encrypted_images[0].size();
    for (const auto& img : encrypted_images) {
        if (img.size() != firstSize)
            throw std::invalid_argument(" The sizes of the images are different, decoding is impossible");
    }
    return encrypted_images;
}


cv::Mat decrypt_images(
    int m,
    const std::filesystem::path& folder, 
    const std::string& path_to_decrypted_file) {

    std::vector<cv::Mat> encrypted_images;
    read_images_from_folder(encrypted_images, folder);

    size_t num_imgs = encrypted_images.size();
    int rows = encrypted_images[0].rows;
    int cols = encrypted_images[0].cols;
    int o = 0;
    cv::Mat decrypted_image(rows, cols / m, CV_8U, cv::Scalar(255));
    for (size_t y = 0; y < rows; ++y) {
        for (size_t x = 0; x < cols; ++x) {
            for (size_t i = 1; i < encrypted_images.size(); ++i) {
                if (encrypted_images[0].at<uchar>(y, x) != encrypted_images[i].at<uchar>(y, x))
                    encrypted_images[0].at<uchar>(y, x) = 255;
                else
                    encrypted_images[0].at<uchar>(y, x) = 0;
            }
        }
        for (size_t x = 0; x < cols / m; ++x) {
            size_t sum = 0;
            for (size_t i = 0; i < m; ++i)
                sum += encrypted_images[0].at<uchar>(y, x * m + i);
            decrypted_image.at<uchar>(y, x) = sum / m < 127 ?
                255 :
                0;
        }
    }
    decrypted_image.convertTo(decrypted_image, CV_8U);
    cv::imwrite("oooo.png", encrypted_images[0]);
    if (!cv::imwrite(path_to_decrypted_file, decrypted_image)) {
        std::cerr << "Error: can't write " << path_to_decrypted_file << "!" << std::endl;
    }
    else {
        std::cout << "Complete: " << path_to_decrypted_file << std::endl;
    }
    return decrypted_image;
}