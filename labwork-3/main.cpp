#include <iostream>
#include <fstream>

using namespace std;

const int fileHeaderSize = 14;
const int informationHeaderSize = 40;
const int maxColourIntensity = 255;
const int numberOfColorsInPallete = 5;
const int bitsPerPixel = 8;

void FileCreation(uint16_t length, uint16_t width, uint64_t** Sand, string& name) {
    uint64_t PaddingSize = (4 - length % 4) % 4;
    FILE *imageFile;
    char *cName = const_cast<char *>(name.data());
    imageFile = fopen(cName, "wb");
    uint64_t fileSize = fileHeaderSize + informationHeaderSize + (width * (length + PaddingSize)); // magic number

    // fileHeader
    static unsigned char fileHeader[] = {
            0, 0, // BM
            0, 0, 0, 0, // File size
            0, 0, 0, 0, // magic line
            0, 0, 0, 0, // data start
    };

    fileHeader[0] = 'B'; // for BMP
    fileHeader[1] = 'M'; // for BMP
    fileHeader[2] = fileSize; // first byte
    fileHeader[3] = fileSize >> 8; // second byte
    fileHeader[4] = fileSize >> 16; // third byte
    fileHeader[5] = fileSize >> 24; // fourth byte
    fileHeader[10] = 70; // data start from magic number


    fwrite(fileHeader, 1, 14, imageFile);

    static unsigned char dibHeader[] = {
            0, 0, 0, 0, // header size
            0, 0, 0, 0, // width
            0, 0, 0, 0, // length
            0, 0, // planes (IDK what is this)
            0, 0, // Bits per pixel
            0, 0, 0, 0, // compression (we don't use this)
            0, 0, 0, 0, // image size
            0, 0, 0, 0, // horizontal resolution
            0, 0, 0, 0, // vertical resolution
            0, 0, 0, 0, // number of colors in palette
            0, 0, 0, 0, //number of important colors used
            // channels blue, green, red
            0, 0, 0, 0, // colour 0 // white
            0, 0, 0, 0, // colour 1 // purple
            0, 0, 0, 0, // colour 2 // yellow
            0, 0, 0, 0, // colour 3 // green
            0, 0, 0, 0, // colour 4 // black
    };

    dibHeader[0] = 40;
    dibHeader[4] = length;
    dibHeader[5] = length >> 8;
    dibHeader[6] = length >> 16;
    dibHeader[7] = length >> 24;
    dibHeader[8] = width;
    dibHeader[9] = width >> 8;
    dibHeader[10] = width >> 16;
    dibHeader[11] = width >> 24;
    dibHeader[12] = 1;
    dibHeader[14] = bitsPerPixel;
    dibHeader[32] = numberOfColorsInPallete;
    dibHeader[40] = maxColourIntensity; // white
    dibHeader[41] = maxColourIntensity; // white
    dibHeader[42] = maxColourIntensity; // white
    dibHeader[44] = maxColourIntensity; // purple
    dibHeader[46] = maxColourIntensity; // purple
    dibHeader[49] = maxColourIntensity; // yellow
    dibHeader[50] = maxColourIntensity; // yellow
    dibHeader[53] = maxColourIntensity; // green



    fwrite(dibHeader, 1, 56, imageFile);

    unsigned char padding[3] = {0, 0, 0};

    for (uint16_t i = 0; i < width; i++) {
        for (uint16_t j = 0; j < length; j++) {
            unsigned char c[1];
            if (Sand[i][j] == 0) {
                c[0] = {0};
            } else if (Sand[i][j] == 1) {
                c[0] = {3};
            } else if (Sand[i][j] == 2) {
                c[0] = {1};
            } else if (Sand[i][j] == 3) {
                c[0] = {2};
            } else if (Sand[i][j] > 3) {
                c[0] = {4};
            }
            fwrite(c, 1, 1, imageFile); // writing our bytes
        }
        fwrite(padding, 1, PaddingSize, imageFile);
    }
    fclose(imageFile);
}

