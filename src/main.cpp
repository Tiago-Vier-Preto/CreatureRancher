#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>

// Headers abaixo são específicos de C++
#include <map>
#include <stack>
#include <string>
#include <vector>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Headers das bibliotecas OpenGL
#include <glad/glad.h>   // Criação de contexto OpenGL 3.3
#include <GLFW/glfw3.h>  // Criação de janelas do sistema operacional

// Headers da biblioteca GLM: criação de matrizes e vetores.
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/type_ptr.hpp>

// Headers da biblioteca para carregar modelos obj
#include <tiny_obj_loader.h>

#include <stb_image.h>

// Headers locais, definidos na pasta "include/"
#include "utils.h"

#include "matrices.h"
#include "creature.hpp"
#include "slime_types.hpp"
#include "curve.hpp"
#include "collisions.hpp"


#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#define window_width 1280
#define window_height 720

#define map_width 300.0f
#define map_length 300.0f
#define map_height 300.0f
#define M_PI 3.141592653589793238462643383279502884L
#define NORMAL_MODE true
#define CHEAT_MODE false
#define START_MODE NORMAL_MODE
#define DEFAULT_INVENTORY_SIZE 4
#define DEFAULT_STAMINA 5.0f
// Estrutura que representa um modelo geométrico carregado a partir de um
// arquivo ".obj". Veja https://en.wikipedia.org/wiki/Wavefront_.obj_file .
struct ObjModel
{
    tinyobj::attrib_t                 attrib;
    std::vector<tinyobj::shape_t>     shapes;
    std::vector<tinyobj::material_t>  materials;

    // Este construtor lê o modelo de um arquivo utilizando a biblioteca tinyobjloader.
    // Veja: https://github.com/syoyo/tinyobjloader
    ObjModel(const char* filename, const char* basepath = NULL, bool triangulate = true)
    {
        printf("Carregando objetos do arquivo \"%s\"...\n", filename);

        // Se basepath == NULL, então setamos basepath como o dirname do
        // filename, para que os arquivos MTL sejam corretamente carregados caso
        // estejam no mesmo diretório dos arquivos OBJ.
        std::string fullpath(filename);
        std::string dirname;
        if (basepath == NULL)
        {
            auto i = fullpath.find_last_of("/");
            if (i != std::string::npos)
            {
                dirname = fullpath.substr(0, i+1);
                basepath = dirname.c_str();
            }
        }

        std::string warn;
        std::string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, basepath, triangulate);

        if (!err.empty())
            fprintf(stderr, "\n%s\n", err.c_str());

        if (!ret)
            throw std::runtime_error("Erro ao carregar modelo.");

        for (size_t shape = 0; shape < shapes.size(); ++shape)
        {
            if (shapes[shape].name.empty())
            {
                fprintf(stderr,
                        "*********************************************\n"
                        "Erro: Objeto sem nome dentro do arquivo '%s'.\n"
                        "Veja https://www.inf.ufrgs.br/~eslgastal/fcg-faq-etc.html#Modelos-3D-no-formato-OBJ .\n"
                        "*********************************************\n",
                    filename);
                throw std::runtime_error("Objeto sem nome.");
            }
            printf("- Objeto '%s'\n", shapes[shape].name.c_str());
        }

        printf("OK.\n");
    }
};

// Declaração de funções utilizadas para pilha de matrizes de modelagem.
void PushMatrix(glm::mat4 M);
void PopMatrix(glm::mat4& M);

// Declaração de várias funções utilizadas em main().  Essas estão definidas
// logo após a definição de main() neste arquivo.
void BuildTrianglesAndAddToVirtualScene(ObjModel*); // Constrói representação de um ObjModel como malha de triângulos para renderização
void ComputeNormals(ObjModel* model); // Computa normais de um ObjModel, caso não existam.
void LoadShadersFromFiles(); // Carrega os shaders de vértice e fragmento, criando um programa de GPU
void LoadTextureImage(const char* filename); // Função que carrega imagens de textura
void DrawVirtualObject(const char* object_name); // Desenha um objeto armazenado em g_VirtualScene
GLuint LoadShader_Vertex(const char* filename);   // Carrega um vertex shader
GLuint LoadShader_Fragment(const char* filename); // Carrega um fragment shader
void LoadShader(const char* filename, GLuint shader_id); // Função utilizada pelas duas acima
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id); // Cria um programa de GPU
void PrintObjModelInfo(ObjModel*); // Função para debugging
void LoadCubemap(std::vector<std::string> faces); // Função para carregar um cubemap

