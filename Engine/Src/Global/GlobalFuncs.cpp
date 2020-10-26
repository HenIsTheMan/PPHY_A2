#include "GlobalFuncs.h"
#include "../Vendor/stb_image.h"

extern bool firstCall;
extern float leftRightMB;
extern float pitch;
extern float yaw;
extern float lastX;
extern float lastY;
extern float SENS;
extern float angularFOV;
extern int optimalWinXPos;
extern int optimalWinYPos;
extern int optimalWinWidth;
extern int optimalWinHeight;
extern int winWidth;
extern int winHeight;

bool Key(const char& key){
    return GetAsyncKeyState(int(key)) & 0x8000;
}

bool InitAPI(GLFWwindow*& win){
    glfwInit();
    //glfwWindowHint(GLFW_SAMPLES, 4); //4 subsamples in a general pattern per set of screen coords of a pixel to determine pixel coverage //Better pixel coverage precision but more performance reduction with more sample pts as they cause size of... buffers to rise by...
    //Super Sample Anti-Aliasing (SSAA, draw more frags, sample pt in the center of each pixel determines if each pixel is influenced by any frag shader or not) temporarily uses a much higher resolution render buffer to render to and the resolution is downsampled back to normal after the scene is rendered
    //Result of Multisample Anti-Aliasing (MSAA) is a framebuffer with higher resolution depth or stencil buffer where primitive edges are smoother
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    #ifdef __APPLE__
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //For Mac OS X
    #endif

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    SetWindowPos(GetConsoleWindow(), 0,
        int((GetSystemMetrics(SM_CXFULLSCREEN) - float(mode->width - GetSystemMetrics(SM_CXFULLSCREEN)) / 2.f) * 5.f / 6.f),
        0,
        int((GetSystemMetrics(SM_CXFULLSCREEN) - float(mode->width - GetSystemMetrics(SM_CXFULLSCREEN)) / 2.f) / 6.f),
        GetSystemMetrics(SM_CYFULLSCREEN),
        0);
    optimalWinWidth = winWidth = int((GetSystemMetrics(SM_CXFULLSCREEN) - float(mode->width - GetSystemMetrics(SM_CXFULLSCREEN)) / 2.f) * 5.f / 6.f);
    optimalWinHeight = winHeight = GetSystemMetrics(SM_CYFULLSCREEN) - int(float(mode->height - GetSystemMetrics(SM_CYFULLSCREEN)) / 2.f);
    win = glfwCreateWindow(winWidth, winHeight, "Nameless Engine", nullptr, nullptr);
    optimalWinXPos = int(float(mode->width - GetSystemMetrics(SM_CXFULLSCREEN)) / 2.f);
    optimalWinYPos = int(float(mode->height - GetSystemMetrics(SM_CYFULLSCREEN)) / 2.f);
    glfwSetWindowPos(win, optimalWinXPos, optimalWinYPos);

    if(win == 0){ //Get a handle to the created window obj
        (void)puts("Failed to create GLFW win\n");
        return false;
    }
    glfwMakeContextCurrent(win); //Make context of the window the main context on the curr thread
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        (void)puts("Failed to init GLAD\n");
        return false;
    }
    glfwSetFramebufferSizeCallback(win, &FramebufferSizeCallback);
    glfwSetCursorPosCallback(win, CursorPosCallback);
    glfwSetMouseButtonCallback(win, MouseButtonCallback);
    glfwSetScrollCallback(win, ScrollCallback);
    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL); //Hide and capture mouse cursor

    return true;
}

bool InitConsole(){
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    SetConsoleTitleA("Nameless Console");
    system("Color 0A");

    HANDLE StdHandle = GetStdHandle(DWORD(-11));
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(StdHandle, &cursorInfo);
    cursorInfo.bVisible = 0;
    SetConsoleCursorInfo(StdHandle, &cursorInfo);

    auto ConsoleEventHandler = [](const DWORD event){
        LPCWSTR msg;
        switch(event){
            case CTRL_C_EVENT: msg = L"Ctrl + C"; break;
            case CTRL_BREAK_EVENT: msg = L"Ctrl + BREAK"; break;
            case CTRL_CLOSE_EVENT: msg = L"Closing prog..."; break;
            case CTRL_LOGOFF_EVENT: case CTRL_SHUTDOWN_EVENT: msg = L"User is logging off..."; break;
            default: msg = L"???";
        }
        MessageBox(NULL, msg, L"Nameless", MB_OK);
        return TRUE;
    };

    ::ShowWindow(::GetConsoleWindow(), SW_SHOW);
    if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleEventHandler, TRUE)){
        (void)puts("Failed to install console event handler!\n");
        return false;
    }
    return true;
}

void SetUpTex(const SetUpTexsParams& params, uint& texRefID){
    stbi_set_flip_vertically_on_load(params.flipTex); //OpenGL reads y/v tex coord in reverse so must flip tex vertically
    glGenTextures(1, &texRefID);
    int currTexRefID;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &currTexRefID);
    glBindTexture(params.texTarget, texRefID); //Make tex referenced by 'texRefIDs[i]' the tex currently bound to the currently active tex unit so subsequent tex commands will config it
    int width, height, colourChannelsAmt;
    unsigned char* data = stbi_load(params.texPath.c_str(), &width, &height, &colourChannelsAmt, 0);
    if(!data){
        return (void)printf("Failed to load tex at \"%s\"\n", params.texPath.c_str());
    }
    glTexImage2D(params.texTarget, 0, colourChannelsAmt == 3 ? GL_RGB16F : GL_RGBA16F, width, height, 0, colourChannelsAmt == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data); //Free the img mem
    glGenerateMipmap(params.texTarget); //Gen required mipmap lvls for currently bound tex
    glTexParameteri(params.texTarget, GL_TEXTURE_WRAP_S, params.texWrapParam);
    glTexParameteri(params.texTarget, GL_TEXTURE_WRAP_T, params.texWrapParam);
    glTexParameteri(params.texTarget, GL_TEXTURE_MIN_FILTER, params.texFilterMin); //Nearest neighbour/Point filtering/interpolation when textures are scaled downwards
    glTexParameteri(params.texTarget, GL_TEXTURE_MAG_FILTER, params.texFilterMag); //Linear filtering/interpolation for upscaled textures
    glBindTexture(params.texTarget, currTexRefID); //Bind previously bound tex
}

static void FramebufferSizeCallback(GLFWwindow*, int width, int height){ //Resize callback
    winWidth = width;
    winHeight = height;
}

static void CursorPosCallback(GLFWwindow*, double xPos, double yPos){
    if(firstCall){
        firstCall = 0;
    } else{ //Add mouse movement offset between last frame and curr frame
        yaw -= (float(xPos) - lastX) * SENS;
        pitch -= (float(yPos) - lastY) * SENS;
    }
    lastX = float(xPos);
    lastY = float(yPos);
}

static void MouseButtonCallback(GLFWwindow* win, int button, int action, int mods){ //For mouse buttons
    leftRightMB = float(glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) - glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT));
}

static void ScrollCallback(GLFWwindow*, double xOffset, double yOffset){
    angularFOV -= float(xOffset) + float(yOffset);
    angularFOV = std::max(1.f, std::min(75.f, angularFOV));
}