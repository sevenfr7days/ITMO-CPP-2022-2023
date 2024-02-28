#include "frames.h"

void getName(std::string &name) {
    std::cout << "Enter name of file or path to parse meta data info " << std::endl;
    std::cin >> name;
    std::ifstream file(name);
    if (!file.is_open()) {
        std::cerr << "Wrong name or path to file, try again... " << std::endl;
    } else {
        std::cout << "Great, here's meta data of this file!" << std::endl;
    }
}

std::string readFlags(char byte) {
    std::bitset<8> binary(byte);
    return binary.to_string();
}

void readID3v2Header(std::ifstream &file, ID3v2Header &header) {
    char size_buff[kSizeBuffSize];
    char flags_buff;
    file.read(header.tag, kHeaderTagSize);
    file.read(header.version, kHeaderVersionSize);
    file.read((char *) &flags_buff, kOneByte);
    file.read((char *) &size_buff, kSizeBuffSize);
    header.flags = readFlags(flags_buff);
    //destroy first bit in all bytes
    header.size = (size_buff[0] << 21) | (size_buff[1] << 14) | (size_buff[2] << 7) | (size_buff[3]);
}

//
void readID3v2FrameHeader(std::ifstream &file, std::string &frame_name, std::string &frame_flags, int &frame_size) {
    char id_buff[kIDBuffSize + kOneByte];
    char size_buff[kSizeBuffSize];
    char flags_buff[kOneByte * 2];
    file.read(id_buff, kIDBuffSize);
    file.read(size_buff, kSizeBuffSize);
    file.read(flags_buff, kOneByte * 2);
    id_buff[4] = '\0';
    frame_name = id_buff;
    frame_flags += readFlags(flags_buff[0]);
    frame_flags += readFlags(flags_buff[1]);
    frame_size = (size_buff[0] << 21) | (size_buff[1] << 14) | (size_buff[2] << 7) | (size_buff[3]);
}