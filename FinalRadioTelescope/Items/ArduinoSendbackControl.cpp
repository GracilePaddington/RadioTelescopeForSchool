#include <windows.h>
#include <iostream>

class ArduinoCommunicator
{
public:
    ArduinoCommunicator(const char* portName); //needs a port name, it's just gonna be COM2 for this. 
    ~ArduinoCommunicator();


    void sendPointData(int X, int Y); //method for passing the data to arduino of the rise and run... 

private:
    HANDLE serialHandle;
};

ArduinoCommunicator::ArduinoCommunicator(const char* portName)
{
    
    serialHandle = CreateFileA(portName, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//opens serial port, I am far more verbose about this process in ArduinoFileControl-- everything I do in here is done there as well... Not gonna spend too much time on comments because of this... 


 
        DCB dcbSerialParams = { 0 };
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        dcbSerialParams.BaudRate = CBR_9600;  
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;

    
}

ArduinoCommunicator::~ArduinoCommunicator() 
{
    
    CloseHandle(serialHandle);
}



void ArduinoCommunicator::sendPointData(int X, int Y) 
{
    
    char buffer[20]; //format these goods... 
    sprintf_s(buffer, sizeof(buffer), "SCAN_%d_%d", X, Y);

   
    DWORD bytesWritten; //fire it off.... 
    WriteFile(serialHandle, buffer, strlen(buffer), &bytesWritten, NULL);


}