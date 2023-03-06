#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <cstring>

#include <openssl/evp.h>

using namespace std;

namespace LiteHTTP {
    namespace Classes {

        class Communication {
        public:
            static string makeRequest(const string& url, const string& parameters) {
                try {
                    string result;
                    vector<uint8_t> param(parameters.begin(), parameters.end());

                    // Create HTTP POST request
                    auto* req = curl_easy_init();
                    if (req == nullptr) {
                        throw runtime_error("Failed to create HTTP request");
                    }

                    curl_easy_setopt(req, CURLOPT_URL, url.c_str());
                    curl_easy_setopt(req, CURLOPT_POST, 1L);
                    curl_easy_setopt(req, CURLOPT_POSTFIELDS, &param[0]);
                    curl_easy_setopt(req, CURLOPT_POSTFIELDSIZE, param.size());
                    curl_easy_setopt(req, CURLOPT_USERAGENT, "E9BC3BD76216AFA560BFB5ACAF5731A3");
                    curl_easy_setopt(req, CURLOPT_HTTPHEADER, curl_slist_append(nullptr, "Content-Type: application/x-www-form-urlencoded"));

                    // Send HTTP request and receive response
                    curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, &Communication::writeCallback);
                    curl_easy_setopt(req, CURLOPT_WRITEDATA, &result);

                    auto res = curl_easy_perform(req);
                    if (res != CURLE_OK) {
                        throw runtime_error("Failed to send HTTP request");
                    }

                    curl_easy_cleanup(req);
                    return result;
                }
                catch (...) {
                    return "rqf";
                }
            }

            static string encrypt(const string& input) {
                try {
                    string key = Settings::edkey;
                    const EVP_CIPHER* cipher = EVP_aes_256_cbc();
                    vector<uint8_t> ky(key.begin(), key.end());
                    vector<uint8_t> inp(input.begin(), input.end());
                    vector<uint8_t> res(inp.size() + cipher->block_size());

                    // Encrypt input using AES-256 CBC mode
                    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
                    if (ctx == nullptr) {
                        throw runtime_error("Failed to create encryption context");
                    }

                    int len = 0;
                    if (!EVP_EncryptInit_ex(ctx, cipher, nullptr, &ky[0], &ky[0] + ky.size())) {
                        throw runtime_error("Failed to initialize encryption");
                    }
                    if (!EVP_EncryptUpdate(ctx, &res[0], &len, &inp[0], inp.size())) {
                        throw runtime_error("Failed to encrypt input");
                    }
                    int totalLen = len;
                    if (!EVP_EncryptFinal_ex(ctx, &res[len], &len)) {
                        throw runtime_error("Failed to finalize encryption");
                    }
                    totalLen += len;

                    EVP_CIPHER_CTX_free(ctx);

                    // Encode encrypted result as base64 string
                    auto base64 = [](const vector<uint8_t>& data) {
                        string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
                        string result;
                        int padding = (3 - data.size() % 3) % 3;
                        uint32_t buffer = 0;
                        int bitsInBuffer = 0;
                        for (auto c : data) {
                            buffer = (buffer << 8) | c;
                            bitsInBuffer += 8;
                            while (bitsInBuffer >= 6) {
                                result += base64Chars[(buffer >> (bitsInBuffer - 6)) & 0x3F];
