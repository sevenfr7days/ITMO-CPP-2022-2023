#include "HelpFuncs.cpp"


void ParseTFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    char frame_encoding;
    char text_encoding;
    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    file.read(&text_encoding, kOneByte);

    int frame_information_size = frame_size - kOneByte;
    char frame_information[frame_information_size + kOneByte];
    file.read(frame_information, frame_information_size);
    frame_information[frame_information_size] = '\0';

    frame_encoding = text_encoding[0];
    TFrame TFrame(frame_name, frame_flags, frame_size, frame_encoding, frame_information);
    Storage.TFrame_vector.emplace_back(TFrame);
}

void ParseTXXXFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    std::string frame_description;
    int frame_size;
    char frame_encoding;
    char text_encoding[kOneByte + kOneByte];

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    file.read(text_encoding, kOneByte);
    text_encoding[1] = '\0';

    getline(file, frame_description, '\0');
    int frame_info_size = frame_size - kOneByte - frame_description.size() - kOneByte;
    char frame_info[frame_info_size + kOneByte];
    file.read(frame_info, frame_info_size);
    frame_info[frame_info_size] = '\0';

    frame_encoding = text_encoding[0];
    TXXXFrame TXXXFrame(frame_name, frame_flags, frame_size, frame_encoding,
                        frame_info, frame_description);
    Storage.TXXXFrame_vector.emplace_back(TXXXFrame);
}

void ParseCOMMFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    std::string short_text_description;
    std::string actual_text;
    char text_encoding[kOneByte];
    char language[kLanguageBuffSize + kOneByte];
    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    file.read(text_encoding, kOneByte);
    file.read(language, 3);
    language[3] = '\0';
    getline(file, short_text_description, '\0');
    int bytes_left = frame_size - kOneByte - kLanguageBuffSize - short_text_description.size() - kOneByte;
    char temp_actual_text[bytes_left];
    file.read(temp_actual_text, bytes_left);
    for (int i = 0; i < bytes_left; ++i) {
        actual_text += temp_actual_text[i];
    }
    COMMFrame COMMFrame(frame_name, frame_flags, frame_size, text_encoding[0],
                        language, short_text_description, actual_text);
    Storage.COMMFrame_vector.emplace_back(COMMFrame);
}

void ParseUFIDFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    std::string owner_identifier;
    std::string identifier;
    getline(file, owner_identifier, '\0');

    int bytes_left = frame_size - owner_identifier.size() - kOneByte;
    char temp_identifier[bytes_left];

    file.read(temp_identifier, bytes_left);
    UFIDFrame UFIDFrame(frame_name, frame_flags, frame_size, owner_identifier, identifier);
    Storage.UFIDFrame_vector.emplace_back(UFIDFrame);
}

void ParseUSERFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    char temp_text_encoding[kOneByte];
    char temp_language[kLanguageBuffSize];
    file.read(temp_text_encoding, kOneByte);
    file.read(temp_language, kLanguageBuffSize);
    int bytes_left = frame_size - kLanguageBuffSize - kOneByte;
    char temp_actual_text[bytes_left];
    file.read(temp_actual_text, bytes_left);
    USERFrame USERFrame(frame_name, frame_flags, frame_size, temp_text_encoding[0],
                        temp_language, temp_actual_text);
    Storage.USERFrame_vector.emplace_back(USERFrame);
}

void ParseWFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    char url_buff[frame_size + kOneByte];
    file.read(url_buff, frame_size);
    url_buff[frame_size] = '\0';
    WFrame WFrame(frame_name, frame_flags, frame_size, url_buff);
    Storage.WFrame_vector.emplace_back(WFrame);
}

void ParseWXXXFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char text_encoding_buff[kOneByte];
    std::string description;
    file.read(text_encoding_buff, kOneByte);
    getline(file, description, '\0');

    auto text_encoding = reinterpret_cast<uint16_t *>(text_encoding_buff[0]);
    int url_buf_size = frame_size - kOneByte - description.size();
    char url[url_buf_size + kOneByte];

    file.read(url, url_buf_size);
    url[url_buf_size] = '\0';

    WXXXFrame WXXXFrame(frame_name, frame_flags, frame_size, *text_encoding,
                        description, url);
    Storage.WXXXFrame_vector.emplace_back(WXXXFrame);
}

void ParseSEEKFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    char min_offset_buff[kMinOffsetBuffSize];
    file.read(min_offset_buff, kMinOffsetBuffSize);
    auto *min_offset = reinterpret_cast<uint32_t *>(min_offset_buff);
    SEEKFrame SEEKFrame(frame_name, frame_flags, frame_size, *min_offset);
    Storage.SEEKFrame_vector.emplace_back(SEEKFrame);
}

void ParseGRIDFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    std::string owner_id;
    char group_symbol[kOneByte];

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    getline(file, owner_id, '\0');
    file.read(group_symbol, kOneByte);

    int dependent_data_buff_size = frame_size - kOneByte - owner_id.size() - kOneByte;
    char dependent_data_buff[dependent_data_buff_size];

    file.read(dependent_data_buff, dependent_data_buff_size);
    GRIDFrame GRIDFrame(frame_name, frame_flags, frame_size, owner_id,
                        group_symbol[0], dependent_data_buff);
    Storage.GRIDFrame_vector.emplace_back(GRIDFrame);
}

void ParseRVA2Frame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    std::string identification;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    getline(file, identification, '\0');
    RVA2Frame RVA2Frame(frame_name, frame_flags, frame_size, identification);
    Storage.RVA2Frame_vector.emplace_back(RVA2Frame);
}

void ParseOWNEFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    std::string price_paid;
    char text_encoding_buff[kOneByte];
    char date_of_purch[kDateOfPurchSize];

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
    file.read(text_encoding_buff, kOneByte);
    getline(file, price_paid, '\0');
    file.read(date_of_purch, kDateOfPurchSize);

    int seller_size = frame_size - kDateOfPurchSize - kOneByte - price_paid.size();
    char seller[seller_size];

    file.read(seller, seller_size);
    auto *text_encoding = reinterpret_cast<uint16_t *>(text_encoding_buff);

    OWNEFrame OWNEFrame(frame_name, frame_flags, frame_size,
                        *text_encoding, price_paid, date_of_purch, seller);
    Storage.OWNEFrame_vector.emplace_back(OWNEFrame);
}

void ParseCOMRFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char text_encoding[kOneByte];
    std::string price_string;
    char valid_until[kValidUntilSize];
    std::string contact_url;
    char received_as[kOneByte];
    std::string seller_name;
    std::string description;
    std::string picture_mime_type;
    std::vector<char> seller_logo;
    char temp_char[kOneByte];
    std::fstream seller_logo_pic;
    seller_logo_pic.open("seller logo.jpg", std::fstream::out);

    file.read(text_encoding, kOneByte);
    getline(file, price_string, '\0');
    file.read(valid_until, kValidUntilSize);
    getline(file, contact_url, '\0');
    file.read(received_as, kOneByte);
    getline(file, seller_name, '\0');
    getline(file, description, '\0');
    getline(file, picture_mime_type, '\0');

    int seller_logo_size = frame_size - kOneByte - price_string.size() - kOneByte - kValidUntilSize -
                           contact_url.size() - kOneByte - kOneByte - seller_name.size() - kOneByte
                           - description.size()
                           - kOneByte - picture_mime_type.size() - kOneByte;

    for (int i = 0; i < seller_logo_size; ++i) {
        file.read(temp_char, kOneByte);
        seller_logo.emplace_back(temp_char[0]);
        seller_logo_pic.write(temp_char, 1);
    }

    COMRFrame COMRFrame(frame_name, frame_flags, frame_size, text_encoding[0],
                        price_string, valid_until,
                        contact_url, received_as[0], seller_name,
                        description, picture_mime_type, seller_logo);
    Storage.COMRFrame_vector.emplace_back(COMRFrame);
}

void ParseENCRFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    std::string owner_identifier;
    char method_symbol[kOneByte];
    std::vector<char> encryption_info;
    getline(file, owner_identifier, '\0');
    file.read(method_symbol, kOneByte);

    int encryption_data_size = frame_size - owner_identifier.size() - kOneByte - kOneByte;
    char temp_char[kOneByte];
    for (int i = 0; i < encryption_data_size; ++i) {
        file.read(temp_char, kOneByte);
        encryption_info.emplace_back(temp_char[0]);
    }

    ENCRFrame ENCRFrame(frame_name, frame_flags, frame_size, owner_identifier,
                        method_symbol[0], encryption_info);
    Storage.ENCRFrame_vector.emplace_back(ENCRFrame);
}

void ParseETCOFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char time_stamp_format[kOneByte];
    file.read(time_stamp_format, kOneByte);

    ETCOFrame ETCOFrame(frame_name, frame_flags, frame_size, time_stamp_format[0]);
    Storage.ETCOFrame_vector.emplace_back(ETCOFrame);
}

void ParseLINKFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    std::vector<char> frame_identifier;
    std::string url;
    char temp_char[kOneByte];

    for (int i = 0; i < kFrameIdentidierSize; ++i) {
        file.read(temp_char, kOneByte);
        frame_identifier.emplace_back(temp_char[0]);
    }
    getline(file, url, '\0');
    int id_and_additional_data_size = frame_size - kFrameIdentidierSize - url.size() - kOneByte;
    char id_and_additional_data[id_and_additional_data_size];
    file.read(id_and_additional_data, id_and_additional_data_size);

    LINKFrame LINKFrame(frame_name, frame_flags, frame_size,
                        frame_identifier, url, id_and_additional_data);
    Storage.LINKFrame_vector.emplace_back(LINKFrame);
}

void ParsePRIVFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    std::string owner_id;
    std::vector<char> private_data;
    getline(file, owner_id, '\0');
    int private_data_size = frame_size - owner_id.size() - kOneByte;
    char temp_char[kOneByte];

    for (int i = 0; i < private_data_size; ++i) {
        file.read(temp_char, kOneByte);
        private_data.emplace_back(temp_char[0]);
    }

    PRIVFrame PRIVFrame(frame_name, frame_flags, frame_size, owner_id, private_data);
    Storage.PRIVFrame_vector.emplace_back(PRIVFrame);
}

void ParsePCNTFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char pcnt_buff[frame_size];
    file.read(pcnt_buff, frame_size);
    auto *counter = reinterpret_cast<uint32_t *>(pcnt_buff);

    PCNTFrame PCNTFrame(frame_name, frame_flags, frame_size, *counter);
    Storage.PCNTFrame_vector.emplace_back(PCNTFrame);
}

void ParsePOPMFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    std::string email;
    char rating_buff[kOneByte];
    getline(file, email, '\0');
    file.read(rating_buff, kOneByte);

    int pcnt_buff_size = frame_size - email.size() - kOneByte - kOneByte;
    char pcnt_buff[pcnt_buff_size];
    file.read(pcnt_buff, pcnt_buff_size);
    auto *counter = reinterpret_cast<uint32_t *>(pcnt_buff);
    auto *rating = reinterpret_cast<uint16_t *>(rating_buff);

    POPMFrame POPMFrame(frame_name, frame_flags, frame_size, email, *rating, *counter);
    Storage.POPMFrame_vector.emplace_back(POPMFrame);
}

void ParsePOSSFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char time_stamp_format_buff[kOneByte];
    file.read(reinterpret_cast<char*>(time_stamp_format), kOneByte);
    int position_buff_size = frame_size - kOneByte;
    char position_buff[position_buff_size];
    file.read(position_buff, position_buff_size);

    auto *time_stamp_format = reinterpret_cast<uint8_t *>(time_stamp_format_buff);
    auto *position = reinterpret_cast<uint32_t *>(position_buff);

    POSSFrame POSSFrame(frame_name, frame_flags, frame_size,
                        *time_stamp_format, *position);
    Storage.POSSFrame_vector.emplace_back(POSSFrame);
}

void ParseEQU2Frame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char interpolation_method[kOneByte];
    std::string identification;
    file.read(interpolation_method, kOneByte);
    getline(file, identification, '\0');

    EQU2Frame EQU2Frame(frame_name, frame_flags, frame_size,
                        interpolation_method[0], identification);
    Storage.EQU2Frame_vector.emplace_back(EQU2Frame);
}

void ParseRBUFFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char buffer_size_buff[kBufferSizeSize];
    char embedded_info_flag_buff[kOneByte];
    char offset_to_next_tag_buff[kOffsetToNextTagBuffSize];

    file.read(buffer_size_buff, 3);
    file.read(embedded_info_flag_buff, kOneByte);
    file.read(offset_to_next_tag_buff, kOffsetToNextTagBuffSize);

    auto *buffer_size = reinterpret_cast<int *>(buffer_size_buff);
    auto *embedded_info_flag = reinterpret_cast<bool *>(embedded_info_flag_buff);
    auto *offset_to_next_tag = reinterpret_cast<int *>(offset_to_next_tag_buff);

    RBUFFrame RBUFFrame(frame_name, frame_flags, frame_size, *buffer_size,
                        *embedded_info_flag, *offset_to_next_tag);
    Storage.RBUFFrame_vector.emplace_back(RBUFFrame);
}

void ParseSYLTFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char text_encoding_buff[kOneByte];
    char language_buff[kLanguageBuffSize];
    char time_stamp_format_buff[kOneByte];
    char content_type_buff[kOneByte];
    std::string content_descriptor;
    file.read(text_encoding_buff, kOneByte);
    file.read(language_buff, kLanguageBuffSize);
    file.read(time_stamp_format_buff, kOneByte);
    file.read(content_type_buff, kOneByte);
    getline(file, content_descriptor, '\0');

    auto *text_encoding = reinterpret_cast<uint16_t *>(text_encoding_buff);
    SYLTFrame SYLTFrame(frame_name, frame_flags, frame_size, *text_encoding,
                        language_buff,
                        time_stamp_format_buff[0], content_type_buff[0],
                        content_descriptor);
    Storage.SYLTFrame_vector.emplace_back(SYLTFrame);
}

void ParseUSLTFrame(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;

    readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);

    char text_encoding_buff[kOneByte];
    char language_buff[kLanguageBuffSize];
    std::string content_descriptor;

    file.read(text_encoding_buff, kOneByte);
    file.read(language_buff, kLanguageBuffSize);
    getline(file, content_descriptor, '\0');
    int lyrics_size = frame_size - kOneByte - kLanguageBuffSize - content_descriptor.size() - kOneByte;
    char lyrics[lyrics_size + kOneByte];
    file.read(lyrics, lyrics_size);
    lyrics[lyrics_size] = '\0';
    auto *text_encoding = reinterpret_cast<uint16_t *>(text_encoding_buff);

    USLTFrame USLTFrame(frame_name, frame_flags, frame_size,
                        *text_encoding, language_buff,
                        lyrics, content_descriptor);
    Storage.USLTFrame_vector.emplace_back(USLTFrame);
}

void ReadFrameName(std::ifstream &file, std::string &frame_name) {
    char frame_name_buff[kFrameNameBuffSize + kOneByte];
    file.read(frame_name_buff, kFrameNameBuffSize);
    frame_name_buff[4] = '\0';
    file.seekg(-(kFrameNameBuffSize), std::ios::cur);
    frame_name = frame_name_buff;
}

void Parse(std::ifstream &file, AllFramesVectorClass &Storage) {
    std::string frame_name;
    std::string frame_flags;
    int frame_size;
    ID3v2Header ID3v2Header;
    readID3v2Header(file, ID3v2Header);
    while (!file.eof()) {
        ReadFrameName(file, frame_name);
        if (frame_name[0] == 'T' && (frame_name[1] != 'X' || frame_name[2] != 'X' || frame_name[3] != 'X')) {
            ParseTFrame(file, Storage);
        } else if (frame_name == "TXXX") {
            ParseTXXXFrame(file, Storage);
        } else if (frame_name == "UFID") {
            ParseUFIDFrame(file, Storage);
        } else if (frame_name == "USER") {
            ParseUSERFrame(file, Storage);
        } else if (frame_name[0] == 'W' && (frame_name[1] != 'X' || frame_name[2] != 'X' || frame_name[3] != 'X')) {
            ParseWFrame(file, Storage);
        } else if (frame_name == "WXXX") {
            ParseWXXXFrame(file, Storage);
        } else if (frame_name == "SEEK") {
            ParseSEEKFrame(file, Storage);
        } else if (frame_name == "GRID") {
            ParseGRIDFrame(file, Storage);
        } else if (frame_name == "RVA2") {
            ParseRVA2Frame(file, Storage);
        } else if (frame_name == "OWNE") {
            ParseOWNEFrame(file, Storage);
        } else if (frame_name == "COMM") {
            ParseCOMMFrame(file, Storage);
        } else if (frame_name == "COMR") {
            ParseCOMRFrame(file, Storage);
        } else if (frame_name == "ENCR") {
            ParseENCRFrame(file, Storage);
        } else if (frame_name == "ETCO") {
            ParseETCOFrame(file, Storage);
        } else if (frame_name == "LINK") {
            ParseLINKFrame(file, Storage);
        } else if (frame_name == "PRIV") {
            ParsePRIVFrame(file, Storage);
        } else if (frame_name == "PCNT") {
            ParsePCNTFrame(file, Storage);
        } else if (frame_name == "POPM") {
            ParsePOPMFrame(file, Storage);
        } else if (frame_name == "POSS") {
            ParsePOSSFrame(file, Storage);
        } else if (frame_name == "EQU2") {
            ParseEQU2Frame(file, Storage);
        } else if (frame_name == "RBUF") {
            ParseRBUFFrame(file, Storage);
        } else if (frame_name == "SYLT") {
            ParseSYLTFrame(file, Storage);
        } else if (frame_name == "USLT") {
            ParseUSLTFrame(file, Storage);
        } else {
            readID3v2FrameHeader(file, frame_name, frame_flags, frame_size);
            if (frame_name.empty()) {
                break;
            }
            file.seekg(frame_size, std::ios::cur);
        }
    }
}


