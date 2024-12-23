#include <iostream>
#include <filesystem>

const std::vector<cv::Mat>& jpgs_from_folder(std::vector<cv::Mat>& encoded_images, const std::string& folder) {
    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        if (entry.path().extension() == ".jpg") {
            cv::Mat encoded_image = cv::imread(entry.path().string(), cv::IMREAD_GRAYSCALE);
            if (encoded_image.empty()) {
                std::cerr << "Error with " << entry.path().string() << "!" << std::endl;
                continue;
            }
            encoded_images.push_back(encoded_image);
        }
    }
    return encoded_images;
}


cv::Mat decode_images(
    int n,
    const std::string& folder,
    const std::string& path_to_decoded_file) {

    std::vector<cv::Mat> encoded_images;
    jpgs_from_folder(encoded_images, folder);

    size_t num_imgs = encoded_images.size();
    int rows = encoded_images[0].rows;
    int cols = encoded_images[0].cols;

    cv::Mat decoded_image(rows, cols / n, CV_8UC1, cv::Scalar(0));

    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; x += n) {
            int sum = 0;
            for (int xi = x; xi < x + n; ++xi) {
                for (int i = 0; i < num_imgs; ++i) {
                    sum += encoded_images[i].at<uchar>(y, xi);
                }
            }
            decoded_image.at<uchar>(y, x / n) = static_cast<uchar>(sum / n / num_imgs >= 255 / 2 ? 255 : 0);
        }
    }
    if (!cv::imwrite(path_to_decoded_file, decoded_image)) {
        std::cerr << "Error: can't write " << path_to_decoded_file << "!" << std::endl;
    }
    else {
        std::cout << "Complete: " << path_to_decoded_file << std::endl;
    }
    return decoded_image;
}