// Declaração de funções auxiliares para renderizar texto dentro da janela
// OpenGL. Estas funções estão definidas no arquivo "textrendering.cpp".
void TextRendering_Init();
float TextRendering_LineHeight(GLFWwindow* window);
float TextRendering_CharWidth(GLFWwindow* window);
void TextRendering_PrintString(GLFWwindow* window, const std::string &str, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrix(GLFWwindow* window, glm::mat4 M, float x, float y, float scale = 1.0f);
void TextRendering_PrintVector(GLFWwindow* window, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProduct(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductMoreDigits(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);
void TextRendering_PrintMatrixVectorProductDivW(GLFWwindow* window, glm::mat4 M, glm::vec4 v, float x, float y, float scale = 1.0f);

// Funções abaixo renderizam como texto na janela OpenGL algumas matrizes e
// outras informações do programa. Definidas após main().
void TextRendering_ShowFramesPerSecond(GLFWwindow* window);

// Funções callback para comunicação com o sistema operacional e interação do
// usuário. Veja mais comentários nas definições das mesmas, abaixo.
void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
void ErrorCallback(int error, const char* description);
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
// Definimos uma estrutura que armazenará dados necessários para renderizar
// cada objeto da cena virtual.
struct SceneObject
{
    std::string  name;        // Nome do objeto
    size_t       first_index; // Índice do primeiro vértice dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    size_t       num_indices; // Número de índices do objeto dentro do vetor indices[] definido em BuildTrianglesAndAddToVirtualScene()
    GLenum       rendering_mode; // Modo de rasterização (GL_TRIANGLES, GL_TRIANGLE_STRIP, etc.)
    GLuint       vertex_array_object_id; // ID do VAO onde estão armazenados os atributos do modelo
    glm::vec3    bbox_min; // Axis-Aligned Bounding Box do objeto
    glm::vec3    bbox_max;
};

// Abaixo definimos variáveis globais utilizadas em várias funções do código.

// A cena virtual é uma lista de objetos nomeados, guardados em um dicionário
// (map).  Veja dentro da função BuildTrianglesAndAddToVirtualScene() como que são incluídos
// objetos dentro da variável g_VirtualScene, e veja na função main() como
// estes são acessados.
std::map<std::string, SceneObject> g_VirtualScene;

// Pilha que guardará as matrizes de modelagem.
std::stack<glm::mat4>  g_MatrixStack;

// Razão de proporção da janela (largura/altura). Veja função FramebufferSizeCallback().
float g_ScreenRatio = 1.0f;

// Ângulos de Euler que controlam a rotação de um dos cubos da cena virtual
float g_AngleX = 0.0f;
float g_AngleY = 0.0f;
float g_AngleZ = 0.0f;

// "g_LeftMouseButtonPressed = true" se o usuário está com o botão esquerdo do mouse
// pressionado no momento atual. Veja função MouseButtonCallback().
bool g_LeftMouseButtonPressed = false;
bool g_RightMouseButtonPressed = false; // Análogo para botão direito do mouse
bool g_MiddleMouseButtonPressed = false; // Análogo para botão do meio do mouse

// Variáveis que definem a câmera em coordenadas esféricas, controladas pelo
// usuário através do mouse (veja função CursorPosCallback()). A posição
// efetiva da câmera é calculada dentro da função main(), dentro do loop de
// renderização.
float g_CameraTheta = 0.0f; // Ângulo no plano ZX em relação ao eixo Z
float g_CameraPhi = 0.0f;   // Ângulo em relação ao eixo Y
float g_CameraDistance = 3.5f; // Distância da câmera para a origem

glm::vec4 camera_position_c = glm::vec4(2.5f, 2.5f, 2.5f, 1.0f); // Ponto "c", centro da câmera

// Variáveis que controlam rotação do antebraço
float g_ForearmAngleZ = 0.0f;
float g_ForearmAngleX = 0.0f;

// Variáveis que controlam translação do torso
float g_TorsoPositionX = 0.0f;
float g_TorsoPositionY = 0.0f;

// Variável que controla o tipo de projeção utilizada: perspectiva ou ortográfica.
bool g_UsePerspectiveProjection = true;

// Variável que controla se o texto informativo será mostrado na tela.
bool g_ShowInfoText = true;

// Variáveis que definem um programa de GPU (shaders). Veja função LoadShadersFromFiles().
GLuint g_GpuProgramID = 0;
GLuint g_SkyboxProgramID = 0;
GLuint g_CubemapTextureID = 0;
GLint g_model_uniform;
GLint g_view_uniform;
GLint g_projection_uniform;
GLint g_object_id_uniform;
GLint g_bbox_min_uniform;
GLint g_bbox_max_uniform;
GLuint tilingLocation;

// Número de texturas carregadas pela função LoadTextureImage()
GLuint g_NumLoadedTextures = 0;

// Variáveis que guardam o estado das teclas do teclado. 
bool g_EsckeyPressed = false;


bool g_DkeyPressed = false;
bool g_AkeyPressed = false;
bool g_WkeyPressed = false;
bool g_SkeyPressed = false;

bool g_ZerokeyPressed = false;
bool g_OnekeyPressed = false;
bool g_TwokeyPressed = false;
bool g_ThreekeyPressed = false;
bool g_FourkeyPressed = false;
bool g_FivekeyPressed = false;

const float GRAVITY = -9.81f; 
const float GROUND_LEVEL = 0.0f; 

float g_CameraVerticalVelocity = 0.0f;

bool g_IsJumping = false;
bool g_Player_Started_Jumping = false;
const float JUMP_VELOCITY = 5.0f; // Initial velocity for the jump

bool g_IsSprinting = false;
const float NORMAL_SPEED = 5.0f;
const float SPRINT_BONUS = 5.0f;

enum GameState{GAME, MAIN_MENU, WIN, UPGRADE};

int main(int argc, char* argv[])
{
    // Inicializamos a biblioteca GLFW, utilizada para criar uma janela do
    // sistema operacional, onde poderemos renderizar com OpenGL.
    int success = glfwInit();
    if (!success)
    {
        fprintf(stderr, "ERROR: glfwInit() failed.\n");
        std::exit(EXIT_FAILURE);
    }

    // Definimos o callback para impressão de erros da GLFW no terminal
    glfwSetErrorCallback(ErrorCallback);

    // Pedimos para utilizar OpenGL versão 3.3 (ou superior)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    #ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    // Pedimos para utilizar o perfil "core", isto é, utilizaremos somente as
    // funções modernas de OpenGL.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GameState current_game_state = MAIN_MENU;
    GameState last_game_state = GAME;
    bool listened_to_lore[4] = {false};

    bool mode = START_MODE;

    srand(time(0));

    // Criamos uma janela do sistema operacional, com 800 colunas e 600 linhas
    // de pixels, e com título "INF01047 ...".
    GLFWwindow* window;
    window = glfwCreateWindow(window_width, window_height, "Creature Rancher", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        fprintf(stderr, "ERROR: glfwCreateWindow() failed.\n");
        std::exit(EXIT_FAILURE);
    }
    
    //Game Icon
    int width, height;
    int channels;
    unsigned char* pixels = stbi_load("../../data/icon/icon.png", &width, &height, &channels, 4);
    GLFWimage icon[1];
    icon[0].width = width;
    icon[0].height = height;
    icon[0].pixels = pixels;
    glfwSetWindowIcon(window, 1, icon);

    ma_result result;
    ma_device_config deviceConfig;
    ma_device device;
    ma_decoder decoder_game;
    ma_decoder decoder_win;
    ma_decoder decoder_menu;
    ma_decoder* currentDecoder;

    const char* game_music_filePath = "../../data/music/game.wav";
    const char* win_music_filePath = "../../data/music/win.wav";
    const char* menu_music_filePath = "../../data/music/menu.wav";
    result = ma_decoder_init_file("../../data/music/game.wav", NULL, &decoder_game);
    if (result != MA_SUCCESS) {
        printf("Failed to load game music file.\n");
        return -1;
    }

    result = ma_decoder_init_file("../../data/music/win.wav", NULL, &decoder_win);
    if (result != MA_SUCCESS) {
        printf("Failed to load win music file.\n");
        ma_decoder_uninit(&decoder_game);
        return -1;
    }

    result = ma_decoder_init_file("../../data/music/menu.wav", NULL, &decoder_menu);
    if (result != MA_SUCCESS) {
        printf("Failed to load menu music file.\n");
        ma_decoder_uninit(&decoder_game);
        ma_decoder_uninit(&decoder_win);
        return -1;
    }

    currentDecoder = &decoder_menu; // Start with game music

    // Configure playback device
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = currentDecoder->outputFormat;
    deviceConfig.playback.channels = currentDecoder->outputChannels;
    deviceConfig.sampleRate        = currentDecoder->outputSampleRate;
    deviceConfig.dataCallback      = data_callback;
    deviceConfig.pUserData         = currentDecoder;

    // Initialize the playback device
    result = ma_device_init(NULL, &deviceConfig, &device);
    if (result != MA_SUCCESS) {
        printf("Failed to initialize playback device.\n");
        ma_decoder_uninit(&decoder_game);
        ma_decoder_uninit(&decoder_win);
        ma_decoder_uninit(&decoder_menu);
        return -1;
    }

    //SFX
    ma_engine engine;
    ma_result result_sfx = ma_engine_init(NULL, &engine);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to initialize audio engine.\n");
        return -1;
    }

    const char* file_path;
    //Load Slime Jump sound effect
    file_path = "../../data/sfx/slime_jump.wav";
    ma_sound slime_jump_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &slime_jump_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load jump sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Step sound effect
    file_path = "../../data/sfx/step.wav";
    ma_sound step_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &step_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load step sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Jump sound effect
    file_path = "../../data/sfx/jump.wav";
    ma_sound jump_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &jump_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load jump sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Suction sound effect
    file_path = "../../data/sfx/suction.wav";
    ma_sound suction_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &suction_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load jump sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Lore1 sound effect
    file_path = "../../data/sfx/lore1.wav";
    ma_sound lore1_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &lore1_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load lore1 sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Lore2 sound effect
    file_path = "../../data/sfx/lore2.wav";
    ma_sound lore2_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &lore2_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load lore2 sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Lore3 sound effect
    file_path = "../../data/sfx/lore3.wav";
    ma_sound lore3_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &lore3_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load lore3 sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Ending sound effect
    file_path = "../../data/sfx/ending.wav";
    ma_sound ending_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &ending_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load ending sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Select sound effect
    file_path = "../../data/sfx/select.wav";
    ma_sound select_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &select_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load select sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Buy sound effect
    file_path = "../../data/sfx/buy.wav";
    ma_sound buy_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &buy_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load buy sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Welcome sound effect
    file_path = "../../data/sfx/welcome.wav";
    ma_sound welcome_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &welcome_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to welcome buy sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Load Goodbye sound effect
    file_path = "../../data/sfx/goodbye.wav";
    ma_sound goodbye_sound;
    result_sfx = ma_sound_init_from_file(&engine, file_path, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_NO_SPATIALIZATION, NULL, NULL, &goodbye_sound);
    if (result_sfx != MA_SUCCESS)
    {
        printf("Failed to load goodbye sound: %d\n", result_sfx);
        ma_engine_uninit(&engine);
        return -1;
    }

    // Definimos a função de callback que será chamada sempre que o usuário
    // pressionar alguma tecla do teclado ...
    glfwSetKeyCallback(window, KeyCallback);
    // ... ou clicar os botões do mouse ...
    glfwSetMouseButtonCallback(window, MouseButtonCallback);
    // ... ou movimentar o cursor do mouse em cima da janela ...
    glfwSetCursorPosCallback(window, CursorPosCallback);
    // ... ou rolar a "rodinha" do mouse.
    glfwSetScrollCallback(window, ScrollCallback);
    // Esconder o cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetCursorPos(window, window_width / 2.0, window_height / 2.0);

    // Indicamos que as chamadas OpenGL deverão renderizar nesta janela
    glfwMakeContextCurrent(window);

    // Carregamento de todas funções definidas por OpenGL 3.3, utilizando a
    // biblioteca GLAD.
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    // Definimos a função de callback que será chamada sempre que a janela for
    // redimensionada, por consequência alterando o tamanho do "framebuffer"
    // (região de memória onde são armazenados os pixels da imagem).
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    FramebufferSizeCallback(window, window_width, window_height); // Forçamos a chamada do callback acima, para definir g_ScreenRatio.

    // Carregamos os shaders de vértices e de fragmentos que serão utilizados
    // para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
    LoadShadersFromFiles();

    // Carregamos duas imagens para serem utilizadas como textura
    LoadTextureImage("../../data/tc-earth_daymap_surface.jpg");      // TextureImage0
    LoadTextureImage("../../data/tc-earth_nightmap_citylights.gif"); // TextureImage1
    LoadTextureImage("../../data/planes/base.jpg"); // TextureImage2
    
    LoadTextureImage("../../data/anemo-slime/textures/bake.png"); // TextureImage3
    LoadTextureImage("../../data/cryo-slime/textures/bake.png"); // TextureImage4
    LoadTextureImage("../../data/dendro-slime/textures/body_bake.png"); // TextureImage5
    LoadTextureImage("../../data/plasma-slime/textures/final_bake.png"); // TextureImage6
    LoadTextureImage("../../data/fire-slime/textures/body.png"); // TextureImage7
    LoadTextureImage("../../data/geo-slime/textures/bake.png"); // TextureImage8
    LoadTextureImage("../../data/electro-slime/textures/final_bake.png"); // TextureImage9
    LoadTextureImage("../../data/water-slime/textures/final_bake.001.png"); // TextureImage10

    std::vector<std::string> faces
    {
        "../../data/skybox/right.jpg",
        "../../data/skybox/left.jpg",
        "../../data/skybox/top.jpg",
        "../../data/skybox/bottom.jpg",
        "../../data/skybox/back.jpg",
        "../../data/skybox/front.jpg"
    };
    stbi_set_flip_vertically_on_load(false);
    LoadCubemap(faces);
    stbi_set_flip_vertically_on_load(true);

    // Texturas da arma
    LoadTextureImage("../../data/weapon/textures/AOMaterial.png"); // TextureImage12
    LoadTextureImage("../../data/weapon/textures/BASECOLOR_Material.png"); // TextureImage13
    LoadTextureImage("../../data/weapon/textures/CURVATUREMaterial.png"); // TextureImage14
    LoadTextureImage("../../data/weapon/textures/EMISSIVE_Material.png"); // TextureImage15
    LoadTextureImage("../../data/weapon/textures/METALLICMaterial.png"); // TextureImage16
    LoadTextureImage("../../data/weapon/textures/NORMAL_Material.png"); // TextureImage17
    LoadTextureImage("../../data/weapon/textures/OPACITYMaterial.png"); // TextureImage18
    LoadTextureImage("../../data/weapon/textures/ROUGHNESS.png"); // TextureImage19

    //Texturas de Terrenos
    LoadTextureImage("../../data/planes/cloud.jpg"); // TextureImage20
    LoadTextureImage("../../data/planes/snow_land.jpg"); // TextureImage21
    LoadTextureImage("../../data/planes/wood_forest.jpg"); // TextureImage22
    LoadTextureImage("../../data/planes/electric_fields.jpg"); // TextureImage23
    LoadTextureImage("../../data/planes/base.jpg"); // TextureImage24
    LoadTextureImage("../../data/planes/burnt_land.jpg"); // TextureImage25
    LoadTextureImage("../../data/planes/rocky_ground.jpg"); // TextureImage26
    LoadTextureImage("../../data/planes/factory.jpg"); // TextureImage27
    LoadTextureImage("../../data/planes/watery_mud.jpg"); // TextureImage28

    std::vector<std::string> faces_heaven
    {
        "../../data/heaven_skybox/right.jpg",
        "../../data/heaven_skybox/left.jpg",
        "../../data/heaven_skybox/top.jpg",
        "../../data/heaven_skybox/bottom.jpg",
        "../../data/heaven_skybox/back.jpg",
        "../../data/heaven_skybox/front.jpg"
    };
    stbi_set_flip_vertically_on_load(false);
    LoadCubemap(faces_heaven);
    stbi_set_flip_vertically_on_load(true);

    LoadTextureImage("../../data/god/god.png"); // 30

    LoadTextureImage("../../data/menu/menu.png"); // 31
    LoadTextureImage("../../data/menu/menu.png"); // 31
    LoadTextureImage("../../data/menu/controls.jpg"); // 32
    LoadTextureImage("../../data/upgrades/upgrades.png"); // 33

    LoadTextureImage("../../data/store-monster/store-monster.png"); // 34

    // Construímos a representação de objetos geométricos através de malhas de triângulos
    ObjModel planemodel("../../data/plane.obj");
    ComputeNormals(&planemodel);
    BuildTrianglesAndAddToVirtualScene(&planemodel);

    ObjModel anemomodel("../../data/anemo-slime/source/anemo.obj");
    ComputeNormals(&anemomodel);
    BuildTrianglesAndAddToVirtualScene(&anemomodel);

    ObjModel cryomodel("../../data/cryo-slime/source/cryo.obj");
    ComputeNormals(&cryomodel);
    BuildTrianglesAndAddToVirtualScene(&cryomodel);

    ObjModel dendromodel("../../data/dendro-slime/source/dendro.obj");
    ComputeNormals(&dendromodel);
    BuildTrianglesAndAddToVirtualScene(&dendromodel);

    ObjModel plasmamodel("../../data/plasma-slime/source/plasma.obj");
    ComputeNormals(&plasmamodel);
    BuildTrianglesAndAddToVirtualScene(&plasmamodel);

    ObjModel firemodel("../../data/fire-slime/source/fire.obj");
    ComputeNormals(&firemodel);
    BuildTrianglesAndAddToVirtualScene(&firemodel);

    ObjModel geomodel("../../data/geo-slime/source/geo.obj");
    ComputeNormals(&geomodel);
    BuildTrianglesAndAddToVirtualScene(&geomodel);

    ObjModel electromodel("../../data/electro-slime/source/electro.obj");
    ComputeNormals(&electromodel);
    BuildTrianglesAndAddToVirtualScene(&electromodel);

    ObjModel watermodel("../../data/water-slime/source/water.obj");
    ComputeNormals(&watermodel);
    BuildTrianglesAndAddToVirtualScene(&watermodel);

    ObjModel cubemodel("../../data/skybox/skybox.obj");
    ComputeNormals(&cubemodel);
    BuildTrianglesAndAddToVirtualScene(&cubemodel);

    ObjModel weapon("../../data/weapon/source/weapon.obj");
    ComputeNormals(&weapon);
    BuildTrianglesAndAddToVirtualScene(&weapon);

    ObjModel heaven_cubemodel("../../data/heaven_skybox/heaven_skybox.obj");
    ComputeNormals(&heaven_cubemodel);
    BuildTrianglesAndAddToVirtualScene(&heaven_cubemodel);

    ObjModel god("../../data/god/god.obj");
    ComputeNormals(&god);
    BuildTrianglesAndAddToVirtualScene(&god);

    ObjModel menu("../../data/menu/menu.obj");
    ComputeNormals(&menu);
    BuildTrianglesAndAddToVirtualScene(&menu);

    ObjModel store_monster("../../data/store-monster/store-monster.obj");
    ComputeNormals(&store_monster);
    BuildTrianglesAndAddToVirtualScene(&store_monster);

    if ( argc > 1 )
    {
        ObjModel model(argv[1]);
        BuildTrianglesAndAddToVirtualScene(&model);
    }

    // Inicializamos o código para renderização de texto.
    TextRendering_Init();

    // Habilitamos o Z-buffer. Veja slides 104-116 do documento Aula_09_Projecoes.pdf.
    glEnable(GL_DEPTH_TEST);

    // Habilitamos o Backface Culling. Veja slides 8-13 do documento Aula_02_Fundamentos_Matematicos.pdf, slides 23-34 do documento Aula_13_Clipping_and_Culling.pdf e slides 112-123 do documento Aula_14_Laboratorio_3_Revisao.pdf.
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    float prev_time = (float)glfwGetTime();

    std::vector<Creature*> creatures = InitialCreatureSpawn(STARTING_SLIMES, map_width, map_length);
    int slime_count = STARTING_SLIMES;
    float captureTime = 0.0f;
    glm::vec4 lastPosition;
    float lastCaptureTime = 0.0f;

    //Player inventory and balance
    std::map<Slime_Type, int> balance = {
        {ANEMO, 0},
        {CRYO, 0},
        {DENDRO, 0},
        {PLASMA, 0},
        {FIRE, 0},
        {GEO, 0},
        {ELECTRO, 0},
        {WATER, 0}
    };
    std::vector<Slime_Type> inventory = {};

    float stamina_counter = DEFAULT_STAMINA;

    int movement_speed_level = 0;
    std::map<Slime_Type, std::vector<int>> movement_speed_costs = {
    {ANEMO, {0, 2, 5}},
    {CRYO, {0, 0, 0}},
    {DENDRO, {0, 0, 0}},
    {PLASMA, {0, 0, 0}},
    {FIRE, {0, 0, 0}},
    {GEO, {0, 0, 0}},
    {ELECTRO, {2, 3, 5}},
    {WATER, {0, 0, 0}}
    };

    int stamina_level = 0;
    std::map<Slime_Type, std::vector<int>> stamina_costs = {
    {ANEMO, {0, 0, 0}},
    {CRYO, {0, 0, 0}},
    {DENDRO, {0, 0, 0}},
    {PLASMA, {0, 0, 0}},
    {FIRE, {2, 3, 5}},
    {GEO, {0, 2, 5}},
    {ELECTRO, {0, 0, 0}},
    {WATER, {0, 0, 0}}
    };

    int slime_spawn_rate_level = 0;
    std::map<Slime_Type, std::vector<int>> slime_spawn_rate_costs = {
    {ANEMO, {0, 0, 0}},
    {CRYO, {0, 0, 0}},
    {DENDRO, {0, 2, 5}},
    {PLASMA, {2, 3, 5}},
    {FIRE, {0, 0, 0}},
    {GEO, {0, 0, 0}},
    {ELECTRO, {0, 0, 0}},
    {WATER, {0, 2, 5}}
    };

    int inventory_level = 0;
    std::map<Slime_Type, std::vector<int>> inventory_costs = {
    {ANEMO, {0, 0, 0}},
    {CRYO, {2, 3, 5}},
    {DENDRO, {0, 0, 0}},
    {PLASMA, {0, 0, 0}},
    {FIRE, {0, 0, 0}},
    {GEO, {0, 0, 0}},
    {ELECTRO, {0, 0, 0}},
    {WATER, {0, 2, 5}}
    };

    int lore_progress_level = 0;
    std::map<Slime_Type, std::vector<int>> lore_progress_costs = {
    {ANEMO, {1, 4, 10}},
    {CRYO, {4, 6, 10}},
    {DENDRO, {1, 4, 10}},
    {PLASMA, {4, 6, 10}},
    {FIRE, {4, 6, 10}},
    {GEO, {1, 4, 10}},
    {ELECTRO, {4, 6, 10}},
    {WATER, {1, 5, 10}}
    };

    std::vector<std::pair<int, int>> potentialCollisions;

    static float slime_spawn_timer = 0.0f;

    // Start playback
    result = ma_device_start(&device);
    if (result != MA_SUCCESS) {
        printf("Failed to start playback device.\n");
        ma_device_uninit(&device);
        ma_decoder_uninit(&decoder_game);
        ma_decoder_uninit(&decoder_win);
        ma_decoder_uninit(&decoder_menu);
        return -1;
    }

    // Ficamos em um loop infinito, renderizando, até que o usuário feche a janela
    while (!glfwWindowShouldClose(window))
    {
        // Aqui executamos as operações de renderização
        switch(current_game_state)
        {
            case MAIN_MENU:
            {
                if (g_EsckeyPressed)
                {
                    glfwSetWindowShouldClose(window, GL_TRUE);
                    break;
                }
                else if(g_OnekeyPressed)
                {
                    current_game_state = last_game_state;
                    if(current_game_state == GAME)
                    {
                        currentDecoder = &decoder_game;
                    }
                    else if(current_game_state == WIN)
                    {
                        currentDecoder = &decoder_win;
                    }
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }
                    g_OnekeyPressed = false;
                }
                else if(g_TwokeyPressed)
                {
                    ma_sound_start(&select_sound);
                    mode = CHEAT_MODE;
                    for (auto& pair : balance) 
                    {
                        pair.second += 99;
                    }
                    g_TwokeyPressed = false;
                }
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glUseProgram(g_GpuProgramID);
                glm::vec3 menu_center = glm::vec3(0.0f, 0.0f, 0.0f);
                float menu_width = 2.0f;
                float menu_height = 2.0f;
                glm::vec4 camera_position_c = glm::vec4(0.0f, 0.0f, 1.5f, 1.0f); // Ponto "c", centro da câmera
                glm::vec4 camera_lookat_l = glm::vec4(menu_center, 1.0f);  // Ponto "l", para onde a câmera (look-at) estará sempre olhando
                glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
                glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
                glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
                glm::mat4 projection;
                float nearplane = -0.1f;
                float farplane  = -1000.0f;
                if (g_UsePerspectiveProjection)
                {
                    float field_of_view = 3.141592f / 3.0f;
                    projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
                }
                else
                {
                    float t = 1.5f*g_CameraDistance/2.5f;
                    float b = -t;
                    float r = t*g_ScreenRatio;
                    float l = -r;
                    projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
                }

                glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
                glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
                #define MENU 31
                #define CONTROLS 32
                glm::mat4 model = Matrix_Identity();
                model = Matrix_Translate(menu_center.x, menu_center.y, menu_center.z)
                        * Matrix_Scale(1.54f, 0.88f, 1.0f);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                if(g_ThreekeyPressed)
                {
                    glUniform1i(g_object_id_uniform, CONTROLS);
                    glUniform2f(tilingLocation, 1.0f, 1.0f);
                    DrawVirtualObject("menu");
                }
                else
                {
                    glUniform1i(g_object_id_uniform, MENU);
                    glUniform2f(tilingLocation, 1.0f, 1.0f);
                    DrawVirtualObject("menu");
                }

                TextRendering_ShowFramesPerSecond(window);
                glfwSwapBuffers(window);
                glfwPollEvents();
                break;
            }
            case UPGRADE:
            {
                if (g_EsckeyPressed)
                {
                    ma_sound_start(&goodbye_sound);
                    current_game_state = GAME;
                    g_EsckeyPressed = false;
                    break;
                }
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glUseProgram(g_GpuProgramID);
                glm::vec3 menu_center = glm::vec3(0.0f, 0.0f, 0.0f);
                float menu_width = 2.0f;
                float menu_height = 2.0f;
                glm::vec4 camera_position_c = glm::vec4(0.0f, 0.0f, 1.5f, 1.0f); // Ponto "c", centro da câmera
                glm::vec4 camera_lookat_l = glm::vec4(menu_center, 1.0f);  // Ponto "l", para onde a câmera (look-at) estará sempre olhando
                glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
                glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
                glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
                glm::mat4 projection;
                float nearplane = -0.1f;
                float farplane  = -1000.0f;
                if (g_UsePerspectiveProjection)
                {
                    float field_of_view = 3.141592f / 3.0f;
                    projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
                }
                else
                {
                    float t = 1.5f*g_CameraDistance/2.5f;
                    float b = -t;
                    float r = t*g_ScreenRatio;
                    float l = -r;
                    projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
                }

                glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
                glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
                #define UPGRADES 33
                glm::mat4 model = Matrix_Identity();
                model = Matrix_Translate(menu_center.x, menu_center.y, menu_center.z)
                        * Matrix_Scale(1.54f, 0.88f, 1.0f);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, UPGRADES);
                glUniform2f(tilingLocation, 1.0f, 1.0f);
                DrawVirtualObject("menu");

                //Text
                float line_spacing = 0.165f;
                float x = -0.99f;
                float start_y = 0.9f;
                float scale = 1.5f;
                std::string constructed_string;
                TextRendering_PrintString(window, "Upgrades (from level 0 to 3):", x, start_y, scale);
                TextRendering_PrintString(window, "(1) Movement Speed Level " + std::to_string(movement_speed_level), x, start_y - line_spacing, scale);
                if(movement_speed_level < 3)
                {
                    constructed_string = "";
                    for (const auto& pair : movement_speed_costs) 
                    {
                        constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second[movement_speed_level]) + ";";
                    }
                }
                else
                {
                    constructed_string = "All upgrades purchased!";
                }
                TextRendering_PrintString(window, "Cost: " + constructed_string, x, start_y - line_spacing * 2, scale);
                TextRendering_PrintString(window, "(2) Stamina Level " + std::to_string(stamina_level), x, start_y - line_spacing * 3, scale);
                if(stamina_level < 3)
                {
                    constructed_string = "";
                    for (const auto& pair : stamina_costs) 
                    {
                        constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second[stamina_level]) + ";";
                    }
                }
                else
                {
                    constructed_string = "All upgrades purchased!";
                }
                TextRendering_PrintString(window, "Cost: " + constructed_string, x, start_y - line_spacing * 4, scale);
                TextRendering_PrintString(window, "(3) Slime Spawn Rate Level " + std::to_string(slime_spawn_rate_level), x, start_y - line_spacing * 5, scale);
                if(slime_spawn_rate_level < 3)
                {
                    constructed_string = "";
                    for (const auto& pair : slime_spawn_rate_costs) 
                    {
                        constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second[slime_spawn_rate_level]) + ";";
                    }
                }
                else
                {
                    constructed_string = "All upgrades purchased!";
                }
                TextRendering_PrintString(window, "Cost: " + constructed_string, x, start_y - line_spacing * 6, scale);
                TextRendering_PrintString(window, "(4) Inventory Level " + std::to_string(inventory_level), x, start_y - line_spacing * 7, scale);
                if(inventory_level < 3)
                {
                    constructed_string = "";
                    for (const auto& pair : inventory_costs) 
                    {
                        constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second[inventory_level]) + ";";
                    }
                }
                else
                {
                    constructed_string = "All upgrades purchased!";
                }
                TextRendering_PrintString(window, "Cost: " + constructed_string, x, start_y - line_spacing * 8, scale);
                if(lore_progress_level < 3)
                {
                    constructed_string = "";
                    for (const auto& pair : lore_progress_costs) 
                    {
                        constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second[lore_progress_level]) + ";";
                    }
                }
                else
                {
                    constructed_string = "All upgrades purchased!";
                }
                TextRendering_PrintString(window, "(5) Progress Lore Level " + std::to_string(lore_progress_level), x, start_y - line_spacing * 9, scale);
                TextRendering_PrintString(window, "Cost: " + constructed_string, x, start_y - line_spacing * 10, scale);
                constructed_string = "";
                for (const auto& pair : balance) 
                {
                    constructed_string += to_string(pair.first) + ":" + std::to_string(pair.second) + ";";
                }
                TextRendering_PrintString(window, "Balance: " + constructed_string, x, start_y - line_spacing * 11, scale);
                TextRendering_ShowFramesPerSecond(window);

                //Purchase Logic
                if (g_OnekeyPressed && movement_speed_level < 3) 
                {
                    bool canAfford = true;
                    for (const auto& pair : movement_speed_costs) 
                    {
                        if (balance[pair.first] < pair.second[movement_speed_level]) 
                        {
                            canAfford = false;
                            break;
                        }
                    }
                    if (canAfford) 
                    {
                        ma_sound_seek_to_pcm_frame(&buy_sound, 0.0);
                        ma_sound_start(&buy_sound);
                        for (const auto& pair : movement_speed_costs) 
                        {
                            balance[pair.first] -= pair.second[movement_speed_level];
                        }
                        movement_speed_level++;
                    }
                    g_OnekeyPressed = false;
                }

                if (g_TwokeyPressed && stamina_level < 3) 
                {
                    bool canAfford = true;
                    for (const auto& pair : stamina_costs) 
                    {
                        if (balance[pair.first] < pair.second[stamina_level]) 
                        {
                            canAfford = false;
                            break;
                        }
                    }
                    if (canAfford) 
                    {
                        ma_sound_seek_to_pcm_frame(&buy_sound, 0.0);
                        ma_sound_start(&buy_sound);
                        for (const auto& pair : stamina_costs) 
                        {
                            balance[pair.first] -= pair.second[stamina_level];
                        }
                        stamina_level++;
                    }
                    g_TwokeyPressed = false;
                }

                if (g_ThreekeyPressed && slime_spawn_rate_level < 3) 
                {
                    bool canAfford = true;
                    for (const auto& pair : slime_spawn_rate_costs) 
                    {
                        if (balance[pair.first] < pair.second[slime_spawn_rate_level]) 
                        {
                            canAfford = false;
                            break;
                        }
                    }
                    if (canAfford) 
                    {
                        ma_sound_seek_to_pcm_frame(&buy_sound, 0.0);
                        ma_sound_start(&buy_sound);
                        for (const auto& pair : slime_spawn_rate_costs) 
                        {
                            balance[pair.first] -= pair.second[slime_spawn_rate_level];
                        }
                        slime_spawn_rate_level++;
                    }
                    g_ThreekeyPressed = false;
                }

                if (g_FourkeyPressed && inventory_level < 3) 
                {
                    bool canAfford = true;
                    for (const auto& pair : inventory_costs) 
                    {
                        if (balance[pair.first] < pair.second[inventory_level]) 
                        {
                            canAfford = false;
                            break;
                        }
                    }
                    if (canAfford) 
                    {
                        ma_sound_seek_to_pcm_frame(&buy_sound, 0.0);
                        ma_sound_start(&buy_sound);
                        for (const auto& pair : inventory_costs) 
                        {
                            balance[pair.first] -= pair.second[inventory_level];
                        }
                        inventory_level++;
                    }
                    g_FourkeyPressed = false;
                }

                if (g_FivekeyPressed && lore_progress_level < 3) 
                {
                    bool canAfford = true;
                    for (const auto& pair : lore_progress_costs) 
                    {
                        if (balance[pair.first] < pair.second[lore_progress_level]) 
                        {
                            canAfford = false;
                            break;
                        }
                    }
                    if (canAfford) 
                    {
                        ma_sound_seek_to_pcm_frame(&buy_sound, 0);
                        ma_sound_start(&buy_sound);
                        for (const auto& pair : lore_progress_costs) 
                        {
                            balance[pair.first] -= pair.second[lore_progress_level];
                        }
                        lore_progress_level++;
                    }
                    g_FivekeyPressed = false;
                }
                glfwSwapBuffers(window);
                glfwPollEvents();
                break;
            }
            case WIN:
            {
                if (g_EsckeyPressed)
                {
                    ma_sound_start(&select_sound);
                    g_EsckeyPressed = false;
                    current_game_state = MAIN_MENU;
                    last_game_state = WIN;
                    currentDecoder = &decoder_menu;
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }
                }
                else if(g_ZerokeyPressed)
                {
                    current_game_state = GAME;
                    g_ZerokeyPressed = false;
                    currentDecoder = &decoder_game;
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }
                }
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glUseProgram(g_GpuProgramID);
                float r = g_CameraDistance + 250;
                float y = r*sin(g_CameraPhi);
                float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
                float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);
                glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
                glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando
                glm::vec4 camera_view_vector = camera_lookat_l - camera_position_c;
                glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f);
                glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);
                glm::mat4 projection;
                float nearplane = -0.1f;
                float farplane  = -1000.0f;
                if (g_UsePerspectiveProjection)
                {
                    float field_of_view = 3.141592f / 3.0f;
                    projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
                }
                else
                {
                    float t = 1.5f*g_CameraDistance/2.5f;
                    float b = -t;
                    float r = t*g_ScreenRatio;
                    float l = -r;
                    projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
                }
                glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
                glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));
                
                glm::mat4 model = Matrix_Identity();
                #define HEAVEN_CUBE 29
                #define GOD 30
                model = Matrix_Translate(0.0f,0.0f,0.0f)
                        * Matrix_Scale(map_width, map_height, map_length);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, HEAVEN_CUBE);
                glUniform2f(tilingLocation, 1.0f, 1.0f);
                DrawVirtualObject("heaven_cube");

                model = Matrix_Translate(0.0f,0.0f,0.0f)
                        * Matrix_Scale(map_width, map_height, map_length);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, GOD);
                glUniform2f(tilingLocation, 1.0f, 1.0f);
                DrawVirtualObject("god");

                TextRendering_ShowFramesPerSecond(window);
                glfwSwapBuffers(window);
                glfwPollEvents();
                break;
            }

            case GAME:
            {
                if (g_EsckeyPressed)
                {
                    ma_sound_start(&select_sound);
                    g_EsckeyPressed = false;
                    current_game_state = MAIN_MENU;
                    last_game_state = GAME;
                    currentDecoder = &decoder_menu;
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }
                }
                // Aqui executamos as operações de renderização

                // Definimos a cor do "fundo" do framebuffer como branco.  Tal cor é
                // definida como coeficientes RGBA: Red, Green, Blue, Alpha; isto é:
                // Vermelho, Verde, Azul, Alpha (valor de transparência).
                // Conversaremos sobre sistemas de cores nas aulas de Modelos de Iluminação.
                //
                //           R     G     B     A
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

                // "Pintamos" todos os pixels do framebuffer com a cor definida acima,
                // e também resetamos todos os pixels do Z-buffer (depth buffer).
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                // Pedimos para a GPU utilizar o programa de GPU criado acima (contendo
                // os shaders de vértice e fragmentos).
                glUseProgram(g_GpuProgramID);

                // Computamos a posição da câmera utilizando coordenadas esféricas.  As
                // variáveis g_CameraDistance, g_CameraPhi, e g_CameraTheta são
                // controladas pelo mouse do usuário. Veja as funções CursorPosCallback()
                // e ScrollCallback().
                float r = g_CameraDistance;
                float y = r*sin(g_CameraPhi);
                float z = r*cos(g_CameraPhi)*cos(g_CameraTheta);
                float x = r*cos(g_CameraPhi)*sin(g_CameraTheta);

                // Abaixo definimos as varáveis que efetivamente definem a câmera virtual.
                // Veja slides 195-227 e 229-234 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
                // glm::vec4 camera_position_c  = glm::vec4(x,y,z,1.0f); // Ponto "c", centro da câmera
                // glm::vec4 camera_lookat_l    = glm::vec4(0.0f,0.0f,0.0f,1.0f); // Ponto "l", para onde a câmera (look-at) estará sempre olhando

                glm::vec4 camera_view_vector = glm::vec4(-x, -y, -z, 0.0f); // Vetor "view", sentido para onde a câmera está virada
                glm::vec4 camera_up_vector   = glm::vec4(0.0f,1.0f,0.0f,0.0f); // Vetor "up" fixado para apontar para o "céu" (eito Y global)

                glm::vec4 w_vector = -camera_view_vector;
                glm::vec4 u_vector = crossproduct(camera_up_vector, w_vector);

                u_vector = u_vector / norm(u_vector);
                w_vector = w_vector / norm(w_vector);

                float current_time = (float)glfwGetTime();
                float delta_t = current_time - prev_time;
                slime_spawn_timer += delta_t;
                prev_time = current_time;
                float speed;
                if(g_IsSprinting)
                {
                    if(stamina_counter > 0.0f)
                    {
                        stamina_counter -= delta_t;
                        speed = NORMAL_SPEED + float(movement_speed_level) + SPRINT_BONUS;
                    }
                    else
                    {
                        stamina_counter = 0.0f;
                        speed = NORMAL_SPEED + float(movement_speed_level);
                    }
                }
                else
                {
                    stamina_counter += delta_t;
                    speed = NORMAL_SPEED + float(movement_speed_level);
                    if(stamina_counter > DEFAULT_STAMINA + stamina_level)
                    {
                        stamina_counter = DEFAULT_STAMINA + stamina_level;
                    }
                }

                bool started_jumping;
                // Initialize variables to track the loudest jump
                float maxVolume = 0.0f;
                Creature* loudestCreature = nullptr;

                // Loop through all creatures to update and find the loudest jump
                for (auto& creature : creatures) {
                    bool started_jumping = creature->Update(delta_t);
                    if (started_jumping) {
                        glm::vec3 playerPos = glm::vec3(camera_position_c);
                        glm::vec3 slimePos = glm::vec3(creature->GetPosition());
                        float distance = glm::distance(playerPos, slimePos);

                        // Normalize distance to volume
                        float maxDistance = 50.0f; // Maximum distance to hear sound
                        float volume = 1.0f - glm::clamp(distance / maxDistance, 0.0f, 1.0f);

                        // Track the loudest jump
                        if (volume > maxVolume) {
                            maxVolume = volume;
                            loudestCreature = creature;
                        }
                    }
                }

                // Play the sound for the loudest jump if any
                if (loudestCreature != nullptr) {
                    ma_sound_set_volume(&slime_jump_sound, maxVolume);
                    ma_sound_start(&slime_jump_sound);
                }
                if ((slime_spawn_timer >= std::max((SLIME_SPAWN_TIME - float(slime_spawn_rate_level)), 1.0f)) && slime_count < SLIME_LIMIT) {
                    // Reset the spawn timer
                    slime_spawn_timer = 0.0f;

                    // Spawn a new slime and add it to the creatures vector
                    Creature* new_slime = SpawnCreature(map_width, map_length, creatures); // Adjust SpawnCreature parameters as needed
                    slime_count++;
                    creatures.push_back(new_slime);
                }

                g_CameraVerticalVelocity += GRAVITY * delta_t;
                camera_position_c.y += g_CameraVerticalVelocity * delta_t;

                if (camera_position_c.y < GROUND_LEVEL)
                {
                    camera_position_c.y = GROUND_LEVEL;
                    g_CameraVerticalVelocity = 0.0f; // reseta a velocidade vertical quando houver colisao com o chao
                    g_IsJumping = false;
                }

                // Atualizamos a posição da câmera utilizando as teclas W, A, S, D
                bool playerMoved = false;
                if (g_WkeyPressed) {
                    camera_position_c += -w_vector * speed * delta_t;
                    playerMoved = true;
                }
                if (g_SkeyPressed) {
                    camera_position_c += w_vector * speed * delta_t;
                    playerMoved = true;
                }
                if (g_AkeyPressed) {
                    camera_position_c += -u_vector * speed * delta_t;
                    playerMoved = true;
                }
                if (g_DkeyPressed) {
                    camera_position_c += u_vector * speed * delta_t;
                    playerMoved = true;
                }

                // Play Step sound if the player moved
                if (playerMoved && !g_IsJumping) {
                    ma_sound_set_volume(&step_sound, g_IsSprinting ? 1.0f : 0.8f);
                    ma_sound_set_pitch(&step_sound, g_IsSprinting ? 1.2f : 1.0f); // Increase pitch when sprinting
                    ma_sound_start(&step_sound);
                }
                if (g_Player_Started_Jumping) {
                    ma_sound_start(&jump_sound);
                    g_Player_Started_Jumping = false;
                }
                
                // Computamos a matriz "View" utilizando os parâmetros da câmera para
                // definir o sistema de coordenadas da câmera.  Veja slides 2-14, 184-190 e 236-242 do documento Aula_08_Sistemas_de_Coordenadas.pdf.
                glm::mat4 view = Matrix_Camera_View(camera_position_c, camera_view_vector, camera_up_vector);

                // Agora computamos a matriz de Projeção.
                glm::mat4 projection;

                // Note que, no sistema de coordenadas da câmera, os planos near e far
                // estão no sentido negativo! Veja slides 176-204 do documento Aula_09_Projecoes.pdf.
                float nearplane = -0.1f;  // Posição do "near plane"
                float farplane  = -1000.0f; // Posição do "far plane"

                if (g_UsePerspectiveProjection)
                {
                    // Projeção Perspectiva.
                    // Para definição do field of view (FOV), veja slides 205-215 do documento Aula_09_Projecoes.pdf.
                    float field_of_view = 3.141592f / 3.0f;
                    projection = Matrix_Perspective(field_of_view, g_ScreenRatio, nearplane, farplane);
                }
                else
                {
                    // Projeção Ortográfica.
                    // Para definição dos valores l, r, b, t ("left", "right", "bottom", "top"),
                    // PARA PROJEÇÃO ORTOGRÁFICA veja slides 219-224 do documento Aula_09_Projecoes.pdf.
                    // Para simular um "zoom" ortográfico, computamos o valor de "t"
                    // utilizando a variável g_CameraDistance.
                    float t = 1.5f*g_CameraDistance/2.5f;
                    float b = -t;
                    float r = t*g_ScreenRatio;
                    float l = -r;
                    projection = Matrix_Orthographic(l, r, b, t, nearplane, farplane);
                }

                glm::mat4 model = Matrix_Identity(); // Transformação identidade de modelagem

                // Enviamos as matrizes "view" e "projection" para a placa de vídeo
                // (GPU). Veja o arquivo "shader_vertex.glsl", onde estas são
                // efetivamente aplicadas em todos os pontos.
                glUniformMatrix4fv(g_view_uniform       , 1 , GL_FALSE , glm::value_ptr(view));
                glUniformMatrix4fv(g_projection_uniform , 1 , GL_FALSE , glm::value_ptr(projection));

                #define CREATURE 3
                #define CUBE     11
                #define WEAPON   12
                #define PRIMEIRO_PLANO 20
                #define STORE_MONSTER 34
                // Desenhamos o plano do chão
                for(int i = 0; i < 9; i++)
                {
                    model = Matrix_Translate(-200.0f + 200 * (i % 3),-1.1f,-200.0f + 200 * (i / 3))
                        * Matrix_Scale(100, 1.0f, 100);
                    glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                    glUniform1i(g_object_id_uniform, 20 + i);
                    glUniform2f(tilingLocation, 10.0f, 10.0f);
                    DrawVirtualObject("the_plane");
                }

                glm::vec4 weapon_position = camera_position_c + 0.4f * normalize(camera_view_vector) - 0.25f * normalize(crossproduct(camera_up_vector, camera_view_vector)) - 0.1f * camera_up_vector;
                glm::vec4 weapon_direction = normalize(camera_view_vector);
                glm::vec4 weapon_right = normalize(crossproduct(camera_up_vector, weapon_direction));
                glm::vec4 weapon_up = normalize(crossproduct(weapon_direction, weapon_right));

                glm::mat4 weapon_rotation = glm::mat4(weapon_right.x, weapon_right.y, weapon_right.z, 0.0f,
                                                    weapon_up.x, weapon_up.y, weapon_up.z, 0.0f,
                                                    weapon_direction.x, weapon_direction.y, weapon_direction.z, 0.0f,
                                                    0.0f, 0.0f, 0.0f, 1.0f);

                model = Matrix_Translate(weapon_position.x, weapon_position.y, weapon_position.z)
                    * weapon_rotation
                    * Matrix_Scale(0.001f, 0.001f, 0.001f); 

                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, WEAPON);
                glUniform2f(tilingLocation, 1.0f, 1.0f);
                DrawVirtualObject("weapon");

                glm::vec3 cubeCenter = glm::vec3(0.0f, 0.0f, 0.0f);
                glm::vec3 cubeSize = glm::vec3(map_width, map_height, map_length);

                AABB frontFace;
                frontFace.min = cubeCenter + glm::vec3(-cubeSize.x, -cubeSize.y, cubeSize.z);
                frontFace.max = cubeCenter + glm::vec3(cubeSize.x, cubeSize.y, cubeSize.z);
                glm::vec4 frontFaceDirection = glm::vec4(0.0f, 0.0f, 1.0f, 0.0f);

                AABB backFace;
                backFace.min = cubeCenter + glm::vec3(-cubeSize.x, -cubeSize.y, -cubeSize.z);
                backFace.max = cubeCenter + glm::vec3(cubeSize.x, cubeSize.y, -cubeSize.z);
                glm::vec4 backFaceDirection = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);

                AABB leftFace;
                leftFace.min = cubeCenter + glm::vec3(-cubeSize.x, -cubeSize.y, -cubeSize.z);
                leftFace.max = cubeCenter + glm::vec3(-cubeSize.x, cubeSize.y, cubeSize.z);
                glm::vec4 leftFaceDirection = glm::vec4(-1.0f, 0.0f, 0.0f, 0.0f);

                AABB rightFace;
                
                rightFace.min = cubeCenter + glm::vec3(cubeSize.x, -cubeSize.y, -cubeSize.z);
                rightFace.max = cubeCenter + glm::vec3(cubeSize.x, cubeSize.y, cubeSize.z);
                glm::vec4 rightFaceDirection = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

                AABB cameraAABB = ComputeAABB(glm::vec3(camera_position_c), glm::vec3(0.7f, 0.7f, 2.5f));

                // Fase de colisao Broad Phase
                if (CheckAABBOverlap(cameraAABB, frontFace)) potentialCollisions.push_back({-2, 0});
                if (CheckAABBOverlap(cameraAABB, backFace)) potentialCollisions.push_back({-2, 1});
                if (CheckAABBOverlap(cameraAABB, leftFace)) potentialCollisions.push_back({-2, 2});
                if (CheckAABBOverlap(cameraAABB, rightFace)) potentialCollisions.push_back({-2, 3});
                
                for (size_t i = 0; i < creatures.size(); ++i) {
                    AABB creatureAABB = ComputeAABB(creatures[i]->GetPosition(), glm::vec3(0.55f, 0.55f, 0.55f));
                    if (CheckAABBOverlap(cameraAABB, creatureAABB)) {
                        potentialCollisions.push_back({-1, i}); // -1 para identificar a camera
                    }
                }

                // Fase de colisao Narrow Phase
                for (const auto& pair : potentialCollisions) {
                    if (pair.first == -1) { // Colisão entre a camera e uma criatura
                        int creatureIndex = pair.second;
                        if (CheckSphereSphereOverlap(camera_position_c, 0.6,
                                            creatures[creatureIndex]->GetPosition(), 0.6)) {
                            glm::vec4 direction = camera_position_c - creatures[creatureIndex]->GetPosition();
                            float magnitude = glm::length(direction);
                            if (magnitude > 1e-5f) {
                                direction = glm::normalize(direction);
                                camera_position_c += direction * speed * delta_t * 0.05f;
                            }
                            camera_position_c += direction * speed * delta_t * 0.05f;
                        }
                    } else if(pair.first == -2) { // Colisão camera com as faces do cubo
                        if (pair.second == 0) {
                            camera_position_c -= frontFaceDirection * speed * delta_t * 0.05f;
                        } else if (pair.second == 1) {
                            camera_position_c -= backFaceDirection * speed * delta_t * 0.05f;
                        } else if (pair.second == 2) {
                            camera_position_c -= leftFaceDirection * speed * delta_t * 0.05f;
                        } else if (pair.second == 3) {
                            camera_position_c -= rightFaceDirection * speed * delta_t * 0.05f;
                        }
                    }
                }

                for (const auto& creature : creatures) {
                    glm::vec4 position = creature->GetPosition();
                    float rotation_angle = creature->GetRotationAngle();

                    if (g_RightMouseButtonPressed) {
                        ma_sound_start(&suction_sound);
                        if (inWeaponRange(weapon_position, weapon_direction, position, 7.0f, 30.0f)) {
                            if (!creature->captured) {  // Inicia a captura se ainda não estiver capturada
                                creature->captured = true;
                                captureTime = 0.0f;  // Resetando o tempo de captura
                            }

                            captureTime += delta_t / 2.0f; // Ajuste a taxa de incremento de tempo
                            captureTime = glm::clamp(captureTime, 0.0f, 1.0f); // Normaliza entre 0 e 1

                            glm::vec3 start = glm::vec3(position);
                            glm::vec3 end = glm::vec3(weapon_position);
                            glm::vec3 newPosition = bezierSpiralPosition(start, end, captureTime, 10, GROUND_LEVEL);
                            position = glm::vec4(newPosition, 1.0f);

                            if (captureTime >= 1.0f) {
                                captureTime = 0.0f;  // Resetando o tempo de captura
                                position = glm::vec4(end, 1.0f); // Finaliza no centro da arma
                            }

                            lastPosition = position;  // Atualiza a posição final durante o movimento
                        } else {
                            if (creature->captured) {
                                creature->captured = false; // Finaliza a captura
                                creature->setPosition(lastPosition);  // Define a posição final quando o botão é solto
                            } 
                        }
                    } else {
                        // Se o botão do mouse foi solto e a criatura estava capturada
                        if (creature->captured) {
                            creature->captured = false; // Finaliza a captura
                            creature->setPosition(lastPosition);  // Define a posição final quando o botão é solto
                            position = lastPosition;
                        }
                    }      
        
                    if (creature->GetType() == 0) { // Anemo
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Scale(1.0f, 1.0f, 1.0f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("anemo1");
                        DrawVirtualObject("anemo2");
                        DrawVirtualObject("anemo3");
                    } else if (creature->GetType() == 1) { // Cryo
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Rotate_X(3*3.141592f/2.0f) 
                                    * Matrix_Scale(1.0f, 1.0f, 1.0f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("cryo1");
                        DrawVirtualObject("cryo2");
                    } else if (creature->GetType() == 2) { // Dendro
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Rotate_X(3*3.141592f/2.0f)
                                    * Matrix_Scale(1.0f, 1.0f, 1.0f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("dendro1");
                        DrawVirtualObject("dendro2");
                        DrawVirtualObject("dendro3");
                        DrawVirtualObject("dendro4");
                        DrawVirtualObject("dendro5");
                        DrawVirtualObject("dendro6");
                        DrawVirtualObject("dendro7");
                        DrawVirtualObject("dendro8");
                        DrawVirtualObject("dendro9");
                        DrawVirtualObject("dendro10");
                        DrawVirtualObject("dendro11");
                        DrawVirtualObject("dendro12");
                        DrawVirtualObject("dendro13");
                        DrawVirtualObject("dendro14");
                        DrawVirtualObject("dendro15");
                        DrawVirtualObject("dendro16");
                    } else if (creature->GetType() == 3) { // Plasma
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Scale(0.01f, 0.01f, 0.01f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("plasma1");
                        DrawVirtualObject("plasma2");
                        DrawVirtualObject("plasma3");
                    } else if (creature->GetType() == 4) { // Fire
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Scale(0.01f, 0.01f, 0.01f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f); 
                        DrawVirtualObject("fire1");
                        DrawVirtualObject("fire2");
                    } else if (creature->GetType() == 5) { // Geo
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Scale(0.01f, 0.01f, 0.01f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("geo1");
                    } else if (creature->GetType() == 6) { //Electro
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z)
                                    * Matrix_Rotate_Y(rotation_angle)
                                    * Matrix_Scale(0.01f, 0.01f, 0.01f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f); 
                        DrawVirtualObject("electro1");
                        DrawVirtualObject("electro2");
                        DrawVirtualObject("electro3");
                    } else if (creature->GetType() == 7) { // Water
                        model = Matrix_Translate(position.x, position.y - 1.5f, position.z) 
                                    * Matrix_Rotate_Y(rotation_angle) 
                                    * Matrix_Scale(0.01f, 0.01f, 0.01f);
                        glUniformMatrix4fv(g_model_uniform, 1, GL_FALSE, glm::value_ptr(model));
                        glUniform1i(g_object_id_uniform, creature->GetType() + CREATURE);
                        glUniform2f(tilingLocation, 1.0f, 1.0f);
                        DrawVirtualObject("water1");
                        DrawVirtualObject("water2");
                    }
                }

                // Desenhamos o modelo do cubo
                model = Matrix_Translate(0.0f,0.0f,0.0f)
                        * Matrix_Scale(map_width, map_height, map_length);
                glUniformMatrix4fv(g_model_uniform, 1 , GL_FALSE , glm::value_ptr(model));
                glUniform1i(g_object_id_uniform, CUBE);
                glUniform2f(tilingLocation, 1.0f, 1.0f);
                DrawVirtualObject("cube");

                // Imprimimos na tela informação sobre o número de quadros renderizados
                // por segundo (frames per second).
                TextRendering_ShowFramesPerSecond(window);

                //Lore audio
                if(((!listened_to_lore[0] && lore_progress_level >= 0) || (g_OnekeyPressed && (mode == CHEAT_MODE || listened_to_lore[0])))
                && (!ma_sound_is_playing(&lore2_sound) && !ma_sound_is_playing(&lore3_sound) && !ma_sound_is_playing(&ending_sound)))
                {
                    ma_sound_set_volume(&lore1_sound, 1.5f);
                    ma_sound_start(&lore1_sound);
                    listened_to_lore[0] = true;
                }
                else if(((!listened_to_lore[1] && lore_progress_level >= 1) || (g_TwokeyPressed && (mode == CHEAT_MODE || listened_to_lore[1])))
                && (!ma_sound_is_playing(&lore1_sound) && !ma_sound_is_playing(&lore3_sound) && !ma_sound_is_playing(&ending_sound)))
                {
                    ma_sound_set_volume(&lore2_sound, 1.5f);
                    ma_sound_start(&lore2_sound);
                    listened_to_lore[1] = true;
                }
                else if(((!listened_to_lore[2] && lore_progress_level >= 2) || (g_ThreekeyPressed && (mode == CHEAT_MODE || listened_to_lore[2])))
                 && (!ma_sound_is_playing(&lore1_sound) && !ma_sound_is_playing(&lore2_sound) && !ma_sound_is_playing(&ending_sound)))
                {
                    ma_sound_set_volume(&lore3_sound, 1.5f);
                    ma_sound_start(&lore3_sound);
                    listened_to_lore[2] = true;
                }
                else if(((!listened_to_lore[3] && lore_progress_level >= 3) || (g_FourkeyPressed && (mode == CHEAT_MODE || listened_to_lore[3])))
                 && (!ma_sound_is_playing(&lore1_sound) && !ma_sound_is_playing(&lore2_sound) && !ma_sound_is_playing(&lore3_sound)))
                {
                    ma_sound_set_volume(&ending_sound, 1.5f);
                    ma_sound_start(&ending_sound);
                    listened_to_lore[3] = true;
                    current_game_state = WIN;
                    currentDecoder = &decoder_win;
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }

                }
                else if(g_ZerokeyPressed && (listened_to_lore[3] || mode == CHEAT_MODE))
                {
                    current_game_state = WIN;
                    g_ZerokeyPressed = false;
                    currentDecoder = &decoder_win;
                    ma_device_uninit(&device);
                    ma_decoder_seek_to_pcm_frame(currentDecoder, 0);
                    deviceConfig.pUserData = currentDecoder;
                    result = ma_device_init(NULL, &deviceConfig, &device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to reinitialize playback device.\n");
                    }
                    result = ma_device_start(&device);
                    if (result != MA_SUCCESS) 
                    {
                        printf("Failed to start playback device.\n");
                    }
                }
                else if(g_FivekeyPressed)
                {
                    ma_sound_start(&welcome_sound);

                    for (const auto& slime : inventory)
                    {
                        balance[slime]++;
                    }
                    inventory.clear();
                    current_game_state = UPGRADE;
                    g_FivekeyPressed = false;
                }


                // O framebuffer onde OpenGL executa as operações de renderização não
                // é o mesmo que está sendo mostrado para o usuário, caso contrário
                // seria possível ver artefatos conhecidos como "screen tearing". A
                // chamada abaixo faz a troca dos buffers, mostrando para o usuário
                // tudo que foi renderizado pelas funções acima.
                // Veja o link: https://en.wikipedia.org/w/index.php?title=Multiple_buffering&oldid=793452829#Double_buffering_in_computer_graphics

                glfwSwapBuffers(window);

                // Verificamos com o sistema operacional se houve alguma interação do
                // usuário (teclado, mouse, ...). Caso positivo, as funções de callback
                // definidas anteriormente usando glfwSet*Callback() serão chamadas
                // pela biblioteca GLFW.
                glfwPollEvents();
                break;
            }
        }
    }
    // Finalizamos o uso dos recursos do sistema operacional
    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder_game);
    ma_decoder_uninit(&decoder_win);
    ma_decoder_uninit(&decoder_menu);
    ma_sound_uninit(&slime_jump_sound);
    ma_sound_uninit(&step_sound);
    ma_sound_uninit(&jump_sound);
    ma_sound_uninit(&lore1_sound);
    ma_sound_uninit(&lore2_sound);
    ma_sound_uninit(&lore3_sound);
    ma_sound_uninit(&ending_sound);
    ma_engine_uninit(&engine);
    glfwTerminate();
    // Fim do programa
    return 0;
}

