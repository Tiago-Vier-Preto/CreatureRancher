#version 330 core

// Atributos de fragmentos recebidos como entrada ("in") pelo Fragment Shader.
// Neste exemplo, este atributo foi gerado pelo rasterizador como a
// interpolação da posição global e a normal de cada vértice, definidas em
// "shader_vertex.glsl" e "main.cpp".
in vec4 position_world;
in vec4 normal;

// Posição do vértice atual no sistema de coordenadas local do modelo.
in vec4 position_model;

// Coordenadas de textura obtidas do arquivo OBJ (se existirem!)
in vec2 texcoords;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define CREATURE 3
uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;
uniform sampler2D TextureImage3; // Creature

// O valor de saída ("out") de um Fragment Shader é a cor final do fragmento.
out vec4 color;

// Constantes
#define M_PI   3.14159265358979323846
#define M_PI_2 1.57079632679489661923

void main()
{
    // Obtemos a posição da câmera utilizando a inversa da matriz que define o
    // sistema de coordenadas da câmera.
    vec4 origin = vec4(0.0, 0.0, 0.0, 1.0);
    vec4 camera_position = inverse(view) * origin;

    // O fragmento atual é coberto por um ponto que percente à superfície de um
    // dos objetos virtuais da cena. Este ponto, p, possui uma posição no
    // sistema de coordenadas global (World coordinates). Esta posição é obtida
    // através da interpolação, feita pelo rasterizador, da posição de cada
    // vértice.
    vec4 p = position_world;

    // Normal do fragmento atual, interpolada pelo rasterizador a partir das
    // normais de cada vértice.
    vec4 n = normalize(normal);

    // Vetor que define o sentido da fonte de luz em relação ao ponto atual.
    vec4 l = normalize(vec4(1.0,1.0,0.0,0.0));

    // Vetor que define o sentido da câmera em relação ao ponto atual.
    vec4 v = normalize(camera_position - p);

    // Coordenadas de textura U e V
    float U = 0.0;
    float V = 0.0;

    if ( object_id == SPHERE )
    {
        // PREENCHA AQUI as coordenadas de textura da esfera, computadas com
        // projeção esférica EM COORDENADAS DO MODELO. Utilize como referência
        // o slides 134-150 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // A esfera que define a projeção deve estar centrada na posição
        // "bbox_center" definida abaixo.

        // Você deve utilizar:
        //   função 'length( )' : comprimento Euclidiano de um vetor
        //   função 'atan( , )' : arcotangente. Veja https://en.wikipedia.org/wiki/Atan2.
        //   função 'asin( )'   : seno inverso.
        //   constante M_PI
        //   variável position_model

        vec4 bbox_center = (bbox_min + bbox_max) / 2.0;

        vec4 p_v = position_model - bbox_center;

        float h = length(p_v);

        float theta = atan(p_v.x, p_v.z);

        float phi = asin(p_v.y / h);

        U = (theta + M_PI) / (2.0 * M_PI);
        V = (phi + M_PI_2) / M_PI;

        vec3 Kd0 = texture(TextureImage0, vec2(U,V)).rgb;

        vec3 Kd1 = texture(TextureImage1, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01) + Kd1 * (1.0 - lambert);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);

    }
    else if ( object_id == BUNNY )
    {
        // PREENCHA AQUI as coordenadas de textura do coelho, computadas com
        // projeção planar XY em COORDENADAS DO MODELO. Utilize como referência
        // o slides 99-104 do documento Aula_20_Mapeamento_de_Texturas.pdf,
        // e também use as variáveis min*/max* definidas abaixo para normalizar
        // as coordenadas de textura U e V dentro do intervalo [0,1]. Para
        // tanto, veja por exemplo o mapeamento da variável 'p_v' utilizando
        // 'h' no slides 158-160 do documento Aula_20_Mapeamento_de_Texturas.pdf.
        // Veja também a Questão 4 do Questionário 4 no Moodle.

        float minx = bbox_min.x;
        float maxx = bbox_max.x;

        float miny = bbox_min.y;
        float maxy = bbox_max.y;

        float minz = bbox_min.z;
        float maxz = bbox_max.z;

        vec4 p_v = position_model; 

        float h = max(maxx - minx, maxy - miny);

        U = (p_v.x - minx) / h;
        V = (p_v.y - miny) / h;

        vec3 Kd0 = texture(TextureImage0, vec2(U,V)).rgb;

        vec3 Kd1 = texture(TextureImage1, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01) + Kd1 * (1.0 - lambert);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if ( object_id == PLANE )
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage2, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == CREATURE)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage3, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
} 

