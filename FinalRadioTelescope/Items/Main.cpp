//Directx9 IMGUI base class... 
//This is a modification of my Project 1 
#include "imgui.h" // imgui class, this is just for general imgui use 
#include "imgui_impl_dx9.h" //imgui class for directx9
#include "imgui_impl_win32.h" //imgui class for interfacing with windows. 
#include <d3d9.h> //general directx behaviors that are made into a custom class for imgui to interface with 
#include <tchar.h> //I forgor..... 
#include <vector> //vector for vectors... you know this one. 
#include <sstream> // std::stringstream -- isn't included in standard project. 
#include <iostream> //for console... 
#include <windows.h>
#include "ImageCompiler.cpp" 
#include <string>
#include <locale>
#include <codecvt>


using namespace std; //you know what that is. 
// Data
static LPDIRECT3D9              g_pD3D = nullptr; 
static LPDIRECT3DDEVICE9        g_pd3dDevice = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd); //this just creates the directx device for our use at the start, like how we make a vector at the start of a program. 
void CleanupDeviceD3D(); //this is for deleting the device later. 
void ResetDevice(); //this is for screen refreshes (like if you resize the window and move it over to another screen or if you minimize. I don't know what else it does exactly beyond that. 
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam); //function signature for the Window Procedure, what a rabbit hole. Don't worry about this. 


bool showWelcomeWindow = true; // bool for welcome window... 
bool firstTimeBeingShownGUI = true; 
bool firstTimeBeingShownConfig = true; 




void WelcomeWindow() 
{
    ImGui::Begin("Welcome to the Radio Telescope GUI");

    ImGui::Text("Welcome to the Radio Telescope GUI!");

    // Button to open a specific .txt file ("LibrariesUsed.txt")
    if (ImGui::Button("Libraries Used")) 
    {
        string filePath = "LibrariesUsed.txt";

        // Check if the file exists before attempting to open it
        ifstream file(filePath);
        if (file.good()) 
        {
            // File exists, perform actions with the file (e.g., display content)
            ImGui::Text("Opening file: %s", filePath.c_str());
        }
        else 
        {
            // File does not exist, handle the error
            ImGui::Text("Error: File not found - %s", filePath.c_str());
        }
    }

    // Button for "Begin"
    if (ImGui::Button("Begin"))
    {
        showWelcomeWindow = false;
    }

    ImGui::End();
}


class GUIWindow 
{

private:
    std::string imagePath; // Path to the BMP image file
    bool loadNewestImageClicked = false;

public:
    GUIWindow(const std::string& initialImagePath) : imagePath(initialImagePath) {}
    
    void render() 
    {
        firstTimeBeingShownGUI = false;
        // ImGui window rendering logic
        ImGui::Begin("Antenna Output");

        // Button to load the newest image
        if (ImGui::Button("Load Newest Image")) 
        {
            loadNewestImageClicked = true;
        }

        // Display the image
        displayImage();

        ImGui::End();
    }

    // Method to display the image
    void displayImage()
    {
        // Here, you can implement the code to display the BMP image.
        // You might want to use a graphics library or framework to handle image rendering.

        // For example, you can print the image path for now.
        ImGui::Text("Displaying image from path: %s", imagePath.c_str());
    }

    // Method to handle loading the newest image
    void loadNewestImage() 
    {
        // Implement logic to find and load the most recent BMP image in the folder
        // Update the 'imagePath' property accordingly

      
        if (loadNewestImageClicked) 
        {
           cout << "Load Newest Image button clicked." << endl; //debug out... 
            loadNewestImageClicked = false;
        }
    }

    // You can add more methods and properties as needed.
};

class ConfigurationWindow
{
private:
    ImageCompiler imageCompiler;
    std::wstring selectedFilePath;  // Use std::wstring for wide strings
    std::pair<int, int> resolution;


    // Helper function to get a list of .txt files in the directory
    std::vector<std::wstring> GetTxtFilesInDirectory(const std::wstring& directory)
    {
        std::vector<std::wstring> txtFiles;

        WIN32_FIND_DATA findFileData;
        HANDLE hFind = FindFirstFile((directory + L"\\*.txt").c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                // Convert C-style wide string to std::wstring
                txtFiles.push_back(std::wstring(findFileData.cFileName));
            } while (FindNextFile(hFind, &findFileData) != 0);

            FindClose(hFind);
        }

        return txtFiles;
    }

