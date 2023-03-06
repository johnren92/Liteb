#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

namespace LiteHTTP {
    class Communication {
    public:
        static std::string makeRequest(const std::string& url, const std::string& parameters) {
            try {
                std::string result;
                CURL* curl = curl_easy_init();
                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, parameters.c_str());
                    curl_easy_setopt(curl, CURLOPT_USERAGENT, "E9BC3BD76216AFA560BFB5ACAF5731A3");
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
                    CURLcode res = curl_easy_perform(curl);
                    if (res != CURLE_OK) {
                        std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
                        return "rqf";
                    }
                    curl_easy_cleanup(curl);
                }
                return result;
            }
            catch (...) {
                return "rqf";
            }
        }

        static std::string encrypt(const std::string& input) {
            try {
                const std::string key = Settings::edkey;
                const EVP_CIPHER* cipher = EVP_aes_256_cbc();
                unsigned char iv[EVP_MAX_IV_LENGTH];
                memset(iv, 0, sizeof(iv));
                std::string output;
                EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                if (ctx) {
                    if (EVP_EncryptInit_ex(ctx, cipher, NULL, (const unsigned char*)key.c_str(), iv) != 1) {
                        std::cerr << "EVP_EncryptInit_ex() failed" << std::endl;
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    std::vector<unsigned char> outbuf(input.size() + AES_BLOCK_SIZE);
                    int outlen;
                    if (EVP_EncryptUpdate(ctx, outbuf.data(), &outlen, (const unsigned char*)input.c_str(), input.size()) != 1) {
                        std::cerr << "EVP_EncryptUpdate() failed" << std::endl;
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    output.append(reinterpret_cast<char*>(outbuf.data()), outlen);
                    if (EVP_EncryptFinal_ex(ctx, outbuf.data(), &outlen) != 1) {
                        std::cerr << "EVP_EncryptFinal_ex() failed" << std::endl;
                        EVP_CIPHER_CTX_free(ctx);
                        return "";
                    }
                    output.append(reinterpret_cast<char*>(outbuf.data()), outlen);
                    EVP_CIPHER_CTX_free(ctx);
                }
                std::ostringstream oss;
                oss << std::hex << std::setfill('0');
                for (size_t i = 0; i < output.size(); i++) {
                    oss << std::setw(2) << static_cast<int>(output[i]);
                }
                return oss.str();
            }
            catch (...) {
                return "";
            }
        }

        static std::string decrypt(const std::string& input) {
            try {
                const std::string key = Settings::edkey;
                const EVP_CIPHER* cipher = EVP_aes_256_cbc();
                unsigned char iv[EVP_MAX_IV_LENGTH];
                memset(iv, 0, sizeof(iv));
               
