#include "encrypt.h"
#include "filesystem"
#include "decrypt.h"

int main(int argc, char* argv[]) {
    if (argc != 6) {
        std::cerr << "Usage: " << argv[0] << " <action> <number>" << std::endl;
        return 1;
    }
    if (std::string(argv[1]) == "encrypt") {
                    //    n              |    k            | image_path | path_to_encoded_imgs         
        encrypt_image(std::stoul(argv[2]), std::stoul(argv[3]), argv[4], argv[5]);
    }
    else if (std::string(argv[1]) == "decrypt") {
                      //        n         |     num_imgs      |folder_imgs| path_te_decrypted     
        decode_images(std::stoul(argv[2]), std::stoul(argv[3]), argv[4], argv[5]);
    }
    else {
        std::cerr << "Invalid action: " << argv[1] << std::endl;
        return 1;
    }
    return 0;
}