#ifndef BASE64_UTILS_H
#define BASE64_UTILS_H

#include <string>
#include <vector>

class Base64Utils {
public:
    // 编码二进制数据为Base64字符串
    static std::string Encode(const std::vector<uint8_t>& data);
    static std::string Encode(const uint8_t* data, size_t length);
    
    // 解码Base64字符串为二进制数据
    static std::vector<uint8_t> Decode(const std::string& encoded);
    
    // 检查字符串是否为有效的Base64
    static bool IsValidBase64(const std::string& encoded);

private:
    static const std::string base64_chars;
    static bool IsBase64Char(unsigned char c);
};

#endif // BASE64_UTILS_H
