//
// Created by Goran Pjević on 01/03/2021.
//

#ifndef MID_CRACK_CODE_IMAGE_H
#define MID_CRACK_CODE_IMAGE_H


class Image {
private:
    int fileSize{};
    int imageWidth{};
    int imageHeight{};
//    2D array to represent the pixels of the image (0 is white, 1 is black)
    int **bitImage{};
//    header sizes
    static const int fileHeaderSize = 14;
    static const int informationHeaderSize = 40;
//    headers
    unsigned char fileHeader[fileHeaderSize]{};
    unsigned char informationHeader[informationHeaderSize]{};
public:
    Image();

    explicit Image(const char *imageFilePath);

    virtual ~Image();

    [[nodiscard]] int getImageWidth() const;

    [[nodiscard]] int getImageHeight() const;

    [[nodiscard]] int **getBitImage() const;

    void setImageWidth(int imageWidthToBeSet);

    void setImageHeight(int imageHeightToBeSet);

    void setBitImage(int **bitImageToBeSet);

    void saveImage(const std::string &fileName);
};


#endif //MID_CRACK_CODE_IMAGE_H