// Função que carrega uma imagem para ser utilizada como textura
void LoadTextureImage(const char* filename)
{
    printf("Carregando imagem \"%s\"... ", filename);

    // Primeiro fazemos a leitura da imagem do disco
    stbi_set_flip_vertically_on_load(true);
    int width;
    int height;
    int channels;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 3);

    if ( data == NULL )
    {
        fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }

    printf("OK (%dx%d).\n", width, height);

    // Agora criamos objetos na GPU com OpenGL para armazenar a textura
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);

    // Veja slides 95-96 do documento Aula_20_Mapeamento_de_Texturas.pdf
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(sampler_id, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Parâmetros de amostragem da textura.
    glSamplerParameteri(sampler_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(sampler_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Agora enviamos a imagem lida do disco para a GPU
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindSampler(textureunit, sampler_id);

    stbi_image_free(data);

    g_NumLoadedTextures += 1;
}

void LoadCubemap(std::vector<std::string> faces)
{
    GLuint texture_id;
    GLuint sampler_id;
    glGenTextures(1, &texture_id);
    glGenSamplers(1, &sampler_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            fprintf(stderr, "ERROR: Cannot open image file \"%s\".\n", faces[i].c_str());
            stbi_image_free(data);
        }
    }

    GLuint textureunit = g_NumLoadedTextures;
    glActiveTexture(GL_TEXTURE0 + textureunit);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture_id);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindSampler(textureunit, sampler_id);
    g_NumLoadedTextures += 1;
}