public:
    bool renderToBmpClicked = false;
    void Render()
    {
        firstTimeBeingShownConfig = false;
        // Set the project directory as the selected file path
        selectedFilePath = L"C:/Users/Spencer/Documents/RadioTelescope/FinalRadioTelescope";

        ImGui::Begin("Configuration Window");

        // Display a list of .txt files in the selected directory
        ImGui::Text("Selected File Path: %ls", selectedFilePath.c_str());

        // List the .txt files in the directory
        auto txtFiles = GetTxtFilesInDirectory(selectedFilePath);

        // Combo box to select a file
        static int selectedFileIndex = 0;
        if (ImGui::Combo("Select File", &selectedFileIndex,
            [](void* data, int idx, const char** out_text) {
                auto& files = *static_cast<std::vector<std::wstring>*>(data);
                *out_text = idx >= 0 && idx < files.size() ?
                    std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(files[idx]).c_str() :
                    "No File";
                return true;
            }, static_cast<void*>(&txtFiles), txtFiles.size()))
        {
            // User selected a file, handle it as needed
            // You can use txtFiles[selectedFileIndex] to get the selected file
        }

            // Button to render to BMP
            if (ImGui::Button("Render to BMP"))
            {
                renderToBmpClicked = true;
            }

            // If Render to BMP button is clicked, ask for resolution
            if (renderToBmpClicked)
            {
                ImGui::InputInt("Resolution X", &resolution.first);
                ImGui::InputInt("Resolution Y", &resolution.second);

                if (ImGui::Button("Confirm Resolution"))
                {
                    // User confirmed resolution and file selection
                    // Now, handle the file and resolution

                    // Get the selected file
                    if (selectedFileIndex >= 0 && selectedFileIndex < txtFiles.size())
                    {
                        std::wstring selectedFile = txtFiles[selectedFileIndex];

                        // Open the selected file
                        std::wifstream file(selectedFile);
                        if (file.is_open())
                        {
                            // Read intensity values from the file and add them to ImageCompiler
                            int value;
                            while (file >> value)
                            {
                                cout << value << endl; 
                                imageCompiler.addToImage(value);
                            }

                            // Close the file
                            file.close();

                            std::vector<int> xy = { resolution.first, resolution.second }; //recast for safety 

                            imageCompiler.giveDetectionArea(xy);

                            // Process the image in ImageCompiler
                            imageCompiler.OrganizedVector();

                            
                        }
                       
                    }
                }
            }

            ImGui::End();
    }
};


// Example function to initialize ImGui and DirectX
void initializeImGuiAndDirectX(HWND hwnd, LPDIRECT3DDEVICE9 g_pd3dDevice) 
{
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);
}

// Example function to clean up ImGui and DirectX
void cleanupImGuiAndDirectX() 
{
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}










void LaunchArduinoSerialPortConsole() //I give up on this... Can't seem to get it to work. Tried three or four different solutions, can't seem to figure out why it doesn't start. 
{
   
    LPCWSTR exeName = L"ArduinoSerialPortConsole";

    
    HINSTANCE result = ShellExecute(NULL, L"open", exeName, NULL, NULL, SW_SHOWNORMAL); //try running it... 

    if (reinterpret_cast<int>(result) > 32)//check against 32? 
    {
        
        cout << "ArduinoSerialPortConsole.exe launched successfully." << endl;
    }
    else
    {
        // An error occurred
       cerr << "Failed to launch ArduinoSerialPortConsole.exe. Please manually start the program from the Items folder in this software. We're sorry."<< endl;
    }
}












