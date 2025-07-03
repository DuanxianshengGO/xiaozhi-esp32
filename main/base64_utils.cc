#include "base64_utils.h"
#include <algorithm>

const std::string Base64Utils::base64_chars = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

std::string Base64Utils::Encode(const std::vector<uint8_t>& data) {
    return Encode(data.data(), data.size());
}

std::string Base64Utils::Encode(const uint8_t* data, size_t length) {
    std::string result;
    int val = 0, valb = -6;
    
    for (size_t i = 0; i < length; ++i) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    
    if (valb > -6) {
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    
    while (result.size() % 4) {
        result.push_back('=');
    }
    
    return result;
}

std::vector<uint8_t> Base64Utils::Decode(const std::string& encoded) {
    std::vector<uint8_t> result;
    int val = 0, valb = -8;
    
    for (unsigned char c : encoded) {
        if (!IsBase64Char(c)) {
            break;
        }
        
        val = (val << 6) + base64_chars.find(c);
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    
    return result;
}

bool Base64Utils::IsValidBase64(const std::string& encoded) {
    if (encoded.empty()) {
        return false;
    }
    
    // 检查长度是否为4的倍数
    if (encoded.length() % 4 != 0) {
        return false;
    }
    
    // 检查字符是否有效
    for (size_t i = 0; i < encoded.length(); ++i) {
        char c = encoded[i];
        if (i >= encoded.length() - 2 && c == '=') {
            // 允许末尾的填充字符
            continue;
        }
        if (!IsBase64Char(c)) {
            return false;
        }
    }
    
    return true;
}

bool Base64Utils::IsBase64Char(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}