// Função que desenha um objeto armazenado em g_VirtualScene. Veja definição
// dos objetos na função BuildTrianglesAndAddToVirtualScene().
void DrawVirtualObject(const char* object_name)
{
    // "Ligamos" o VAO. Informamos que queremos utilizar os atributos de
    // vértices apontados pelo VAO criado pela função BuildTrianglesAndAddToVirtualScene(). Veja
    // comentários detalhados dentro da definição de BuildTrianglesAndAddToVirtualScene().
    glBindVertexArray(g_VirtualScene[object_name].vertex_array_object_id);

    // Setamos as variáveis "bbox_min" e "bbox_max" do fragment shader
    // com os parâmetros da axis-aligned bounding box (AABB) do modelo.
    glm::vec3 bbox_min = g_VirtualScene[object_name].bbox_min;
    glm::vec3 bbox_max = g_VirtualScene[object_name].bbox_max;
    glUniform4f(g_bbox_min_uniform, bbox_min.x, bbox_min.y, bbox_min.z, 1.0f);
    glUniform4f(g_bbox_max_uniform, bbox_max.x, bbox_max.y, bbox_max.z, 1.0f);

    // Pedimos para a GPU rasterizar os vértices dos eixos XYZ
    // apontados pelo VAO como linhas. Veja a definição de
    // g_VirtualScene[""] dentro da função BuildTrianglesAndAddToVirtualScene(), e veja
    // a documentação da função glDrawElements() em
    // http://docs.gl/gl3/glDrawElements.
    glDrawElements(
        g_VirtualScene[object_name].rendering_mode,
        g_VirtualScene[object_name].num_indices,
        GL_UNSIGNED_INT,
        (void*)(g_VirtualScene[object_name].first_index * sizeof(GLuint))
    );

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Função que carrega os shaders de vértices e de fragmentos que serão
// utilizados para renderização. Veja slides 180-200 do documento Aula_03_Rendering_Pipeline_Grafico.pdf.
//
void LoadShadersFromFiles()
{
    // Note que o caminho para os arquivos "shader_vertex.glsl" e
    // "shader_fragment.glsl" estão fixados, sendo que assumimos a existência
    // da seguinte estrutura no sistema de arquivos:
    //
    //    + FCG_Lab_01/
    //    |
    //    +--+ bin/
    //    |  |
    //    |  +--+ Release/  (ou Debug/ ou Linux/)
    //    |     |
    //    |     o-- main.exe
    //    |
    //    +--+ src/
    //       |
    //       o-- shader_vertex.glsl
    //       |
    //       o-- shader_fragment.glsl
    //
    GLuint vertex_shader_id = LoadShader_Vertex("../../src/shader_vertex.glsl");
    GLuint fragment_shader_id = LoadShader_Fragment("../../src/shader_fragment.glsl");

    // Deletamos o programa de GPU anterior, caso ele exista.
    if ( g_GpuProgramID != 0 )
        glDeleteProgram(g_GpuProgramID);

    // Criamos um programa de GPU utilizando os shaders carregados acima.
    g_GpuProgramID = CreateGpuProgram(vertex_shader_id, fragment_shader_id);

    // Buscamos o endereço das variáveis definidas dentro do Vertex Shader.
    // Utilizaremos estas variáveis para enviar dados para a placa de vídeo
    // (GPU)! Veja arquivo "shader_vertex.glsl" e "shader_fragment.glsl".
    g_model_uniform      = glGetUniformLocation(g_GpuProgramID, "model"); // Variável da matriz "model"
    g_view_uniform       = glGetUniformLocation(g_GpuProgramID, "view"); // Variável da matriz "view" em shader_vertex.glsl
    g_projection_uniform = glGetUniformLocation(g_GpuProgramID, "projection"); // Variável da matriz "projection" em shader_vertex.glsl
    g_object_id_uniform  = glGetUniformLocation(g_GpuProgramID, "object_id"); // Variável "object_id" em shader_fragment.glsl
    g_bbox_min_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_min");
    g_bbox_max_uniform   = glGetUniformLocation(g_GpuProgramID, "bbox_max");
    tilingLocation       = glGetUniformLocation(g_GpuProgramID, "tiling_factor");

    // Variáveis em "shader_fragment.glsl" para acesso das imagens de textura
    glUseProgram(g_GpuProgramID);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage0"), 0);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage1"), 1);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage2"), 2);

    glUniform1i(glGetUniformLocation(g_GpuProgramID, "anemo"), 3);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "cryo"), 4);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "dendro"), 5);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "plasma"), 6);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "fire"), 7);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "geo"), 8);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "electro"), 9);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "water"), 10);
    // skybox
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "skybox"), 11);
    
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage12"), 12);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage13"), 13);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage14"), 14);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage15"), 15);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage16"), 16);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage17"), 17);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage18"), 18);

    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage19"), 19);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage20"), 20);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage21"), 21);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage22"), 22);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage23"), 23);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage24"), 24);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage25"), 25);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage26"), 26);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage27"), 27);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "TextureImage28"), 28);

    glUniform1i(glGetUniformLocation(g_GpuProgramID, "heaven_skybox"), 29);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "god"), 30);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "menu"), 31);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "menu"), 32);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "controls"), 33);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "upgrades"), 34);
    glUniform1i(glGetUniformLocation(g_GpuProgramID, "store_monster"), 35);
    glUseProgram(0);
}

