#pragma once
#include "key.h"
#include "Header.h"

int key_size;

void setKeyConstInt(const int key) {
    key_size = key;
}

const int getKeyConstInt() {
    return key_size;
}





//----------------------------------------------------------------------------------
//                        Шифрование с помощью пароля
//----------------------------------------------------------------------------------
void GenerateHashKey(std::vector<unsigned char>& key) {
    std::mt19937 gen(324334);
    for (int i = 0; i < key_size; ++i) {
        key[i] = static_cast<unsigned char>(gen() % 256);
    }
}

void Encrypt(std::vector<unsigned char>& buffer, const std::vector<unsigned char>& key) {
    for (int i = 0; i < buffer.size(); ++i) {
        buffer[i] = buffer[i] ^ key[i % key_size];
    }
}

void Decrypt(std::vector<unsigned char>& buffer, const std::vector<unsigned char>& key) {
    for (int i = 0; i < buffer.size(); ++i) {
        buffer[i] = buffer[i] ^ key[i % key_size];
    }
}

//----------------------------------------------------------------------------------
//                        Шифрование с помощью текста
//----------------------------------------------------------------------------------
void stringKeyGenerateHashKey(std::vector<unsigned char>& key, const wchar_t* keyBuffer) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::string keyString = converter.to_bytes(keyBuffer);
    std::mt19937 gen(std::hash<std::string>{}(keyString));
    for (size_t i = 0; i < keyString.length(); ++i) {
        key[i] = static_cast<unsigned char>(gen() % 256);
    }
}

void stringKeyEncrypt(std::vector<unsigned char>& buffer, const std::vector<unsigned char>& key) {
    for (int i = 0; i < buffer.size(); ++i) {
        buffer[i] = buffer[i] ^ key[i % key.size()];
    }
}

void stringKeyDecrypt(std::vector<unsigned char>& buffer, const std::vector<unsigned char>& key) {
    for (int i = 0; i < buffer.size(); ++i) {
        buffer[i] = buffer[i] ^ key[i % key.size()];
    }
}

//----------------------------------------------------------------------------------
//                       Вспомогательные функции
//----------------------------------------------------------------------------------
void SaveToFile(const wchar_t* filename, const std::vector<unsigned char>& data) {
    std::ofstream outFile(std::wstring(filename), std::ios::binary);
    if (!outFile.is_open()) {
        MessageBox(NULL, L"файл  не сохранили", L"Внимание", MB_OK);
        return;
    }
    //MessageBox(NULL, L"файл  сохранили", L"Внимание", MB_OK);
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
}


std::vector<unsigned char> LoadFromFile(const wchar_t* filename) {
    std::ifstream inFile(filename, std::ios::binary);
    if (!inFile.is_open()) {
        MessageBoxW(NULL, L"Не удалось открыть входной файл\nСкорее всего он чем-то занят или не существует\n", L"Ошибка", MB_ICONERROR);
        return {};
    }
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inFile), {});
    inFile.close();
    //MessageBoxW(NULL, L"Файл загружен", L"Внимание", MB_OK);
    return buffer;
}


//----------------------------------------------------------------------------------
//                       Я ебал этот С++
//----------------------------------------------------------------------------------

void Metod1(wchar_t* KeyBuffer, wchar_t* InputBuffer, wchar_t* OutputBuffer)
{
        int keyLength = wcslen(KeyBuffer);
        setKeyConstInt(keyLength); // устнавливаем ключ
        std::vector<unsigned char> buffer = LoadFromFile(InputBuffer);
        if (!buffer.empty()) {
            std::vector<unsigned char> key(key_size);
            GenerateHashKey(key);
            Encrypt(buffer, key);

            //Decrypt(buffer, key);

            SaveToFile(OutputBuffer, buffer);
        }
}



void Metod2(wchar_t* KeyBuffer, wchar_t* InputBuffer, wchar_t* OutputBuffer)
{
    std::vector<unsigned char> buffer = LoadFromFile(InputBuffer);
    if (!buffer.empty()) {

        std::vector<unsigned char> key(wcslen(KeyBuffer) * 2);
        stringKeyGenerateHashKey(key, KeyBuffer);
        stringKeyEncrypt(buffer, key);

        //stringKeyDecrypt(buffer, key);

        SaveToFile(OutputBuffer, buffer);
    }
}