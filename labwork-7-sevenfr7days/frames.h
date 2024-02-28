#include <iostream>
#include <fstream>
#include <vector>

const int kHeaderVersionSize = 2;
const int kHeaderTagSize = 3;
const int kOneByte = 1;
const int kFrameIdentidierSize = 4;
const int kBufferSizeSize = 3;
const int kOffsetToNextTagBuffSize = 4;
const int kLanguageBuffSize = 3;
const int kFrameNameBuffSize = 4;
const int kSizeBuffSize = 4;
const int kIDBuffSize = 4;
const int kMinOffsetBuffSize = 4;
const int kDateOfPurchSize = 8;
const int kValidUntilSize = 8;

struct ID3v2Header {
    char tag[kHeaderTagSize];
    char version[kHeaderVersionSize];
    std::string flags;
    int size;
};

struct FrameHeader {
public:
    std::string frame_name_;
    std::string frame_flags_;
    int frame_size_;

    FrameHeader(std::string id, std::string flags, int size) :
            frame_name_(std::move(id)),
            frame_flags_(std::move(flags)),
            frame_size_(size) {};

    void CoutBaseInfo() const {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
    }
};

class TFrame : public FrameHeader {
public:
    char frame_encoding_;
    std::string frame_info_;

    TFrame(std::string id, std::string flags, int size, char frame_encoding, std::string frame_info) :
            frame_encoding_(frame_encoding),
            frame_info_(std::move(frame_info)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    virtual void CoutAllInfo() {
        std::cout << "Frame encoding: " << int(frame_encoding_) << "\n";
        std::cout << "Frame data: " << frame_info_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class TXXXFrame : public TFrame {
public:
    std::string frame_description_;

    TXXXFrame(std::string id, std::string flags, int size, char frame_encoding,
              std::string frame_info, std::string frame_description) :
            frame_description_(std::move(frame_description)),
            TFrame(std::move(id), std::move(flags), size, frame_encoding,
                   std::move(frame_info)) {}

    void CoutAllInfo() override {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame encoding: " << int(frame_encoding_) << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Frame description: " << frame_description_ << "\n";
        std::cout << "Frame data: " << frame_info_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class UFIDFrame : public FrameHeader {
protected:
    std::string owner_identifier_;
    std::string identifier_;
public:
    UFIDFrame(std::string id, std::string flags, int size, std::string owner_identifier, std::string identifier) :
            owner_identifier_(std::move(owner_identifier)),
            identifier_(std::move(identifier)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Owner Identifier: " << owner_identifier_ << "\n";
        std::cout << "Identifier: " << identifier_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class USERFrame : public FrameHeader {
protected:
    uint16_t text_encoding_;
    std::string language_;
    std::string actual_text_;
public:
    USERFrame(std::string id, std::string flags, int size, uint16_t text_encoding, std::string language,
              std::string actual_text) :
            text_encoding_(text_encoding),
            language_(std::move(language)),
            actual_text_(std::move(actual_text)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Language: " << language_ << "\n";
        std::cout << "Actual text: " << actual_text_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class WFrame : public FrameHeader {
public:
    std::string url_;
public:
    WFrame(std::string id, std::string flags, int size, std::string url) :
            url_(std::move(url)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    virtual void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "URL: " << url_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class WXXXFrame : public WFrame {
public:
    uint16_t text_encoding_;
    std::string text_description_;

    WXXXFrame(std::string id, std::string flags, int size, uint16_t text_encoding, std::string text_description,
              std::string url) :
            text_encoding_(text_encoding),
            text_description_(std::move(text_description)),
            WFrame(std::move(id), std::move(flags), size, std::move(url)) {}

    void CoutAllInfo() override {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Frame encoding: " << int(text_encoding_) << "\n";
        std::cout << "Frame description: " << text_description_ << "\n";
        std::cout << "URL: " << url_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class SEEKFrame : public FrameHeader {
protected:
    uint32_t min_offset_;
public:
    SEEKFrame(std::string id, std::string flags, int size, uint32_t min_offset) :
            min_offset_(min_offset),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Minimum offset to next tag: " << min_offset_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class GRIDFrame : public FrameHeader {
protected:
    std::string owner_id_;
    uint16_t group_symbol_;
    std::string group_dependent_data_;
public:
    GRIDFrame(std::string id, std::string flags, int size, std::string owner_id,
              uint16_t group_symbol, std::string group_dependent_data) :
            owner_id_(std::move(owner_id)),
            group_symbol_(group_symbol),
            group_dependent_data_(std::move(group_dependent_data)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Owner ID: " << owner_id_ << "\n";
        std::cout << "Group symbol: " << group_symbol_ << "\n";
        std::cout << "Group dependent data: " << group_dependent_data_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class RVA2Frame : public FrameHeader {
protected:
    std::string identification_;
public:
    RVA2Frame(std::string id, std::string flags, int size, std::string identification) :
            identification_(std::move(identification)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Identification: " << identification_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class OWNEFrame : public FrameHeader {
protected:
    uint16_t text_encoding_;
    std::string price_paid_;
    std::string data_of_purch_;
    std::string seller_;
public:
    OWNEFrame(std::string id, std::string flags, int size, uint16_t text_encoding,
              std::string price_paid, std::string data_of_purch, std::string seller) :
            text_encoding_(text_encoding),
            price_paid_(std::move(price_paid)),
            data_of_purch_(std::move(data_of_purch)),
            seller_(std::move(seller)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Price paid: " << price_paid_ << "\n";
        std::cout << "Data of purch: " << data_of_purch_ << "\n";
        std::cout << "Seller: " << seller_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class COMMFrame : public FrameHeader {
protected:
    uint16_t text_encoding_;
    std::string language_;
    std::string short_text_description_;
    std::string actual_text_;
public:
    COMMFrame(std::string id, std::string flags, int size, uint16_t text_encoding, std::string language,
              std::string short_text_description, std::string actual_text) :
            text_encoding_(text_encoding),
            language_(std::move(language)),
            short_text_description_(std::move(short_text_description)),
            actual_text_(std::move(actual_text)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Language: " << language_ << "\n";
        std::cout << "Short text description: " << short_text_description_ << "\n";
        std::cout << "Actual text: " << actual_text_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class COMRFrame : public FrameHeader {
protected:
    char text_encoding_;
    std::string price_;
    std::string valid_until_;
    std::string contact_URL_;
    char received_as_;
    std::string name_of_seller_;
    std::string description_;
    std::string picture_mime_type_;
    std::vector<char> seller_logo_;
public:
    COMRFrame(std::string id, std::string flags, int size, char text_encoding,
              std::string price, std::string valid_until, std::string contact_URL, char received_as,
              std::string name_of_seller, std::string description,
              std::string picture_mime_type, std::vector<char> seller_logo) :
            text_encoding_(text_encoding),
            price_(std::move(price)),
            valid_until_(std::move(valid_until)),
            contact_URL_(std::move(contact_URL)),
            received_as_(received_as),
            name_of_seller_(std::move(name_of_seller)),
            description_(std::move(description)),
            picture_mime_type_(std::move(picture_mime_type)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Price: " << price_ << "\n";
        std::cout << "Valid until: " << valid_until_ << "\n";
        std::cout << "Contact URL: " << contact_URL_ << "\n";
        std::cout << "Received as: " << received_as_ << "\n";
        std::cout << "Seller name: " << name_of_seller_ << "\n";
        std::cout << "Description: " << description_ << "\n";
        std::cout << "Picture MIME type: " << picture_mime_type_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class ENCRFrame : public FrameHeader {
protected:
    std::string owner_identifier_;
    char method_symbol_;
    std::vector<char> encryption_data_;
public:
    ENCRFrame(std::string id, std::string flags, int size, std::string owner_identifier,
              char method_symbol, std::vector<char> encryption_data) :
            owner_identifier_(std::move(owner_identifier)),
            method_symbol_(method_symbol),
            encryption_data_(std::move(encryption_data)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Owner identifier: " << owner_identifier_ << "\n";
        std::cout << "Method symbol: " << method_symbol_ << "\n";
        std::cout << "Encryption data: ";
        for (unsigned char i: encryption_data_) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
        std::cout << "========================================================" << "\n";
    }
};

class ETCOFrame : public FrameHeader {
protected:
    char time_stamp_format_;
public:
    ETCOFrame(std::string id, std::string flags, int size, char time_stamp_format) :
            time_stamp_format_(time_stamp_format),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Time stamp format: " << time_stamp_format_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class LINKFrame : public FrameHeader {
protected:
    std::vector<char> frame_identifier_;
    std::string url_;
    std::string id_and_additional_data_;
public:
    LINKFrame(std::string id, std::string flags, int size, std::vector<char> frame_identifier,
              std::string url, std::string id_and_additional_data) :
            frame_identifier_(std::move(frame_identifier)),
            url_(std::move(url)),
            id_and_additional_data_(std::move(id_and_additional_data)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Frame identifier: ";
        for (unsigned char i: frame_identifier_) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
        std::cout << "========================================================" << "\n";
    }
};

class PRIVFrame : public FrameHeader {
protected:
    std::string owner_identifier_;
    std::vector<char> private_data_;
public:
    PRIVFrame(std::string id, std::string flags, int size,
              std::string owner_identifier, std::vector<char> private_data) :
            owner_identifier_(std::move(owner_identifier)),
            private_data_(std::move(private_data)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Owner identifier: " << owner_identifier_ << "\n";
        std::cout << "Private data: ";
        for (unsigned char i: private_data_) {
            std::cout << i << ' ';
        }
        std::cout << '\n';
        std::cout << "========================================================" << "\n";
    }
};

class PCNTFrame : public FrameHeader {
public:
    uint32_t counter_;

    PCNTFrame(std::string id, std::string flags, int size, uint32_t counter) :
            counter_(counter),
            FrameHeader(std::move(id), std::move(flags), size) {}

    virtual void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Counter: " << counter_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class POPMFrame : public PCNTFrame {
public:
    std::string email_;
    uint16_t rating_;

    POPMFrame(std::string id, std::string flags, int size, std::string email, uint8_t rating, uint32_t counter) :
            email_(std::move(email)),
            rating_(rating),
            PCNTFrame(std::move(id), std::move(flags), size, counter) {}

    void CoutAllInfo() override {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Email: " << email_ << "\n";
        std::cout << "Rating: " << rating_ << "\n";
        std::cout << "Counter: " << counter_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class POSSFrame : public FrameHeader {
protected:
    uint8_t time_stamp_format_;
    uint32_t position_;
public:
    POSSFrame(std::string id, std::string flags, int size, uint8_t time_stamp_format, uint32_t position) :
            time_stamp_format_(time_stamp_format),
            position_(position),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Time stamp format: " << time_stamp_format_ << "\n";
        std::cout << "Position: " << position_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class EQU2Frame : public FrameHeader {
protected:
    char interpolation_;
    std::string identification_;
public:
    EQU2Frame(std::string id, std::string flags, int size, char interpolation, std::string identification) :
            interpolation_(interpolation),
            identification_(std::move(identification)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Interpolation method: " << interpolation_ << "\n";
        std::cout << "Identification: " << identification_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class RBUFFrame : public FrameHeader {
protected:
    int buffer_size_;
    bool embedded_into_flag_;
    int offset_to_next_tag_;
public:
    RBUFFrame(std::string id, std::string flags, int size, int buffer_size,
              bool embedded_into_tag, int offset_to_next_tag) :
            buffer_size_(buffer_size),
            embedded_into_flag_(embedded_into_tag),
            offset_to_next_tag_(offset_to_next_tag),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Buffer size: " << buffer_size_ << "\n";
        std::cout << "Embedded into flag: " << embedded_into_flag_ << "\n";
        std::cout << "Offset to next tag: " << offset_to_next_tag_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class SYLTFrame : public FrameHeader {
protected:
    uint16_t text_encoding_;
    std::string language_;
    char time_stamp_format_;
    char content_type_;
    std::string content_descriptor_;
public:
    SYLTFrame(std::string id, std::string flags, int size, uint16_t text_encoding, std::string language,
              char time_stamp_format, char content_type,
              std::string content_descriptor) :
            text_encoding_(text_encoding),
            language_(std::move(language)),
            time_stamp_format_(time_stamp_format),
            content_type_(content_type),
            content_descriptor_(std::move(content_descriptor)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Language: " << language_ << "\n";
        std::cout << "Time stamp format: " << time_stamp_format_ << "\n";
        std::cout << "Content type: " << content_type_ << "\n";
        std::cout << "Content descriptor: " << content_descriptor_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};

class USLTFrame : public FrameHeader {
protected:
    uint16_t text_encoding_;
    std::string language_;
    std::string content_descriptor_;
    std::string lyrics_or_text_;
public:
    USLTFrame(std::string id, std::string flags, int size, uint16_t text_encoding,
              std::string language, std::string lyrics_or_text,
              std::string content_descriptor) :
            text_encoding_(text_encoding),
            language_(std::move(language)),
            content_descriptor_(std::move(content_descriptor)),
            lyrics_or_text_(std::move(lyrics_or_text)),
            FrameHeader(std::move(id), std::move(flags), size) {}

    void CoutAllInfo() {
        std::cout << "========================================================" << "\n";
        std::cout << "Frame ID: " << frame_name_ << "\n";
        std::cout << "Frame size: " << frame_size_ << "\n";
        std::cout << "Frame flags: " << frame_flags_ << "\n";
        std::cout << "Text encoding: " << text_encoding_ << "\n";
        std::cout << "Language: " << language_ << "\n";
        std::cout << "Content descriptor: " << content_descriptor_ << "\n";
        std::cout << "Lyrics/text: " << lyrics_or_text_ << "\n";
        std::cout << "========================================================" << "\n";
    }
};


class AllFramesVectorClass {
public:
    std::vector<TFrame> TFrame_vector;
    std::vector<TXXXFrame> TXXXFrame_vector;
    std::vector<UFIDFrame> UFIDFrame_vector;
    std::vector<USERFrame> USERFrame_vector;
    std::vector<WFrame> WFrame_vector;
    std::vector<WXXXFrame> WXXXFrame_vector;
    std::vector<SEEKFrame> SEEKFrame_vector;
    std::vector<GRIDFrame> GRIDFrame_vector;
    std::vector<RVA2Frame> RVA2Frame_vector;
    std::vector<OWNEFrame> OWNEFrame_vector;
    std::vector<COMMFrame> COMMFrame_vector;
    std::vector<COMRFrame> COMRFrame_vector;
    std::vector<ENCRFrame> ENCRFrame_vector;
    std::vector<ETCOFrame> ETCOFrame_vector;
    std::vector<LINKFrame> LINKFrame_vector;
    std::vector<PRIVFrame> PRIVFrame_vector;
    std::vector<PCNTFrame> PCNTFrame_vector;
    std::vector<POPMFrame> POPMFrame_vector;
    std::vector<POSSFrame> POSSFrame_vector;
    std::vector<EQU2Frame> EQU2Frame_vector;
    std::vector<RBUFFrame> RBUFFrame_vector;
    std::vector<SYLTFrame> SYLTFrame_vector;
    std::vector<USLTFrame> USLTFrame_vector;

    void CoutWXXXFrame() {
        for (auto i: WXXXFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutTFrame() {
        for (auto i: TFrame_vector) {
            i.CoutBaseInfo();
            i.CoutAllInfo();
        }
    }

    void CoutTXXXFrame() {
        for (auto i: TXXXFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutUFIDFrame() {
        for (auto i: UFIDFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutUSERFrame() {
        for (auto i: USERFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutWFrame() {
        for (auto i: WFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutSEEKFrame() {
        for (auto i: SEEKFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutGRIDFrame() {
        for (auto i: GRIDFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutRVA2Frame() {
        for (auto i: RVA2Frame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutOWNEFrame() {
        for (auto i: OWNEFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutCOMMFrame() {
        for (auto i: COMMFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutCOMRFrame() {
        for (auto i: COMRFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutENCRFrame() {
        for (auto i: ENCRFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutETCOFrame() {
        for (auto i: ETCOFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutLINKFrame() {
        for (auto i: LINKFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutPRIVFrame() {
        for (auto i: PRIVFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutPCNTFrame() {
        for (auto i: PCNTFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutPOPMFrame() {
        for (auto i: POPMFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutPOSSFrame() {
        for (auto i: POSSFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutEQU2Frame() {
        for (auto i: EQU2Frame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutRBUFFrame() {
        for (auto i: RBUFFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutSYLTFrame() {
        for (auto i: SYLTFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutUSLTFrame() {
        for (auto i: USLTFrame_vector) {
            i.CoutAllInfo();
        }
    }

    void CoutAll() {
        CoutTFrame();
        CoutTXXXFrame();
        CoutUFIDFrame();
        CoutUSERFrame();
        CoutWFrame();
        CoutSEEKFrame();
        CoutGRIDFrame();
        CoutRVA2Frame();
        CoutOWNEFrame();
        CoutCOMMFrame();
        CoutCOMRFrame();
        CoutENCRFrame();
        CoutETCOFrame();
        CoutLINKFrame();
        CoutPRIVFrame();
        CoutPCNTFrame();
        CoutPOPMFrame();
        CoutPOSSFrame();
        CoutEQU2Frame();
        CoutRBUFFrame();
        CoutSYLTFrame();
        CoutUSLTFrame();
        CoutWXXXFrame();
    }
};