// Função que pega a matriz M e guarda a mesma no topo da pilha
void PushMatrix(glm::mat4 M)
{
    g_MatrixStack.push(M);
}

// Função que remove a matriz atualmente no topo da pilha e armazena a mesma na variável M
void PopMatrix(glm::mat4& M)
{
    if ( g_MatrixStack.empty() )
    {
        M = Matrix_Identity();
    }
    else
    {
        M = g_MatrixStack.top();
        g_MatrixStack.pop();
    }
}

// Função que computa as normais de um ObjModel, caso elas não tenham sido
// especificadas dentro do arquivo ".obj"
void ComputeNormals(ObjModel* model)
{
    if ( !model->attrib.normals.empty() )
        return;

    // Primeiro computamos as normais para todos os TRIÂNGULOS.
    // Segundo, computamos as normais dos VÉRTICES através do método proposto
    // por Gouraud, onde a normal de cada vértice vai ser a média das normais de
    // todas as faces que compartilham este vértice.

    size_t num_vertices = model->attrib.vertices.size() / 3;

    std::vector<int> num_triangles_per_vertex(num_vertices, 0);
    std::vector<glm::vec4> vertex_normals(num_vertices, glm::vec4(0.0f,0.0f,0.0f,0.0f));

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            glm::vec4  vertices[3];
            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                vertices[vertex] = glm::vec4(vx,vy,vz,1.0);
            }

            const glm::vec4  a = vertices[0];
            const glm::vec4  b = vertices[1];
            const glm::vec4  c = vertices[2];

            const glm::vec4  n = crossproduct(b-a,c-a);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];
                num_triangles_per_vertex[idx.vertex_index] += 1;
                vertex_normals[idx.vertex_index] += n;
                model->shapes[shape].mesh.indices[3*triangle + vertex].normal_index = idx.vertex_index;
            }
        }
    }

    model->attrib.normals.resize( 3*num_vertices );

    for (size_t i = 0; i < vertex_normals.size(); ++i)
    {
        glm::vec4 n = vertex_normals[i] / (float)num_triangles_per_vertex[i];
        n /= norm(n);
        model->attrib.normals[3*i + 0] = n.x;
        model->attrib.normals[3*i + 1] = n.y;
        model->attrib.normals[3*i + 2] = n.z;
    }
}