// Main code
int main(int, char**)
{
    
  
    ConfigurationWindow configurationWindow;
    GUIWindow guiWindow("initial_image_path.jpg");
   

    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Woodward Impedance Calculator", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Radio Telescope GUI", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance); //scope resolution operator for maximum effect I guess? They're searching for something globally
        //beyond this class or namespace. I guess this is probably best practice to super make sure you're killing the program so it doesn't linger and 
        //start to clog up people's computers. I just saw it in the other deployments. I guess in the case of failure you want to undoubtedly kill the process.
        return 1; 
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION(); // I didn't dive too deep into what this meant exactly, I know that at work, a version check isn't always a brick wall to keep
    //the user from running the program, sometimes you will have error handling for stuff you know will break with new updates coming out and will kind of 
    //be prepared for certain cases on version check. It looks like they're just passing in dimension info from first glance. 
    ImGui::CreateContext(); // the define here is pretty solid, this library really rocks. They made it really easy to learn and use. 
    ImGuiIO& io = ImGui::GetIO(); (void)io; //just guessing this is a housecleaning thing. It's like setting null on stuff after using it in C# for Unity
    //since engine will occasionally freak out. UE4 does the same with certain cases, if you don't explicitly null out certain objects before changing scenes
    //it'll just forget and act like it's supposed to be there when the next scene loads or during a build it's super annoying. 
 
    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_pd3dDevice);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // cool color config that Imgui has. You can be wild with it too, you can even animate it. 

    // Main loop
    bool done = false;
    LaunchArduinoSerialPortConsole();
    while (!done)
    {
       

        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            g_d3dpp.BackBufferWidth = g_ResizeWidth;
            g_d3dpp.BackBufferHeight = g_ResizeHeight;
            g_ResizeWidth = g_ResizeHeight = 0;
            ResetDevice();

            //In a Windows application, the WM_SIZE message is sent to the window procedure when the size of the window is changed
            //While you can handle window resizing directly inside the WM_SIZE handler, there are cases where it might not be ideal to perform resource-intensive operations,
            //such as resizing the back buffer for Direct3D rendering, directly within that message handler. Anything regarding GPU is kind of fickle in my experience and is
            //not always the most reliable act. There is some cool stuff you can do with 3js or if I chose to use openGL a lot of the math being done here could have been 
            //relegated to the graphics card as a shader op. 
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        

        if (showWelcomeWindow)
        {
            WelcomeWindow();
             // Disable welcome window after it's shown once
        }
        else if (!showWelcomeWindow)
        {
          

               
                configurationWindow.Render();
                
                guiWindow.render();
           
        }

        //my own logic for handling the window popups. Just waits until you've got your info put in and then prompts... Nothing special. 
      

        // Rendering
        //the weird thing about imgui is that it doesn't seem like it was originally written with Windows or DirectX in mind. There are some renormalizations
        //you have to write for the colors as it works on a 0,1 scale versus the 0,255 scale of DX. No complaints or anything but it was an interesting thing
        //to come across. Not sure why they chose a 0,1 in all honesty. 
        ImGui::EndFrame(); //you need to declare an endframe as the GPU or whatever is handling the rendering in this case needs to be told "Okay, redraw it please." 
        g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE); //this is just header stuff for DirectX
        g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);//this is just header stuff for DirectX
        g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);//this is just header stuff for DirectX
        D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f)); //colors being renormalized here, this is kind of like a "bonus" I guess. 
        //don't expect our program to look super cute or anything. 
        g_pd3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_pd3dDevice->BeginScene() >= 0)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_pd3dDevice->EndScene();
        }
        HRESULT result = g_pd3dDevice->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
            ResetDevice();
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions, I didn't write these, I just swiped them straight from the example project and documentation because I didn't really know what
// good cleanup or config looked like for this. This is my first time using a GUI library that isn't a game engine. This was an impressive struggle LOL. 
//I'll probably have my most verbose comments here as I learned the most here in particular. I didn't get to go as deeply as I would have liked into 
//the more involved graphics library handling that Imgui has employed. Maybe one day! 

bool CreateDeviceD3D(HWND hWnd)
{
    if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
        return false;

    // Create the D3DDevice
    ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
    g_d3dpp.Windowed = TRUE;
    g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3dpp.EnableAutoDepthStencil = TRUE; // stencil stuff for shader don't worry about it 
    g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
        return false;

    return true;
}

void CleanupDeviceD3D()
{
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
    if (g_pD3D) { g_pD3D->Release(); g_pD3D = nullptr; }
}

void ResetDevice()
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
    if (hr == D3DERR_INVALIDCALL)
        IM_ASSERT(0);
    ImGui_ImplDX9_CreateDeviceObjects();
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
// Win32 message handler. Did a bit of reading aobut this last night as it was my first time actually coming across it as something I could not afford
//to ignore. This is usually out of my wheelhouse as a tool developer but let's go over what I've gathered about what each of these things are. 
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) 
//This is the window procedure function. It takes four parameters: hWnd: a Handle to the window that recieved the message
//msg, the actual message code-- unsigned int, fancy. wParam and lParam are message specific information. wParam stands for word parameter and it is 
//a 16 bit parameter that carries information about the message. It's sometimes used to handle keycodes of pressed keys, sometimes for mouse messages. 
//lParam is a 32 bit parameter for passing bigger stuff. A mouse click is one thing, but a cartesian position is another. This is used for passing the
//u,v of the mouse on the screen. It also appears to be used for key combinations?  
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE: //wow, take a look at the defines for the msg types-- actual hexadecimal, real fancy. Closest I've ever been to the metal so far! 
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND: //it appears they all refer back to a hexadecimal value. Cool stuff. 
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
