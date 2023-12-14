#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include <windows.h> //need this for bmp 
#include <WinUser.h>

//ImageCompiler class by Gaby 

class ImageCompiler
{
public:
    std::vector<int> unorganizedImage;
    std::vector<std::vector<int>> organizedVectors;
    std::pair<int, int> detectionAreaSentFromGUI; //this is a pair object doesn't always have to be two ints but it's good for this case. 

    //Calls Detection Area
    void giveDetectionArea(const std::vector<int>& detectionArea)
    {
        //Detects if the detection area has 2 elements
        if (detectionArea.size() == 2)
        {
            //Uses make pair data type, this is used for heterogeneous objects and bound them together
            detectionAreaSentFromGUI = std::make_pair(detectionArea[0], detectionArea[1]);
            return;
        }
        //If detection area has the wrong format, outputs error message and empty vector. 
        else {
            std::cout << "Invalid detection area format. Expected '(x,y)'. " << std::endl;
            return; //Return null. 
        }
    }

    //Adds intensity to the Unorganized Image
    void addToImage(int intensity)
    {
        std::cout << "new Intesity of" <<intensity << "added" << std::endl;
        intensity = (intensity < 0) ? 0 : (intensity > 255) ? 255 : intensity;
        unorganizedImage.push_back(intensity);
    }

    void OrganizedVector()
    {
        // Clear existing data
        unorganizedImage.clear();

        std::cout << "vectors x and y " << detectionAreaSentFromGUI.first << detectionAreaSentFromGUI.second << "added" << std::endl; //testflag



        organizedVectors.clear();//clear it out... 

        //2d matrix for loop.... 
        for (int i = 0; i < detectionAreaSentFromGUI.second; i++)
        {
            std::vector<int> lineVector;
            for (int j = 0; j < detectionAreaSentFromGUI.first; j++)
            {
                // Calculate the index in the unorganizedImage vector
                int index = i * detectionAreaSentFromGUI.first + j;

                // Check if the index is within the bounds of unorganizedImage
                if (index < unorganizedImage.size())
                {
                    lineVector.push_back(unorganizedImage[index]);
                }
                else
                {
                    // Handle the case where the index is out of bounds
                    std::cerr << "Index out of bounds: " << index << std::endl;
                    // You might want to decide how to handle this situation, e.g., push back a default value.
                    lineVector.push_back(0); // Add a default value
                }
            }
            organizedVectors.push_back(lineVector);
        }
        reverse(organizedVectors.begin(), organizedVectors.end());

        // Make BMP picture or perform any other operations you need
        MakeABmpPicture();
    }

private:
    void MakeABmpPicture() //removed the arg there's no real point lol. 
    {
        const int width = detectionAreaSentFromGUI.first;
        const int height = detectionAreaSentFromGUI.second;
        //BMP = Bitmap Image
        //Need to make info header for file
        BITMAPINFOHEADER bmpInfoHeader;
        //Computer is being told where this is in memory
        memset(&bmpInfoHeader, 0, sizeof(BITMAPINFOHEADER));
        //BMP Size
        bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
        //BMP Width
        bmpInfoHeader.biWidth = width;
        //BMP Height
        bmpInfoHeader.biHeight = height;
        //BMP Color Planes
        bmpInfoHeader.biPlanes = 1;
        // 24 bits per pixel (RGB)
        bmpInfoHeader.biBitCount = 24;
        //Compression- how it stores the data
        bmpInfoHeader.biCompression = BI_RGB;

        //Device that its drawing on
        HDC hdc = GetDC(nullptr);
        void* bits;
        /*Making the image file by getting device that we just established then getting the header file that we just made, and gathering a color table that
        is to be used in this image.
        &bits - the way that the image recieves the info associated with it (actual picture)
        null pointer - doesnt care what device its being shown on
        0 - indicates that this can be edited
        */
        HBITMAP hBitmap = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bmpInfoHeader), DIB_RGB_COLORS, &bits, nullptr, 0);

        //Error statement if the DIB section failed to be assigned
        if (hBitmap == nullptr)
        {
            std::cerr << "Failed to create DIB section." << std::endl;
            //releases file from memory
            ReleaseDC(nullptr, hdc);
            return;
        }

        // Set pixel color based on intensity values, matrix operation. 
        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                BYTE* pixel = static_cast<BYTE*>(bits) + (y * width + x) * 3;

                int intensity = organizedVectors[y][x];

                // Map intensity to RGB color (blue to green to red gradient)
                pixel[0] = intensity < 128 ? 2 * intensity : 0; // Blue component (gradient for low/mid, no blue for high)
                pixel[1] = intensity < 128 ? 2 * intensity : 255 - 2 * intensity;  // Green component (gradient)
                pixel[2] = intensity >= 128 ? 2 * (intensity - 128) : 0; // Red component (gradient for high, no red for low/mid)
            }
        }

        // Save the BMP file and creates a file handle
        HANDLE hFile = CreateFile(L"RadioAntennaOutput.bmp", GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) {
            std::cerr << "Failed to create BMP file." << std::endl;
            DeleteObject(hBitmap);
            ReleaseDC(nullptr, hdc);
            return;
        }

        BITMAPFILEHEADER bmpFileHeader;
        memset(&bmpFileHeader, 0, sizeof(BITMAPFILEHEADER));
        //States what type of file this is
        bmpFileHeader.bfType = 0x4D42; // "BM"
        //Size of file
        bmpFileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + width * height * 3;
        //Size of file in bits
        bmpFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
        //Makes the space for the file and then puts the file in that space
        DWORD bytesWritten;
        WriteFile(hFile, &bmpFileHeader, sizeof(BITMAPFILEHEADER), &bytesWritten, nullptr);
        WriteFile(hFile, &bmpInfoHeader, sizeof(BITMAPINFOHEADER), &bytesWritten, nullptr);
        WriteFile(hFile, bits, width * height * 3, &bytesWritten, nullptr);
        //Closes file
        CloseHandle(hFile);
        //Deletes the map that was made to be copied
        DeleteObject(hBitmap);
        //Releases the device from being used
        ReleaseDC(nullptr, hdc);

        std::cout << "The BMP image is ready. Check RadioAntennaOutput.bmp." << std::endl; //this is for console... not really necessary for IMGUI but is in code for now for the sake of debugging and 
        //having active flags for quick checks. 
    }
};