// Constrói triângulos para futura renderização a partir de um ObjModel.
void BuildTrianglesAndAddToVirtualScene(ObjModel* model)
{
    GLuint vertex_array_object_id;
    glGenVertexArrays(1, &vertex_array_object_id);
    glBindVertexArray(vertex_array_object_id);

    std::vector<GLuint> indices;
    std::vector<float>  model_coefficients;
    std::vector<float>  normal_coefficients;
    std::vector<float>  texture_coefficients;

    for (size_t shape = 0; shape < model->shapes.size(); ++shape)
    {
        size_t first_index = indices.size();
        size_t num_triangles = model->shapes[shape].mesh.num_face_vertices.size();

        const float minval = std::numeric_limits<float>::min();
        const float maxval = std::numeric_limits<float>::max();

        glm::vec3 bbox_min = glm::vec3(maxval,maxval,maxval);
        glm::vec3 bbox_max = glm::vec3(minval,minval,minval);

        for (size_t triangle = 0; triangle < num_triangles; ++triangle)
        {
            assert(model->shapes[shape].mesh.num_face_vertices[triangle] == 3);

            for (size_t vertex = 0; vertex < 3; ++vertex)
            {
                tinyobj::index_t idx = model->shapes[shape].mesh.indices[3*triangle + vertex];

                indices.push_back(first_index + 3*triangle + vertex);

                const float vx = model->attrib.vertices[3*idx.vertex_index + 0];
                const float vy = model->attrib.vertices[3*idx.vertex_index + 1];
                const float vz = model->attrib.vertices[3*idx.vertex_index + 2];
                //printf("tri %d vert %d = (%.2f, %.2f, %.2f)\n", (int)triangle, (int)vertex, vx, vy, vz);
                model_coefficients.push_back( vx ); // X
                model_coefficients.push_back( vy ); // Y
                model_coefficients.push_back( vz ); // Z
                model_coefficients.push_back( 1.0f ); // W

                bbox_min.x = std::min(bbox_min.x, vx);
                bbox_min.y = std::min(bbox_min.y, vy);
                bbox_min.z = std::min(bbox_min.z, vz);
                bbox_max.x = std::max(bbox_max.x, vx);
                bbox_max.y = std::max(bbox_max.y, vy);
                bbox_max.z = std::max(bbox_max.z, vz);

                // Inspecionando o código da tinyobjloader, o aluno Bernardo
                // Sulzbach (2017/1) apontou que a maneira correta de testar se
                // existem normais e coordenadas de textura no ObjModel é
                // comparando se o índice retornado é -1. Fazemos isso abaixo.

                if ( idx.normal_index != -1 )
                {
                    const float nx = model->attrib.normals[3*idx.normal_index + 0];
                    const float ny = model->attrib.normals[3*idx.normal_index + 1];
                    const float nz = model->attrib.normals[3*idx.normal_index + 2];
                    normal_coefficients.push_back( nx ); // X
                    normal_coefficients.push_back( ny ); // Y
                    normal_coefficients.push_back( nz ); // Z
                    normal_coefficients.push_back( 0.0f ); // W
                }

                if ( idx.texcoord_index != -1 )
                {
                    const float u = model->attrib.texcoords[2*idx.texcoord_index + 0];
                    const float v = model->attrib.texcoords[2*idx.texcoord_index + 1];
                    texture_coefficients.push_back( u );
                    texture_coefficients.push_back( v );
                }
            }
        }

        size_t last_index = indices.size() - 1;

        SceneObject theobject;
        theobject.name           = model->shapes[shape].name;
        theobject.first_index    = first_index; // Primeiro índice
        theobject.num_indices    = last_index - first_index + 1; // Número de indices
        theobject.rendering_mode = GL_TRIANGLES;       // Índices correspondem ao tipo de rasterização GL_TRIANGLES.
        theobject.vertex_array_object_id = vertex_array_object_id;

        theobject.bbox_min = bbox_min;
        theobject.bbox_max = bbox_max;

        g_VirtualScene[model->shapes[shape].name] = theobject;
    }

    GLuint VBO_model_coefficients_id;
    glGenBuffers(1, &VBO_model_coefficients_id);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_model_coefficients_id);
    glBufferData(GL_ARRAY_BUFFER, model_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, model_coefficients.size() * sizeof(float), model_coefficients.data());
    GLuint location = 0; // "(location = 0)" em "shader_vertex.glsl"
    GLint  number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
    glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(location);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if ( !normal_coefficients.empty() )
    {
        GLuint VBO_normal_coefficients_id;
        glGenBuffers(1, &VBO_normal_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_normal_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, normal_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, normal_coefficients.size() * sizeof(float), normal_coefficients.data());
        location = 1; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 4; // vec4 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if ( !texture_coefficients.empty() )
    {
        GLuint VBO_texture_coefficients_id;
        glGenBuffers(1, &VBO_texture_coefficients_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_texture_coefficients_id);
        glBufferData(GL_ARRAY_BUFFER, texture_coefficients.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, texture_coefficients.size() * sizeof(float), texture_coefficients.data());
        location = 2; // "(location = 1)" em "shader_vertex.glsl"
        number_of_dimensions = 2; // vec2 em "shader_vertex.glsl"
        glVertexAttribPointer(location, number_of_dimensions, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(location);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    GLuint indices_id;
    glGenBuffers(1, &indices_id);

    // "Ligamos" o buffer. Note que o tipo agora é GL_ELEMENT_ARRAY_BUFFER.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices.size() * sizeof(GLuint), indices.data());
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // XXX Errado!
    //

    // "Desligamos" o VAO, evitando assim que operações posteriores venham a
    // alterar o mesmo. Isso evita bugs.
    glBindVertexArray(0);
}

// Carrega um Vertex Shader de um arquivo GLSL. Veja definição de LoadShader() abaixo.
GLuint LoadShader_Vertex(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos vértices.
    GLuint vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, vertex_shader_id);

    // Retorna o ID gerado acima
    return vertex_shader_id;
}

// Carrega um Fragment Shader de um arquivo GLSL . Veja definição de LoadShader() abaixo.
GLuint LoadShader_Fragment(const char* filename)
{
    // Criamos um identificador (ID) para este shader, informando que o mesmo
    // será aplicado nos fragmentos.
    GLuint fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

    // Carregamos e compilamos o shader
    LoadShader(filename, fragment_shader_id);

    // Retorna o ID gerado acima
    return fragment_shader_id;
}

// Função auxilar, utilizada pelas duas funções acima. Carrega código de GPU de
// um arquivo GLSL e faz sua compilação.
void LoadShader(const char* filename, GLuint shader_id)
{
    // Lemos o arquivo de texto indicado pela variável "filename"
    // e colocamos seu conteúdo em memória, apontado pela variável
    // "shader_string".
    std::ifstream file;
    try {
        file.exceptions(std::ifstream::failbit);
        file.open(filename);
    } catch ( std::exception& e ) {
        fprintf(stderr, "ERROR: Cannot open file \"%s\".\n", filename);
        std::exit(EXIT_FAILURE);
    }
    std::stringstream shader;
    shader << file.rdbuf();
    std::string str = shader.str();
    const GLchar* shader_string = str.c_str();
    const GLint   shader_string_length = static_cast<GLint>( str.length() );

    // Define o código do shader GLSL, contido na string "shader_string"
    glShaderSource(shader_id, 1, &shader_string, &shader_string_length);

    // Compila o código do shader GLSL (em tempo de execução)
    glCompileShader(shader_id);

    // Verificamos se ocorreu algum erro ou "warning" durante a compilação
    GLint compiled_ok;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled_ok);

    GLint log_length = 0;
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_length);

    // Alocamos memória para guardar o log de compilação.
    // A chamada "new" em C++ é equivalente ao "malloc()" do C.
    GLchar* log = new GLchar[log_length];
    glGetShaderInfoLog(shader_id, log_length, &log_length, log);

    // Imprime no terminal qualquer erro ou "warning" de compilação
    if ( log_length != 0 )
    {
        std::string  output;

        if ( !compiled_ok )
        {
            output += "ERROR: OpenGL compilation of \"";
            output += filename;
            output += "\" failed.\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }
        else
        {
            output += "WARNING: OpenGL compilation of \"";
            output += filename;
            output += "\".\n";
            output += "== Start of compilation log\n";
            output += log;
            output += "== End of compilation log\n";
        }

        fprintf(stderr, "%s", output.c_str());
    }

    // A chamada "delete" em C++ é equivalente ao "free()" do C
    delete [] log;
}