void TSVReading(const string& input, uint64_t** Sand) {
    std::ifstream f;
    f.open(input);
    int Coordinate_X;
    int Coordinate_Y;
    int amount_of_sand;
    while (!(f.eof())) {
        int massive[3] = {0, 0, 0};
        std::string buff_line;
        for (int &i: massive) {
            std::getline(f, buff_line, '\t');
            i = std::stoi(buff_line);
        }
        Coordinate_X = massive[0];
        Coordinate_Y = massive[1];
        amount_of_sand = massive[2];
        Sand[Coordinate_X][Coordinate_Y] = amount_of_sand;
    }
}

string CreateName(const string &output, uint64_t frequency) {
    std::string name;
    if (!output.empty()) {
        name = output + "/" + std::to_string(frequency) + ".bmp";
    } else {
        name = std::to_string(frequency) + ".bmp";
    }
    return name;
}

void SandFall(uint64_t** Sand, uint16_t i, uint16_t j, uint16_t length, uint16_t width, uint64_t temp){
    if (i - 1 >= 0) {
        Sand[i - 1][j] += temp / 4;
    }
    if (i + 1 < length) {
        Sand[i + 1][j] += temp / 4;
    }
    if (j + 1 < width) {
        Sand[i][j + 1] += temp / 4;
    }
    if (j - 1 >= 0) {
        Sand[i][j - 1] += temp / 4;
    }
    Sand[i][j] = temp % 4;
}

void IterSandFall(uint16_t length, uint16_t width, uint64_t** Sand, uint64_t& max_iter, int& freq, int frequency, uint64_t& file_index, string output, uint64_t& count) {
    for (uint16_t i = 0; i < length; i++) {
        for (uint16_t j = 0; j < width; j++) {
            uint64_t temp = Sand[i][j];
            if (temp >= 4 & max_iter > 0) {
                freq += 1;
                SandFall(Sand, i, j, length, width, temp);
                count += 1;
            }
            if (freq == frequency & frequency != 0) {
                string name = CreateName(output, file_index);
                FileCreation(length, width, Sand, name);
                file_index += 1;
                freq = 0;
            }
        }
    }
    max_iter -= 1;
}

int main(int argc, char *argv[]) {
    uint16_t length;
    uint16_t width;
    uint64_t max_iter;
    int frequency;
    string output;
    int freq = 0;
    uint64_t file_index = 0;
    uint64_t count = 0;
    string input;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--length") == 0) {
            length = std::stoi(argv[++i]);
        }
        if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--width") == 0) {
            width = std::stoi(argv[++i]);
        }
        if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            input = argv[++i];
        }
        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            output = argv[++i];
        }
        if (std::strcmp(argv[i], "-m") == 0 || std::strcmp(argv[i], "--max-iter") == 0) {
            max_iter = stoi(argv[++i]);
        }
        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--freq") == 0) {
            frequency = stoi(argv[++i]);
        }
    }

    //Sand creation
    auto** Sand = new uint64_t *[length];
    for (int i = 0; i < length; i++) {
        Sand[i] = new uint64_t[width];
    }
    for (uint16_t i = 0; i < length; i++) {
        for (uint16_t j = 0; j < width; j++) {
            Sand[i][j] = 0;
        }
    }

    TSVReading(input, Sand);
    //Sand fall
    bool flag = true;
    uint64_t iteration_counter = 0;
    while (flag  ){
        count = 0;
        IterSandFall(length, width, Sand, max_iter, freq, frequency, file_index, output, count);
        iteration_counter ++;
        if (count > 0) {
            flag = true;
        } else if(count == 0 && frequency == 0) {
                string name = CreateName(output, file_index);
                FileCreation(length, width, Sand, name);
                flag = false;
            }
        else{
            flag = false;
        }
    }

    //Clearing of Sand array
    for (int i=0; i<length; i++) delete [] Sand[i];
    delete[](Sand);
    if(frequency == 0){
        cout << "File was created!" << endl;
    }
    else{
        cout << "Files was created!" << endl;
    }
    return 0;
}