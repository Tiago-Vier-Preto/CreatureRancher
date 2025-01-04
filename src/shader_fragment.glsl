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


in vec4 color_v;

// Matrizes computadas no código C++ e enviadas para a GPU
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Identificador que define qual objeto está sendo desenhado no momento
#define SPHERE 0
#define BUNNY  1
#define PLANE  2
#define ANEMO_SLIME 3
#define CRYO_SLIME 4
#define DENDRO_SLIME 5
#define PLASMA_SLIME 6
#define FIRE_SLIME 7
#define GEO_SLIME 8
#define ELECTRO_SLIME 9
#define WATER_SLIME 10
#define SKYBOX  11
#define WEAPON 12
#define HEAVEN_SKYBOX 29
#define GOD 30
#define MENU 31
#define CONTROLS 32
#define UPGRADES 33
#define STORE_MONSTER 34

uniform int object_id;

// Parâmetros da axis-aligned bounding box (AABB) do modelo
uniform vec4 bbox_min;
uniform vec4 bbox_max;

// Variáveis para acesso das imagens de textura
uniform sampler2D TextureImage0;
uniform sampler2D TextureImage1;
uniform sampler2D TextureImage2;

 // Slimes
uniform sampler2D anemo;
uniform sampler2D cryo;
uniform sampler2D dendro;
uniform sampler2D plasma;
uniform sampler2D fire;
uniform sampler2D geo;
uniform sampler2D electro;
uniform sampler2D water;

uniform samplerCube skybox; // Skybox

// texturas da arma 
uniform sampler2D TextureImage12;  // AOTexture
uniform sampler2D TextureImage13;  // Base Color
uniform sampler2D TextureImage14;  // Curvature
uniform sampler2D TextureImage15;  // Emmissive
uniform sampler2D TextureImage16;  // Metallic
uniform sampler2D TextureImage17;  // Normal
uniform sampler2D TextureImage18; // Opacity
uniform sampler2D TextureImage19; // Roughness

uniform sampler2D TextureImage20;
uniform sampler2D TextureImage21;
uniform sampler2D TextureImage22;
uniform sampler2D TextureImage23;
uniform sampler2D TextureImage24;
uniform sampler2D TextureImage25;
uniform sampler2D TextureImage26;
uniform sampler2D TextureImage27;
uniform sampler2D TextureImage28;

uniform samplerCube heaven_skybox;

uniform sampler2D god;

uniform sampler2D menu;
uniform sampler2D controls;
uniform sampler2D upgrades;