// Esta função cria um programa de GPU, o qual contém obrigatoriamente um
// Vertex Shader e um Fragment Shader.
GLuint CreateGpuProgram(GLuint vertex_shader_id, GLuint fragment_shader_id)
{
    // Criamos um identificador (ID) para este programa de GPU
    GLuint program_id = glCreateProgram();

    // Definição dos dois shaders GLSL que devem ser executados pelo programa
    glAttachShader(program_id, vertex_shader_id);
    glAttachShader(program_id, fragment_shader_id);

    // Linkagem dos shaders acima ao programa
    glLinkProgram(program_id);

    // Verificamos se ocorreu algum erro durante a linkagem
    GLint linked_ok = GL_FALSE;
    glGetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);

    // Imprime no terminal qualquer erro de linkagem
    if ( linked_ok == GL_FALSE )
    {
        GLint log_length = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &log_length);

        // Alocamos memória para guardar o log de compilação.
        // A chamada "new" em C++ é equivalente ao "malloc()" do C.
        GLchar* log = new GLchar[log_length];

        glGetProgramInfoLog(program_id, log_length, &log_length, log);

        std::string output;

        output += "ERROR: OpenGL linking of program failed.\n";
        output += "== Start of link log\n";
        output += log;
        output += "\n== End of link log\n";

        // A chamada "delete" em C++ é equivalente ao "free()" do C
        delete [] log;

        fprintf(stderr, "%s", output.c_str());
    }

    // Os "Shader Objects" podem ser marcados para deleção após serem linkados 
    glDeleteShader(vertex_shader_id);
    glDeleteShader(fragment_shader_id);

    // Retornamos o ID gerado acima
    return program_id;
}

// Definição da função que será chamada sempre que a janela do sistema
// operacional for redimensionada, por consequência alterando o tamanho do
// "framebuffer" (região de memória onde são armazenados os pixels da imagem).
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // Indicamos que queremos renderizar em toda região do framebuffer. A
    // função "glViewport" define o mapeamento das "normalized device
    // coordinates" (NDC) para "pixel coordinates".  Essa é a operação de
    // "Screen Mapping" ou "Viewport Mapping" vista em aula ({+ViewportMapping2+}).
    glViewport(0, 0, width, height);

    // Atualizamos também a razão que define a proporção da janela (largura /
    // altura), a qual será utilizada na definição das matrizes de projeção,
    // tal que não ocorra distorções durante o processo de "Screen Mapping"
    // acima, quando NDC é mapeado para coordenadas de pixels. Veja slides 205-215 do documento Aula_09_Projecoes.pdf.
    //
    // O cast para float é necessário pois números inteiros são arredondados ao
    // serem divididos!
    g_ScreenRatio = (float)width / height;
}

// Variáveis globais que armazenam a última posição do cursor do mouse, para
// que possamos calcular quanto que o mouse se movimentou entre dois instantes
// de tempo. Utilizadas no callback CursorPosCallback() abaixo.
double g_LastCursorPosX, g_LastCursorPosY;

// Função callback chamada sempre que o usuário aperta algum dos botões do mouse
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_LeftMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_LeftMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_LeftMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_RightMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        g_RightMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_RightMouseButtonPressed = false;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        // Se o usuário pressionou o botão esquerdo do mouse, guardamos a
        // posição atual do cursor nas variáveis g_LastCursorPosX e
        // g_LastCursorPosY.  Também, setamos a variável
        // g_MiddleMouseButtonPressed como true, para saber que o usuário está
        // com o botão esquerdo pressionado.
        glfwGetCursorPos(window, &g_LastCursorPosX, &g_LastCursorPosY);
        g_MiddleMouseButtonPressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
    {
        // Quando o usuário soltar o botão esquerdo do mouse, atualizamos a
        // variável abaixo para false.
        g_MiddleMouseButtonPressed = false;
    }
}

// Função callback chamada sempre que o usuário movimentar o cursor do mouse em
// cima da janela OpenGL.
void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    // Deslocamento do cursor do mouse em x e y de coordenadas de tela!
    float dx = xpos - g_LastCursorPosX;
    float dy = ypos - g_LastCursorPosY;
    
    // Atualizamos parâmetros da câmera com os deslocamentos
    g_CameraTheta -= 0.01f*dx;
    g_CameraPhi   += 0.01f*dy;
    
    // Em coordenadas esféricas, o ângulo phi deve ficar entre -pi/2 e +pi/2.
    float phimax = 3.141592f/2;
    float phimin = -phimax;
    
    if (g_CameraPhi > phimax)
        g_CameraPhi = phimax;
    
    if (g_CameraPhi < phimin)
        g_CameraPhi = phimin;
    
    // Atualizamos as variáveis globais para armazenar a posição atual do
    // cursor como sendo a última posição conhecida do cursor.
    g_LastCursorPosX = xpos;
    g_LastCursorPosY = ypos;
}

// Função callback chamada sempre que o usuário movimenta a "rodinha" do mouse.
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Atualizamos a distância da câmera para a origem utilizando a
    // movimentação da "rodinha", simulando um ZOOM.
    g_CameraDistance -= 0.1f*yoffset;

    // Uma câmera look-at nunca pode estar exatamente "em cima" do ponto para
    // onde ela está olhando, pois isto gera problemas de divisão por zero na
    // definição do sistema de coordenadas da câmera. Isto é, a variável abaixo
    // nunca pode ser zero. Versões anteriores deste código possuíam este bug,
    // o qual foi detectado pelo aluno Vinicius Fraga (2017/2).
    const float verysmallnumber = std::numeric_limits<float>::epsilon();
    if (g_CameraDistance < verysmallnumber)
        g_CameraDistance = verysmallnumber;
}

