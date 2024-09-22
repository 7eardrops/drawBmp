#include <iostream>
#include <fstream>
#include <windows.h>
#include <cstdint>

using namespace std;

class BMPImage {
public:
    BMPImage() : pixelData(nullptr) {}

    ~BMPImage() {
        closeBMP();
    }

    // Открытие BMP-файла
    bool openBMP(const string& fileName) {
        // Открываем файл в бинарном режиме
        file.open(fileName, ios::binary);
        if (!file.is_open()) {
            cerr << "Не удалось открыть файл: " << fileName << endl;
            return false;
        }

        // Чтение заголовка BMP-файла
        file.read(reinterpret_cast<char*>(&fileHeader), sizeof(BITMAPFILEHEADER));
        if (fileHeader.bfType != 0x4D42) { // Проверка, что это файл формата BMP (BM)
            cerr << "Файл не является BMP." << endl;
            return false;
        }

        // Чтение заголовка информации о BMP
        file.read(reinterpret_cast<char*>(&infoHeader), sizeof(BITMAPINFOHEADER));

        // Проверка поддержки только 24- или 32-битных BMP
        if (infoHeader.biBitCount != 24 && infoHeader.biBitCount != 32) {
            cerr << "Поддерживаются только 24- и 32-битные BMP." << endl;
            return false;
        }

        // Чтение пиксельных данных
        int rowSize = ((infoHeader.biWidth * infoHeader.biBitCount + 31) / 32) * 4;
        pixelData = new uint8_t[rowSize * infoHeader.biHeight];

        file.seekg(fileHeader.bfOffBits, file.beg);
        file.read(reinterpret_cast<char*>(pixelData), rowSize * infoHeader.biHeight);

        return true;
    }

    // Отображение BMP-файла
    void displayBMP() const {
        int rowSize = ((infoHeader.biWidth * infoHeader.biBitCount + 31) / 32) * 4;

        for (int y = 0; y < infoHeader.biHeight; ++y) {
            for (int x = 0; x < infoHeader.biWidth; ++x) {
                uint32_t pixel = getPixel(x, y, rowSize);
                if (isBlack(pixel)) {
                    cout << '1';  // Черный цвет
                }
                else if (isWhite(pixel)) {
                    cout << '0';  // Белый цвет (фон)
                }
                else {
                    cerr << "Обнаружен цвет, отличный от черного и белого." << endl;
                    return;
                }
            }
            cout << endl;
        }
    }

    // Закрытие BMP-файла и освобождение памяти
    void closeBMP() {
        if (file.is_open()) {
            file.close();
        }
        if (pixelData) {
            delete[] pixelData;
            pixelData = nullptr;
        }
    }

private:
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    ifstream file;
    uint8_t* pixelData;

    // Получение пикселя по координатам x, y
    uint32_t getPixel(int x, int y, int rowSize) const {
        const uint8_t* row = pixelData + rowSize * (infoHeader.biHeight - 1 - y);  // Инверсия по Y
        const uint8_t* pixel = row + x * (infoHeader.biBitCount / 8);

        if (infoHeader.biBitCount == 24) {
            // Для 24-битного BMP
            uint8_t blue = pixel[0];
            uint8_t green = pixel[1];
            uint8_t red = pixel[2];
            return (red << 16) | (green << 8) | blue;
        }
        else if (infoHeader.biBitCount == 32) {
            // Для 32-битного BMP
            uint8_t blue = pixel[0];
            uint8_t green = pixel[1];
            uint8_t red = pixel[2];
            // Пропуск альфа-канала
            return (red << 16) | (green << 8) | blue;
        }
        return 0;
    }

    // Проверка, является ли пиксель черным
    bool isBlack(uint32_t pixel) const {
        return pixel == 0x000000;  // RGB(0, 0, 0)
    }

    // Проверка, является ли пиксель белым
    bool isWhite(uint32_t pixel) const {
        return pixel == 0xFFFFFF;  // RGB(255, 255, 255)
    }
};

// Основная функция программы
int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "RU");

    if (argc != 2) {
        cerr << "Использование: " << argv[0] << " <путь к BMP файлу>" << endl;
        return 1;
    }

    BMPImage image;
    if (!image.openBMP(argv[1])) {
        return 1;
    }

    image.displayBMP();
    image.closeBMP();

    return 0;
}