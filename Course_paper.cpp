#include "encrypt.h"
#include "filesystem"
#include "decrypt.h"

int main(int argc, char* argv[]) {
    if (std::string(argv[1]) == "encrypt") {
        if (argc != 7) {
            std::cerr << "Usage: " << argv[0] << "  Incorrect number of arguments" << argc << std::endl;
            return 1;
        } 
        encrypt_image(
            std::stoul(argv[2]), //n
            std::stoul(argv[3]), //k
            argv[4],             //image path
            argv[5],             //folder to save encoded images
            argv[6]);            //start name of encoded files
    }
    else if (std::string(argv[1]) == "decrypt") {
        if (argc != 5) {
            std::cerr << "Usage: " << argv[0] << "  Incorrect number of arguments" << argc << std::endl;
            return 1;
        }
        decode_images(
            std::stoul(argv[2]),  // n
            argv[3],              // folder of encoded files
            argv[4]               // path to result
        );
    }
    else {
        std::cerr << "Invalid action: " << argv[1] << std::endl;
        return 1;
    }
    return 0;
}