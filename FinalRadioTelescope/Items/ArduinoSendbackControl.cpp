#include <windows.h>
#include <iostream>

class ArduinoCommunicator
{
public:
    ArduinoCommunicator(const char* portName);
    ~ArduinoCommunicator();


    void sendPointData(int X, int Y);

private:
    HANDLE serialHandle;
};

ArduinoCommunicator::ArduinoCommunicator(const char* portName)
{
    // Open the serial port
    serialHandle = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (serialHandle == INVALID_HANDLE_VALUE) {
        std::cerr << "Error opening serial port." << std::endl;
    }
    else {
        DCB dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

        if (!GetCommState(serialHandle, &dcbSerialParams)) {
            std::cerr << "Error getting serial port state." << std::endl;
        }

        dcbSerialParams.BaudRate = CBR_9600;  // Set your desired baud rate
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

        if (!SetCommState(serialHandle, &dcbSerialParams)) {
            std::cerr << "Error setting serial port state." << std::endl;
        }
    }
}

ArduinoCommunicator::~ArduinoCommunicator() 
{
    // Close the serial port
    CloseHandle(serialHandle);
}



void ArduinoCommunicator::sendPointData(int X, int Y) 
{
    
    char buffer[20]; //format these goods... 
    sprintf_s(buffer, sizeof(buffer), "SCAN_%d_%d", X, Y);

   
    DWORD bytesWritten; //fire it off.... 
    WriteFile(serialHandle, buffer, strlen(buffer), &bytesWritten, NULL);


}