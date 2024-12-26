#include <iostream>
#include <filesystem>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>

const std::vector<cv::Mat>& read_images_from_folder(std::vector<cv::Mat>& encrypted_images, const std::filesystem::path& folder) {
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        cv::Mat encoded_image = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE); // < add binarization
        cv::threshold(encoded_image, encoded_image, 127, 255, cv::THRESH_BINARY);
        if (encoded_image.empty()) {
            throw std::invalid_argument(" Failed to read image from " + entry.path().string());
            continue;
        }
        encrypted_images.push_back(encoded_image);
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
    int n,
    const std::filesystem::path& folder, 
    const std::string& path_to_decrypted_file) {

    std::vector<cv::Mat> encrypted_images;
    read_images_from_folder(encrypted_images, folder);

    size_t num_imgs = encrypted_images.size();
    int rows = encrypted_images[0].rows;
    int cols = encrypted_images[0].cols;

    cv::Mat decrypted_images(rows, cols / n, CV_8UC1, cv::Scalar(0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; x += n) {
            int sum = 0;
            for (int xi = x; xi < x + n; ++xi) {
                for (int i = 0; i < num_imgs; ++i) {
                    sum += encrypted_images[i].at<uchar>(y, xi);
                }
            }
            decrypted_images.at<uchar>(y, x / n) = static_cast<uchar>(sum / n / num_imgs >= 255 / 2 ? 255 : 0);
        }
    }
    if (!cv::imwrite(path_to_decrypted_file, decrypted_images)) {
        std::cerr << "Error: can't write " << path_to_decrypted_file << "!" << std::endl;
    }
    else {
        std::cout << "Complete: " << path_to_decrypted_file << std::endl;
    }
    return decrypted_images;
}