uniform sampler2D store_monster;

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
    else if (object_id == ANEMO_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(anemo, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == CRYO_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(cryo, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == DENDRO_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(dendro, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == PLASMA_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(plasma, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == FIRE_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(fire, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == GEO_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(geo, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == ELECTRO_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(electro, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == WATER_SLIME)
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(water, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == SKYBOX)
    {
        vec3 direction = normalize(vec3(texcoords, 1.0)); // Ajuste conforme necessário para obter a direção correta

        vec3 Kd0 = texture(skybox, vec3(position_model[0], position_model[1], position_model[2])).rgb;

        color.rgb = Kd0;
        
        color.rgb = pow(color.rgb, vec3(1.0, 1.0, 1.0) / 2.2);
    }
    else if (object_id == WEAPON) 
    {
        vec4 p = position_world;
        vec4 n = normalize(normal);

        // Sample textures
        vec3 baseColor = texture(TextureImage13, texcoords).rgb;
        float ao = texture(TextureImage12, texcoords).r;
        vec3 emissive = texture(TextureImage15, texcoords).rgb;
        float metallic = texture(TextureImage16, texcoords).r;
        float roughness = texture(TextureImage19, texcoords).r;
        float opacity = texture(TextureImage18, texcoords).r;

        // Curvature map for detail enhancement
        float curvature = texture(TextureImage14, texcoords).r;

        // Adjust normals using Tangent Space Normal Mapping
        vec3 tangentNormal = texture(TextureImage17, texcoords).rgb * 2.0 - 1.0;

        // Compute TBN matrix (Tangent-Bitangent-Normal)
        vec3 T = normalize(vec3(model * vec4(1.0, 0.0, 0.0, 0.0)));
        vec3 B = normalize(vec3(model * vec4(0.0, 1.0, 0.0, 0.0)));
        vec3 N = normalize(vec3(n));
        mat3 TBN = mat3(T, B, N);
        vec3 adjustedNormal = normalize(TBN * tangentNormal);

        // Lighting vectors
        vec3 lightDir = normalize(vec3(1.0, 1.0, 0.0)); // Light direction
        vec3 viewDir = normalize(vec3(camera_position - p)); // View direction

        // Apply curvature as a multiplier to baseColor
        vec3 detailedBaseColor = baseColor * (1.0 + curvature * 0.5);

        // Ambient light with AO
        vec3 ambient = detailedBaseColor * ao;

        // Diffuse shading
        float lambert = max(dot(adjustedNormal, lightDir), 0.0);
        vec3 diffuse = detailedBaseColor * lambert;

        // Specular reflection using Blinn-Phong with roughness
        vec3 halfDir = normalize(lightDir + viewDir);
        float specAngle = max(dot(adjustedNormal, halfDir), 0.0);
        float fresnel = pow(1.0 - dot(viewDir, halfDir), 5.0);
        float specular = fresnel * pow(specAngle, 1.0 / (roughness + 0.001));

        // Apply reflections (environment mapping with roughness)
        vec3 reflection = texture(skybox, reflect(-viewDir, adjustedNormal)).rgb;
        vec3 reflectionColor = mix(reflection, detailedBaseColor, metallic);

        // Final color computation
        vec3 finalColor = ambient + diffuse + specular * reflectionColor;
        finalColor += emissive; // Add emissive contribution

        color.rgb = finalColor;
        color.a = opacity; // Apply opacity
        color.rgb = pow(color.rgb, vec3(1.0, 1.0, 1.0) / 2.2); // Gamma correction

    } 
    else if (object_id == 20) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage20, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 21) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage21, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 22) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage22, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 23) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage23, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 24) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage24, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 25) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage25, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 26) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage26, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 27) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage27, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == 28) 
    {
        // Coordenadas de textura do plano, obtidas do arquivo OBJ.
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(TextureImage28, vec2(U,V)).rgb;

        float lambert = max(0,dot(n,l));

        color.rgb = Kd0 * (lambert + 0.01);
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == HEAVEN_SKYBOX)
    {
        vec3 direction = normalize(vec3(texcoords, 1.0)); // Ajuste conforme necessário para obter a direção correta

        vec3 Kd0 = texture(heaven_skybox, vec3(position_model[0], position_model[1], position_model[2])).rgb;

        color.rgb = Kd0;
        
        color.rgb = pow(color.rgb, vec3(1.0, 1.0, 1.0) / 2.2);
    }
    else if (object_id == GOD)
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(god, vec2(U,V)).rgb;
        color = color_v;
        color.rgb = Kd0 * 0.7f + color.rgb * 0.3f;
    }
    else if (object_id == MENU)
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(menu, vec2(U,V)).rgb;

        color.rgb = Kd0;
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == CONTROLS)
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(controls, vec2(U,V)).rgb;

        color.rgb = Kd0;
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == UPGRADES)
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(upgrades, vec2(U,V)).rgb;

        color.rgb = Kd0;
        color.a = 1;

        color.rgb = pow(color.rgb, vec3(1.0,1.0,1.0)/2.2);
    }
    else if (object_id == STORE_MONSTER)
    {
        U = texcoords.x;
        V = texcoords.y;

        vec3 Kd0 = texture(store_monster, vec2(U,V)).rgb;
        color = color_v;
        color.rgb = Kd0 * 0.7f + color.rgb * 0.3f;
    }
    else if (object_id == -1) 
    {
        color.rgb = vec3(1.0, 0.0, 0.0);
        
    }
    else
    {
        color.rgb = vec3(1.0, 0.0, 0.0);
        
    }

} 