// Definição da função que será chamada sempre que o usuário pressionar alguma
// tecla do teclado. Veja http://www.glfw.org/docs/latest/input_guide.html#input_key
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mod)
{
    if (key == GLFW_KEY_0 && action == GLFW_PRESS) g_ZerokeyPressed = true;
    else if (key == GLFW_KEY_0 && action == GLFW_RELEASE) g_ZerokeyPressed = false;

    if (key == GLFW_KEY_1 && action == GLFW_PRESS) g_OnekeyPressed = true;
    else if (key == GLFW_KEY_1 && action == GLFW_RELEASE) g_OnekeyPressed = false;

    if (key == GLFW_KEY_2 && action == GLFW_PRESS) g_TwokeyPressed = true;
    else if (key == GLFW_KEY_2 && action == GLFW_RELEASE) g_TwokeyPressed = false;

    if (key == GLFW_KEY_3 && action == GLFW_PRESS) g_ThreekeyPressed = true;
    else if (key == GLFW_KEY_3 && action == GLFW_RELEASE) g_ThreekeyPressed = false;

    if (key == GLFW_KEY_4 && action == GLFW_PRESS) g_FourkeyPressed = true;
    else if (key == GLFW_KEY_4 && action == GLFW_RELEASE) g_FourkeyPressed = false;

    if (key == GLFW_KEY_5 && action == GLFW_PRESS) g_FivekeyPressed = true;
    else if (key == GLFW_KEY_5 && action == GLFW_RELEASE) g_FivekeyPressed = false;

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) g_EsckeyPressed = true;
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) g_EsckeyPressed = false;

    // Altera o estado das teclas de movimentacao
    if (key == GLFW_KEY_D && action == GLFW_PRESS) g_DkeyPressed = true;
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE) g_DkeyPressed = false;

    if (key == GLFW_KEY_A && action == GLFW_PRESS) g_AkeyPressed = true;
    else if (key == GLFW_KEY_A && action == GLFW_RELEASE) g_AkeyPressed = false;

    if (key == GLFW_KEY_W && action == GLFW_PRESS) g_WkeyPressed = true;
    else if (key == GLFW_KEY_W && action == GLFW_RELEASE) g_WkeyPressed = false;

    if (key == GLFW_KEY_S && action == GLFW_PRESS) g_SkeyPressed = true;
    else if (key == GLFW_KEY_S && action == GLFW_RELEASE) g_SkeyPressed = false;

    // O código abaixo implementa a seguinte lógica:
    //   Se apertar tecla X       então g_AngleX += delta;
    //   Se apertar tecla shift+X então g_AngleX -= delta;
    //   Se apertar tecla Y       então g_AngleY += delta;
    //   Se apertar tecla shift+Y então g_AngleY -= delta;
    //   Se apertar tecla Z       então g_AngleZ += delta;
    //   Se apertar tecla shift+Z então g_AngleZ -= delta;

    if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_PRESS)
        g_IsSprinting = true;
    else if (key == GLFW_KEY_LEFT_SHIFT && action == GLFW_RELEASE)
        g_IsSprinting = false;

        // Se o usuário apertar a tecla espaço, resetamos os ângulos de Euler para zero.
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !g_IsJumping)
    {
        g_IsJumping = true;
        g_CameraVerticalVelocity = JUMP_VELOCITY;
        g_Player_Started_Jumping = true;
    }

    // Se o usuário apertar a tecla R, recarregamos os shaders dos arquivos "shader_fragment.glsl" e "shader_vertex.glsl".
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        LoadShadersFromFiles();
        fprintf(stdout,"Shaders recarregados!\n");
        fflush(stdout);
    }
}

// Definimos o callback para impressão de erros da GLFW no terminal
void ErrorCallback(int error, const char* description)
{
    fprintf(stderr, "ERROR: GLFW: %s\n", description);
}

// Escrevemos na tela o número de quadros renderizados por segundo (frames per
// second).
void TextRendering_ShowFramesPerSecond(GLFWwindow* window)
{
    if ( !g_ShowInfoText )
        return;

    // Variáveis estáticas (static) mantém seus valores entre chamadas
    // subsequentes da função!
    static float old_seconds = (float)glfwGetTime();
    static int   ellapsed_frames = 0;
    static char  buffer[20] = "?? fps";
    static int   numchars = 7;

    ellapsed_frames += 1;

    // Recuperamos o número de segundos que passou desde a execução do programa
    float seconds = (float)glfwGetTime();

    // Número de segundos desde o último cálculo do fps
    float ellapsed_seconds = seconds - old_seconds;

    if ( ellapsed_seconds > 1.0f )
    {
        numchars = snprintf(buffer, 20, "%.2f fps", ellapsed_frames / ellapsed_seconds);
    
        old_seconds = seconds;
        ellapsed_frames = 0;
    }

    float lineheight = TextRendering_LineHeight(window);
    float charwidth = TextRendering_CharWidth(window);

    TextRendering_PrintString(window, buffer, 1.0f-(numchars + 1)*charwidth, 1.0f-lineheight, 1.0f);
}

// Função para debugging: imprime no terminal todas informações de um modelo
// geométrico carregado de um arquivo ".obj".
// Veja: https://github.com/syoyo/tinyobjloader/blob/22883def8db9ef1f3ffb9b404318e7dd25fdbb51/loader_example.cc#L98
void PrintObjModelInfo(ObjModel* model)
{
  const tinyobj::attrib_t                & attrib    = model->attrib;
  const std::vector<tinyobj::shape_t>    & shapes    = model->shapes;
  const std::vector<tinyobj::material_t> & materials = model->materials;

  printf("# of vertices  : %d\n", (int)(attrib.vertices.size() / 3));
  printf("# of normals   : %d\n", (int)(attrib.normals.size() / 3));
  printf("# of texcoords : %d\n", (int)(attrib.texcoords.size() / 2));
  printf("# of shapes    : %d\n", (int)shapes.size());
  printf("# of materials : %d\n", (int)materials.size());

  for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
    printf("  v[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.vertices[3 * v + 0]),
           static_cast<const double>(attrib.vertices[3 * v + 1]),
           static_cast<const double>(attrib.vertices[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.normals.size() / 3; v++) {
    printf("  n[%ld] = (%f, %f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.normals[3 * v + 0]),
           static_cast<const double>(attrib.normals[3 * v + 1]),
           static_cast<const double>(attrib.normals[3 * v + 2]));
  }

  for (size_t v = 0; v < attrib.texcoords.size() / 2; v++) {
    printf("  uv[%ld] = (%f, %f)\n", static_cast<long>(v),
           static_cast<const double>(attrib.texcoords[2 * v + 0]),
           static_cast<const double>(attrib.texcoords[2 * v + 1]));
  }

  // For each shape
  for (size_t i = 0; i < shapes.size(); i++) {
    printf("shape[%ld].name = %s\n", static_cast<long>(i),
           shapes[i].name.c_str());
    printf("Size of shape[%ld].indices: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.indices.size()));

    size_t index_offset = 0;

    assert(shapes[i].mesh.num_face_vertices.size() ==
           shapes[i].mesh.material_ids.size());

    printf("shape[%ld].num_faces: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.num_face_vertices.size()));

    // For each face
    for (size_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
      size_t fnum = shapes[i].mesh.num_face_vertices[f];

      printf("  face[%ld].fnum = %ld\n", static_cast<long>(f),
             static_cast<unsigned long>(fnum));

      // For each vertex in the face
      for (size_t v = 0; v < fnum; v++) {
        tinyobj::index_t idx = shapes[i].mesh.indices[index_offset + v];
        printf("    face[%ld].v[%ld].idx = %d/%d/%d\n", static_cast<long>(f),
               static_cast<long>(v), idx.vertex_index, idx.normal_index,
               idx.texcoord_index);
      }

      printf("  face[%ld].material_id = %d\n", static_cast<long>(f),
             shapes[i].mesh.material_ids[f]);

      index_offset += fnum;
    }

    printf("shape[%ld].num_tags: %lu\n", static_cast<long>(i),
           static_cast<unsigned long>(shapes[i].mesh.tags.size()));
    for (size_t t = 0; t < shapes[i].mesh.tags.size(); t++) {
      printf("  tag[%ld] = %s ", static_cast<long>(t),
             shapes[i].mesh.tags[t].name.c_str());
      printf(" ints: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].intValues.size(); ++j) {
        printf("%ld", static_cast<long>(shapes[i].mesh.tags[t].intValues[j]));
        if (j < (shapes[i].mesh.tags[t].intValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" floats: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].floatValues.size(); ++j) {
        printf("%f", static_cast<const double>(
                         shapes[i].mesh.tags[t].floatValues[j]));
        if (j < (shapes[i].mesh.tags[t].floatValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");

      printf(" strings: [");
      for (size_t j = 0; j < shapes[i].mesh.tags[t].stringValues.size(); ++j) {
        printf("%s", shapes[i].mesh.tags[t].stringValues[j].c_str());
        if (j < (shapes[i].mesh.tags[t].stringValues.size() - 1)) {
          printf(", ");
        }
      }
      printf("]");
      printf("\n");
    }
  }

  for (size_t i = 0; i < materials.size(); i++) {
    printf("material[%ld].name = %s\n", static_cast<long>(i),
           materials[i].name.c_str());
    printf("  material.Ka = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].ambient[0]),
           static_cast<const double>(materials[i].ambient[1]),
           static_cast<const double>(materials[i].ambient[2]));
    printf("  material.Kd = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].diffuse[0]),
           static_cast<const double>(materials[i].diffuse[1]),
           static_cast<const double>(materials[i].diffuse[2]));
    printf("  material.Ks = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].specular[0]),
           static_cast<const double>(materials[i].specular[1]),
           static_cast<const double>(materials[i].specular[2]));
    printf("  material.Tr = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].transmittance[0]),
           static_cast<const double>(materials[i].transmittance[1]),
           static_cast<const double>(materials[i].transmittance[2]));
    printf("  material.Ke = (%f, %f ,%f)\n",
           static_cast<const double>(materials[i].emission[0]),
           static_cast<const double>(materials[i].emission[1]),
           static_cast<const double>(materials[i].emission[2]));
    printf("  material.Ns = %f\n",
           static_cast<const double>(materials[i].shininess));
    printf("  material.Ni = %f\n", static_cast<const double>(materials[i].ior));
    printf("  material.dissolve = %f\n",
           static_cast<const double>(materials[i].dissolve));
    printf("  material.illum = %d\n", materials[i].illum);
    printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
    printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
    printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
    printf("  material.map_Ns = %s\n",
           materials[i].specular_highlight_texname.c_str());
    printf("  material.map_bump = %s\n", materials[i].bump_texname.c_str());
    printf("  material.map_d = %s\n", materials[i].alpha_texname.c_str());
    printf("  material.disp = %s\n", materials[i].displacement_texname.c_str());
    printf("  <<PBR>>\n");
    printf("  material.Pr     = %f\n", materials[i].roughness);
    printf("  material.Pm     = %f\n", materials[i].metallic);
    printf("  material.Ps     = %f\n", materials[i].sheen);
    printf("  material.Pc     = %f\n", materials[i].clearcoat_thickness);
    printf("  material.Pcr    = %f\n", materials[i].clearcoat_thickness);
    printf("  material.aniso  = %f\n", materials[i].anisotropy);
    printf("  material.anisor = %f\n", materials[i].anisotropy_rotation);
    printf("  material.map_Ke = %s\n", materials[i].emissive_texname.c_str());
    printf("  material.map_Pr = %s\n", materials[i].roughness_texname.c_str());
    printf("  material.map_Pm = %s\n", materials[i].metallic_texname.c_str());
    printf("  material.map_Ps = %s\n", materials[i].sheen_texname.c_str());
    printf("  material.norm   = %s\n", materials[i].normal_texname.c_str());
    std::map<std::string, std::string>::const_iterator it(
        materials[i].unknown_parameter.begin());
    std::map<std::string, std::string>::const_iterator itEnd(
        materials[i].unknown_parameter.end());

    for (; it != itEnd; it++) {
      printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");
  }
}

// set makeprg=cd\ ..\ &&\ make\ run\ >/dev/null
// vim: set spell spelllang=pt_br :

void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }

    ma_uint64 framesRead;
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    // Looping logic: If fewer frames are read, loop the audio
    if (framesRead < frameCount) {
        size_t bytesPerFrame = ma_get_bytes_per_frame(pDevice->playback.format, pDevice->playback.channels);

        // Fill the remaining frames by looping
        ma_uint64 remainingFrames = frameCount - framesRead;
        memset((char*)pOutput + framesRead * bytesPerFrame, 0, remainingFrames * bytesPerFrame); // Zero out remainder

        // Reset decoder and read from the beginning
        ma_decoder_seek_to_pcm_frame(pDecoder, 0);

        // Read the remaining frames after resetting
        ma_uint64 extraFramesRead;
        ma_decoder_read_pcm_frames(pDecoder, (char*)pOutput + framesRead * bytesPerFrame, remainingFrames, &extraFramesRead);
    }

    (void)pInput; // Unused
}
