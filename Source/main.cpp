#include "../Externals/Include/Include.h"
#include "../Externals/Include/assimp/Importer.hpp"
#include "../Externals/Include/assimp/scene.h"
#include "../Externals/Include/assimp/postprocess.h"
#include "../Externals/Include/assimp/types.h"

// Menu control
#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define MENU_IDLE 4
#define MENU_ABSTRACT 5
#define MENU_COMPARE 6
#define MENU_SAME 7
#define MENU_STEREO 8
#define MENU_PIXEL 9
#define MENU_LAPLACIAN 10
#define MENU_BLOOM 11
// #define MENU_MAGNIFIER 12
#define MENU_NOISE 13
#define MENU_TWIST 14
#define MENU_POSTERIZATION 15

const int maxEffect = 8;

#define PI 3.141592
#define M_PI 3.1415926535897932384626433832795

// Scene from: https://sketchfab.com/models/a56eba5cb42e477dbebe093aeab7812e#
const char *myScene = "Mineways2Skfb.obj";
const char *defaultSceneTex = "Mineways2Skfb-RGBA.png";

void My_Reshape(int width, int height);

GLubyte timer_cnt = 0;
float timer_val = 1.0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

using namespace glm;
using namespace std;

// Transformation matrices and vectors
mat4 model_matrix;
mat4 view_matrix;
mat4 proj_matrix;
vec3 charEye = vec3(0.3, -19.05, 0.4);
vec3 charCenter = vec3(0.3, -19.05, 0.05);
vec3 eye = charEye;
vec3 center = charCenter;
vec3 up = vec3(0.0, 1.0, 0.0);

// For character
mat4 model_head;
mat4 model_torso;
mat4 model_waste;

mat4 model_larm;
mat4 model_rarm;

mat4 model_lhand;
mat4 model_rhand;

mat4 model_lleg;
mat4 model_rleg;

mat4 model_lfeet;
mat4 model_rfeet;

mat4 model_lshoulder;
mat4 model_rshoulder;
mat4 model_lttoleg;
mat4 model_rttoleg;

mat4 rot_torso = mat4();
mat4 scaling = scale(mat4(), vec3(0.04, 0.04, 0.04));
mat4 translation = translate(mat4(), vec3(0.0, -19.8, 0.0));

mat4 r_head;
mat4 r_lhand;
mat4 r_rhand;
mat4 r_lleg;
mat4 r_rleg;
mat4 r_lhand2;
mat4 r_rhand2;
mat4 r_lleg2;
mat4 r_rleg2;
mat4 r_axis;
mat4 bugg;
mat4 mv;
mat4 mvp;

GLfloat lefthand = 0.0;
GLfloat leftleg = 0.0;
GLfloat head_ang = 0.0;
GLfloat bug_ang = 0.0;
GLfloat righthand = 0.0;
GLfloat rightleg = 0.0;

int rot_x = 0, rot_y = 0, old_rot_x = 0, old_rot_y = 0, record_x = 0, record_y = 0;
int rotate_x, rotate_y;
vec3 temp;
vec3 charTrans = vec3(0.3, 0.7, 0.0);
bool charMode = true; // See if camera follows character

// For rotating torso
// Scale: 0 ~ 89, clockwise
int facing = 0;
const int maxFacing = 89;

int i;
int lhandtag = 0;
int llegtag = 0;
int rhandtag = 0;
int rlegtag = 0;
int headtag = 0;
int mousetag = 0;

GLint modeid;
GLint um4mvp;
GLint uniform_mv;
GLuint char_program;
GLuint tex_object;
GLuint tex_object2;
GLuint tex_object3;

// For original scene
GLint um4p;
GLint um4mv;
GLint tex;
GLuint program;

// For skybox
GLuint skybox_prog;
GLuint tex_envmap;
GLuint skybox_vao;
GLint tex_cubemap;

int needFog = 1;
GLint fog1;
GLint fog2;

// For frame buffer object (FBO)
GLuint program2;
GLuint window_vao;
GLuint window_buffer;
GLuint FBO;
GLuint depthRBO;
GLuint FBODataTexture;

int current_x = 0, current_y = 0;
int fbo_mode = 0;

GLint mode;
GLint compare;
GLint mouse_x;
GLint mouse_y;
GLint time_val;

// For meshes
struct Shape
{
    GLuint vao;
    GLuint vbo_position;
    GLuint vbo_normal;
    GLuint vbo_texcoord;
    GLuint ibo;
    int drawCount;
    int materialID;
};

struct Material
{
    GLuint diffuse_tex;
};

// Vector of meshes and materials
vector<Material> mat_vector;
vector<Shape> shape_vector;

// For character
typedef struct
{
	GLuint vao;
	GLuint vbo;
	GLuint vboTex;
	GLuint ebo;

	GLuint p_normal;
	int materialId;
	int indexCount;
	GLuint m_texture;
} SmallShape;

SmallShape cube;
SmallShape head_shpae;
SmallShape arm_shpae;
SmallShape elbow_shpae;
std::vector<tinyobj::shape_t> shapes;

// For skybox
struct
{
    struct
    {
        GLint inv_vp_matrix;
        GLint eye_pos;
    } skybox;
} uniforms;

char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete[] srcp[0];
    delete[] srcp;
}

// Position and texture coord of the FBO
static const GLfloat window_positions[] =
{
    1.0f, -1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f
};

// define a simple data structure for storing texture image raw data
typedef struct _TextureData
{
    _TextureData(void) :
    width(0),
    height(0),
    data(0)
    {
    }
    
    int width;
    int height;
    unsigned char* data;
} TextureData;

// load a png image and return a TextureData structure with raw data
// not limited to png format. works with any image format that is RGBA-32bit
TextureData loadPNG(const char* const pngFilepath)
{
    TextureData texture;
    int components;
    
    // load the texture with stb image, force RGBA (4 components required)
    stbi_uc *data = stbi_load(pngFilepath, &texture.width, &texture.height, &components, 4);
    
    // is the image successfully loaded?
    if (data != NULL)
    {
        // copy the raw data
        size_t dataSize = texture.width * texture.height * 4 * sizeof(unsigned char);
        texture.data = new unsigned char[dataSize];
        memcpy(texture.data, data, dataSize);
        
        // mirror the image vertically to comply with OpenGL convention
        for (size_t i = 0; i < texture.width; ++i)
        {
            for (size_t j = 0; j < texture.height / 2; ++j)
            {
                for (size_t k = 0; k < 4; ++k)
                {
                    size_t coord1 = (j * texture.width + i) * 4 + k;
                    size_t coord2 = ((texture.height - j - 1) * texture.width + i) * 4 + k;
                    std::swap(texture.data[coord1], texture.data[coord2]);
                }
            }
        }
        
        // release the loaded image
        stbi_image_free(data);
    }
    
    return texture;
}

void My_LoadModels()
{
	std::vector<tinyobj::material_t> materials;
	std::string err;

	bool ret = tinyobj::LoadObj(shapes, materials, err, "cube.obj");
	if (err.size()>0)
	{
		printf("Load Models Fail! Please check the solution path");
		return;
	}

	printf("Load Models Success ! Shapes size %d Maerial size %d\n", shapes.size(), materials.size());

	for (int i = 0; i < shapes.size(); i++)
	{

		glGenVertexArrays(1, &cube.vao);
		glBindVertexArray(cube.vao);

		glGenBuffers(3, &cube.vbo);
		glGenBuffers(1, &cube.p_normal);
		glBindBuffer(GL_ARRAY_BUFFER, cube.vbo);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * sizeof(float) + shapes[i].mesh.normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);

		glBufferSubData(GL_ARRAY_BUFFER, 0, shapes[i].mesh.positions.size() * sizeof(float), &shapes[i].mesh.positions[0]);
		glBufferSubData(GL_ARRAY_BUFFER, shapes[i].mesh.positions.size() * sizeof(float), shapes[i].mesh.normals.size() * sizeof(float), &shapes[i].mesh.normals[0]);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void *)(shapes[i].mesh.positions.size() * sizeof(float)));

		glBindBuffer(GL_ARRAY_BUFFER, cube.p_normal);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.normals.size() * sizeof(float), shapes[i].mesh.normals.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glBindBuffer(GL_ARRAY_BUFFER, cube.vboTex);
		glBufferData(GL_ARRAY_BUFFER, shapes[i].mesh.texcoords.size() * sizeof(float), shapes[i].mesh.texcoords.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube.ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, shapes[i].mesh.indices.size() * sizeof(unsigned int), shapes[i].mesh.indices.data(), GL_STATIC_DRAW);
		cube.materialId = shapes[i].mesh.material_ids[0];
		cube.indexCount = shapes[i].mesh.indices.size();


		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

	}
}

// Load a scene and get the models' verices, textures, and normals
void My_LoadScenes(const char *modelName)
{
    // Load the scene
    const aiScene *scene = aiImportFile(modelName, aiProcessPreset_TargetRealtime_MaxQuality);
    if (!scene)
    {
        printf("Load model Fail!");
        exit(1);
    }
    
    // Load the textures
    printf("Load material...\n");
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        aiMaterial *material = scene->mMaterials[i];
        Material mat;
        aiString texturePath;
        const char* def_texture = defaultSceneTex;
        
        if (material -> GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == aiReturn_SUCCESS)
        {
            TextureData tdata = loadPNG(texturePath.C_Str());
            // printf("%s\n", texturePath.C_Str());
            glGenTextures(1, &mat.diffuse_tex);
            glBindTexture(GL_TEXTURE_2D, mat.diffuse_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            TextureData tdata = loadPNG(def_texture);
            glGenTextures(1, &mat.diffuse_tex);
            glBindTexture(GL_TEXTURE_2D, mat.diffuse_tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tdata.width, tdata.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        mat_vector.push_back(mat);
    }
    
    // Load the models
    printf("Load meshes...\n");
    vector<float> positions;
    vector<float> normals;
    vector<float> texcoords;
    vector<unsigned int> indices;
    
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[i];
        Shape shape;
        glGenVertexArrays(1, &shape.vao);
        glBindVertexArray(shape.vao);
        positions.clear();
        normals.clear();
        texcoords.clear();
        indices.clear();
        
        for (unsigned int v = 0; v < mesh->mNumVertices; ++v)
        {
            positions.push_back(mesh->mVertices[v].x);
            positions.push_back(mesh->mVertices[v].y);
            positions.push_back(mesh->mVertices[v].z);
            normals.push_back(mesh->mNormals[v].x);
            normals.push_back(mesh->mNormals[v].y);
            normals.push_back(mesh->mNormals[v].z);
            texcoords.push_back(mesh->mTextureCoords[0][v].x);
            texcoords.push_back(mesh->mTextureCoords[0][v].y);
        }
        for (unsigned int f = 0; f < mesh->mNumFaces; ++f)
        {
            indices.push_back(mesh->mFaces[f].mIndices[0]);
            indices.push_back(mesh->mFaces[f].mIndices[1]);
            indices.push_back(mesh->mFaces[f].mIndices[2]);
        }
        
        glGenBuffers(1, &shape.vbo_position);
        glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_position);
        glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(float), &positions[0], GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        
        glGenBuffers(1, &shape.vbo_texcoord);
        glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_texcoord);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(float), &texcoords[0], GL_STATIC_DRAW);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(1);
        
        glGenBuffers(1, &shape.vbo_normal);
        glBindBuffer(GL_ARRAY_BUFFER, shape.vbo_normal);
        glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(float), &normals[0], GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(2);
        
        glGenBuffers(1, &shape.ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, shape.ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        
        shape.materialID = mesh->mMaterialIndex;
        shape.drawCount = mesh->mNumFaces * 3;
        shape_vector.push_back(shape);
    }
    aiReleaseImport(scene);
}

// Load the texture for skybox
void My_LoadSkybox()
{
    // Skybox from: http://www.custommapmakers.org/skyboxes.php
    vector<TextureData> sky_tex(6);
    sky_tex[0] = loadPNG("siege_rt.tga");
    sky_tex[1] = loadPNG("siege_lf.tga");
    sky_tex[2] = loadPNG("siege_up.tga");
    sky_tex[3] = loadPNG("siege_dn.tga");
    sky_tex[4] = loadPNG("siege_ft.tga");
    sky_tex[5] = loadPNG("siege_bk.tga");
    glGenTextures(1, &tex_envmap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);
    for(int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, sky_tex[i].width, sky_tex[i].height, 0, GL_RGBA, GL_UNSIGNED_BYTE, sky_tex[i].data);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    for(int i = 0; i < 6; ++i)
    {
        delete[] sky_tex[i].data;
    }
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

// Initialize scene
void my_InitScene(const char *sceneName)
{
    program = glCreateProgram();
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
    char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
    freeShaderSource(vertexShaderSource);
    freeShaderSource(fragmentShaderSource);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    um4p = glGetUniformLocation(program, "um4p");
    um4mv = glGetUniformLocation(program, "um4mv");
    tex = glGetUniformLocation(program, "tex");
    fog1 = glGetUniformLocation(program, "fog1");
    
    glUseProgram(program);
    
    printf("Load model start!\n");
    My_LoadScenes(sceneName);
    printf("Load model finish!\n");
}

// Initialize skybox
void my_InitSkybox()
{
    skybox_prog = glCreateProgram();
    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    char** fragmentShaderSource_sky = loadShaderSource("fragment_sky.fs.glsl");
    glShaderSource(fs, 1, fragmentShaderSource_sky, NULL);
    freeShaderSource(fragmentShaderSource_sky);
    glCompileShader(fs);
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    char** vertexShaderSource_sky = loadShaderSource("vertex_sky.vs.glsl");
    glShaderSource(vs, 1, vertexShaderSource_sky, NULL);
    freeShaderSource(vertexShaderSource_sky);
    glCompileShader(vs);
    glAttachShader(skybox_prog, vs);
    glAttachShader(skybox_prog, fs);
    glLinkProgram(skybox_prog);
    glUseProgram(skybox_prog);
    
    uniforms.skybox.inv_vp_matrix = glGetUniformLocation(skybox_prog, "inv_vp_matrix");
    uniforms.skybox.eye_pos = glGetUniformLocation(skybox_prog, "eye");
    tex_cubemap = glGetUniformLocation(skybox_prog, "tex_cubemap");
    
    My_LoadSkybox();
    
    glGenVertexArrays(1, &skybox_vao);
}

// Initialize character
void my_InitChar()
{
	// Create Shader Program
	char_program = glCreateProgram();

	// Create customize shader by tell openGL specify shader type 
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Load shader file
	char** vertexShaderSource = loadShaderSource("vertex_char.vs.glsl");
	char** fragmentShaderSource = loadShaderSource("fragment_char.fs.glsl");

	// Assign content of these shader files to those shaders we created before
	glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
	glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);

	// Free the shader file string(won't be used any more)
	freeShaderSource(vertexShaderSource);
	freeShaderSource(fragmentShaderSource);

	// Compile these shaders
	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	// Assign the program we created before with these shaders
	glAttachShader(char_program, vertexShader);
	glAttachShader(char_program, fragmentShader);
	glLinkProgram(char_program);

	// Get the id of inner variable 'um4mvp' in shader programs
	um4mvp = glGetUniformLocation(char_program, "um4mvp");
	uniform_mv = glGetUniformLocation(char_program, "uniform_mv");
	modeid = glGetUniformLocation(char_program, "mode");
    fog2 = glGetUniformLocation(program, "fog2");
	// Tell OpenGL to use this shader program now
	glUseProgram(char_program);


	///////////////////////////////////
	//texture

	TextureData tex = loadPNG("2.png");
	glGenTextures(1, &tex_object);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	///
	TextureData tex2 = loadPNG("2.png");
	glGenTextures(1, &tex_object2);
	glBindTexture(GL_TEXTURE_2D, tex_object2);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex2.width, tex2.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex2.data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	//Create vertex array object and bind it to  OpenGL (OpenGL will apply operation only to the vertex array objects it bind)
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Enable shader layout location 0 and 1 for vertex and color   
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	//Create buffer and bind it to OpenGL
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	My_LoadModels();
}

//Initialize frame buffer object
void My_InitFBO()
{
    program2 = glCreateProgram();
    
    GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);
    char** vertexShaderSource2 = loadShaderSource("vertex2.vs.glsl");
    glShaderSource(vs2, 1, vertexShaderSource2, NULL);
    freeShaderSource(vertexShaderSource2);
    glCompileShader(vs2);
    GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);
    char** fragmentShaderSource2 = loadShaderSource("fragment2.fs.glsl");
    glShaderSource(fs2, 1, fragmentShaderSource2, NULL);
    freeShaderSource(fragmentShaderSource2);
    glCompileShader(fs2);
    glAttachShader(program2, vs2);
    glAttachShader(program2, fs2);
    
    glLinkProgram(program2);
    glUseProgram(program2);
    
    mode = glGetUniformLocation(program2, "mode");
    compare = glGetUniformLocation(program2, "compare");
    mouse_x = glGetUniformLocation(program2, "mouse_x");
    mouse_y = glGetUniformLocation(program2, "mouse_y");
    time_val = glGetUniformLocation(program2, "time_val");
    
    glGenVertexArrays(1, &window_vao);
    glBindVertexArray(window_vao);
    
    glGenBuffers(1, &window_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, window_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(window_positions), window_positions, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GL_FLOAT) * 4, (const GLvoid*)(sizeof(GL_FLOAT) * 2));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    
    glGenFramebuffers(1, &FBO);
}

void My_Init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // Scene
    my_InitScene(myScene);
    
    // Skybox
    my_InitSkybox();

	// Character
	my_InitChar();
    
    // Frame buffer object
    My_InitFBO();
}

void My_Display()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    
    // Clear depth and color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    static const GLfloat gray[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    static const GLfloat ones[] = { 1.0f };
    glClearBufferfv(GL_COLOR, 0, gray);
    glClearBufferfv(GL_DEPTH, 0, ones);
    
    mat4 Identy_Init(1.0);
    model_matrix = translate(Identy_Init, vec3(0.0, -20.0, 0.0));
    view_matrix = lookAt(eye, center, up);
    mat4 inv_vp_matrix = inverse(proj_matrix * view_matrix);
    
    // Draw skybox
    glUniform1i(tex_cubemap, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex_envmap);
    
    glUseProgram(skybox_prog);
    glBindVertexArray(skybox_vao);
    
    glUniformMatrix4fv(uniforms.skybox.inv_vp_matrix, 1, GL_FALSE, &inv_vp_matrix[0][0]);
    glUniform3fv(uniforms.skybox.eye_pos, 1, &eye[0]);
    
    glDisable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glEnable(GL_DEPTH_TEST);
    
    // Draw original scene
    glUseProgram(program);
    
    glUniformMatrix4fv(um4mv, 1, GL_FALSE, value_ptr(view_matrix * model_matrix));
    glUniformMatrix4fv(um4p, 1, GL_FALSE, value_ptr(proj_matrix));
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(tex, 0);
    glUniform1i(fog1, needFog);
    
    for (int i = 0; i < shape_vector.size(); i++)
    {
        glBindVertexArray(shape_vector[i].vao);
        int materialID = shape_vector[i].materialID;
        glBindTexture(GL_TEXTURE_2D, mat_vector[materialID].diffuse_tex);
        glDrawElements(GL_TRIANGLES, shape_vector[i].drawCount, GL_UNSIGNED_INT, 0);
    }

	// Draw character
	//torso
    glUniform1i(fog2, needFog);
    glUseProgram(char_program);
	glUniform1i(modeid, 0);
	glBindVertexArray(cube.vao);
	model_torso = translate(translation, charTrans);
	mvp = proj_matrix * view_matrix * model_torso * rot_torso * scale(mat4(), vec3(0.8, 1.2, 0.5)) * scaling;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mvp));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//head
	glUniform1i(modeid, 2);
	glBindVertexArray(cube.vao);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glUseProgram(char_program);
	r_head = rotate(mat4(), float(deg2rad(head_ang * 1.0f)), vec3(1.0f, 0.0f, 0.0f));
	model_head = translate(mat4(), vec3(0.0, 0.035, 0.0));
	mvp = proj_matrix * view_matrix * model_torso * model_head * rot_torso * scale(mat4(), vec3(0.8, 0.5, 0.5)) * scaling;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

    float num1, num2;
    num1 = -cos(deg2rad(360 / (maxFacing + 1) * facing));
    num2 = sin(deg2rad(360 / (maxFacing + 1) * facing));

	//shoulder
    r_lhand = rotate(mat4(), float(deg2rad(-lefthand) * num1), vec3(1.0, 0.0, 0.0));
    r_lhand2 = rotate(mat4(), float(deg2rad(lefthand) * num2), vec3(0.0, 0.0, 1.0));
	model_lshoulder = r_lhand2 * r_lhand * scaling * rot_torso * translate(mat4(), vec3(0.625, 0.5, 0.0));
    r_rhand = rotate(mat4(), float(deg2rad(lefthand) * num1), vec3(1.0, 0.0, 0.0));
    r_rhand2 = rotate(mat4(), float(deg2rad(-lefthand) * num2), vec3(0.0, 0.0, 1.0));
	model_rshoulder = r_rhand2 * r_rhand * scaling * rot_torso * translate(mat4(), vec3(-0.625, 0.5, 0.0));

	//torso to leg
    r_lleg = rotate(mat4(), float(deg2rad(leftleg) * num1), vec3(1.0, 0.0, 0.0));
    r_lleg2 = rotate(mat4(), float(deg2rad(-leftleg) * num2), vec3(0.0, 0.0, 1.0));
    model_lttoleg = r_lleg2 * r_lleg * scaling * rot_torso * translate(mat4(), vec3(0.225, -1.1, 0.0));
    r_rleg = rotate(mat4(), float(deg2rad(-leftleg) * num1), vec3(1.0, 0.0, 0.0));
    r_rleg2 = rotate(mat4(), float(deg2rad(leftleg) * num2), vec3(0.0, 0.0, 1.0));
    model_rttoleg = r_rleg2 * r_rleg * scaling * rot_torso * translate(mat4(), vec3(-0.225, -1.1, 0.0));

	//larm
	glUniform1i(modeid, 0);
	glBindVertexArray(cube.vao);
	glUseProgram(char_program);
	model_larm = translate(mat4(), vec3(0, -0.3, 0.0));
	mv = view_matrix * model_torso * model_lshoulder * model_larm * scale(mat4(), vec3(0.4, 0.7, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//lhand
	glUniform1i(modeid, 2);
	glBindVertexArray(cube.vao);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glUseProgram(char_program);
	model_lhand = translate(mat4(), vec3(0, -0.575, 0.0));
	mv = view_matrix * model_torso * model_lshoulder * model_larm * model_lhand * scale(mat4(), vec3(0.4, 0.4, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//rarm
	glUniform1i(modeid, 0);
	glBindVertexArray(cube.vao);
	glUseProgram(char_program);
	model_rarm = translate(mat4(), vec3(0, -0.3, 0.0));
	mv = view_matrix * model_torso * model_rshoulder * model_rarm * scale(mat4(), vec3(0.4, 0.7, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//lhand
	glUniform1i(modeid, 2);
	glBindVertexArray(cube.vao);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glUseProgram(char_program);
	model_rhand = translate(mat4(), vec3(0, -0.575, 0.0));
	mv = view_matrix * model_torso * model_rshoulder * model_rarm * model_rhand * scale(mat4(), vec3(0.4, 0.4, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//lleg
	glUniform1i(modeid, 0);
	glBindVertexArray(cube.vao);
	glUseProgram(char_program);
	model_lleg = translate(mat4(), vec3(0.0, 0.0, 0.0));
	mv = view_matrix * model_torso * model_lttoleg * model_lleg * scale(mat4(), vec3(0.4, 0.9, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//lfeet
	glUniform1i(modeid, 2);
	glBindVertexArray(cube.vao);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glUseProgram(char_program);
	model_lfeet = translate(mat4(), vec3(0, -0.575, 0.0));
	mv = view_matrix * model_torso * model_lttoleg * model_lleg * model_lfeet * scale(mat4(), vec3(0.4, 0.25, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//rleg
	glUniform1i(modeid, 0);
	glBindVertexArray(cube.vao);
	glUseProgram(char_program);
	model_rleg = translate(mat4(), vec3(0.0, 0.0, 0.0));
	mv = view_matrix * model_torso * model_rttoleg * model_rleg * scale(mat4(), vec3(0.4, 0.9, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);

	//rfeet
	glUniform1i(modeid, 2);
	glBindVertexArray(cube.vao);
	glBindTexture(GL_TEXTURE_2D, tex_object);
	glUseProgram(char_program);
	model_rfeet = translate(mat4(), vec3(0, -0.575, 0.0));
	mv = view_matrix * model_torso * model_rttoleg * model_rleg * model_rfeet * scale(mat4(), vec3(0.4, 0.25, 0.4));
	mvp = proj_matrix * mv;
	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
	glUniformMatrix4fv(uniform_mv, 1, GL_FALSE, value_ptr(mv));
	glDrawElements(GL_TRIANGLES, cube.indexCount, GL_UNSIGNED_INT, 0);
    
    // Rebind to the FBO
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    
    glUseProgram(program2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FBODataTexture);
    glBindVertexArray(window_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    
    glutSwapBuffers();
}

void My_Reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    
    float viewportAspect = (float)width / (float)height;
    proj_matrix = perspective(radians(60.0f), viewportAspect, 0.1f, 1000.0f);
    
    // Create depth FBO
    glDeleteRenderbuffers(1, &depthRBO);
    glDeleteTextures(1, &FBODataTexture);
    glGenRenderbuffers(1, &depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, width, height);
    
    // Create color FBO
    glGenTextures(1, &FBODataTexture);
    glBindTexture(GL_TEXTURE_2D, FBODataTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Set depth FBO and color FBO to current FBO
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, FBODataTexture, 0);

}

int motivation = 0;
int animation_count = 0;
int jump = 1;
int jumptag = 1;
int location = 0;

void My_Timer(int val)
{
    timer_cnt++;
    timer_val = timer_val + 0.001;
    glutPostRedisplay();
    glUniform1f(time_val, timer_val);
    if (timer_enabled)
    {
        if (motivation == 1) { //forward animation
            if (animation_count == 0) {
                if (lhandtag == 0) {
                    lefthand += 3.0;
                    if (lefthand >= 90.0)
                        lhandtag = 1;
                }
                else if (lhandtag == 1) {
                    lefthand -= 3.0;
                    if (lefthand <= -90.0)
                        lhandtag = 2;
                }
                else {
                    lefthand += 3.0;
                    if (lefthand == 0.0)
                    {
                        lhandtag = 0;
                        animation_count = 1;
                    }
                }
                if (llegtag == 0) {
                    leftleg += 1.0;
                    if (leftleg >= 30.0)
                        llegtag = 1;
                }
                else if (llegtag == 1) {
                    leftleg -= 1.0;
                    if (leftleg <= -30.0)
                        llegtag = 2;
                }
                else {
                    leftleg += 1.0;
                    if (leftleg == 0.0)
                    {
                        llegtag = 0;
                    }
                }
            }
            else if (animation_count == 1) {
                motivation = 0;
                animation_count = 0;
            }
        }
        else if (motivation == 2) { //backward animation
            if (animation_count == 0) {
                if (lhandtag == 0) {
                    lefthand += 0.5;
                    if (lefthand >= 15.0)
                        lhandtag = 1;
                }
                else if (lhandtag == 1) {
                    lefthand -= 0.5;
                    if (lefthand <= -15.0)
                        lhandtag = 2;
                }
                else {
                    lefthand += 0.5;
                    if (lefthand == 0.0)
                    {
                        lhandtag = 0;
                        animation_count = 1;
                    }
                }
                if (llegtag == 0) {
                    leftleg += 1.0;
                    if (leftleg >= 30.0)
                        llegtag = 1;
                }
                else if (llegtag == 1) {
                    leftleg -= 1.0;
                    if (leftleg <= -30.0)
                        llegtag = 2;
                }
                else {
                    leftleg += 1.0;
                    if (leftleg == 0.0)
                    {
                        llegtag = 0;
                    }
                }
            }
            else if (animation_count == 1) {
                motivation = 0;
                animation_count = 0;
            }
        }
        if (jump == 0)
        {
            if (jumptag == 1) {
                if (location <= 20) {
                    location++;
                    charTrans += vec3(0, 0.001, 0);
                }
                else jumptag = 0;
            }
            else if (jumptag == 0) {
                if (location >= 0) {
                    location--;
                    charTrans -= vec3(0, 0.001, 0);
                }
                else {
                    jumptag = 1;
                    jump = 1;
                }
            }
        }
    }
    glutTimerFunc(timer_speed, My_Timer, val);
}

void My_Mouse(int button, int state, int x, int y)
{
    if(state == GLUT_DOWN)
    {
        printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
    }
    else if(state == GLUT_UP)
    {
        printf("Mouse %d is released at (%d, %d)\n", button, x, y);
        if (x >= 20 && x <= 50 && y >= 550 && y <= 580)
        {
            // toot(400.0, 100);
            if (fbo_mode == 0)
            {
                fbo_mode = maxEffect;
                glUniform1i(mode, maxEffect);
            }
            else
            {
                fbo_mode--;
                glUniform1i(mode, fbo_mode);
            }
        }
        else if (x >= 70 && x <= 100 && y >= 550 && y <= 580)
        {
            // toot(400.0, 100);
            if (fbo_mode == maxEffect)
            {
                fbo_mode = 0;
                glUniform1i(mode, 0);
            }
            else
            {
                fbo_mode++;
                glUniform1i(mode, fbo_mode);
            }
        }
        else if (x >= 750 && x <= 780 && y >= 550 && y <= 580)
        {
            printf("a");
            charMode = !charMode;
            if (charMode)
            {
                eye = charEye;
                center = charCenter;
            }
        }
        else if (x >= 20 && x <= 50 && y >= 20 && y <= 50)
        {
            if (needFog == 1)
            {
                needFog = 0;
            }
            else
            {
                needFog = 1;
            }
            glUniform1i(fog1, needFog);
        }
        else if (x >= 750 && x <= 780 && y >= 20 && y <= 50)
        {
            exit(0);
        }
    }
    if (button == GLUT_LEFT_BUTTON)
    {
        current_x = x;
        current_y = y;
        glUniform1i(mouse_x, current_x);
        glUniform1i(mouse_y, current_y);
    }
}

// Drag the mouse to rotate the camera view
void My_MouseMotion(int x, int y)
{
    int diff_x = x - current_x;
    int diff_y = y - current_y;
    current_x = x;
    current_y = y;
    glUniform1i(mouse_x, current_x);
    glUniform1i(mouse_y, current_y);
    mat3 rotY = mat3(cos(diff_x / 400.0), 0.0, sin(diff_x / 400.0),
                     0.0, 1.0, 0.0,
                     -sin(diff_x / 400.0), 0.0, cos(diff_x / 400.0));
    mat3 rotX = mat3(1.0, 0.0, 0.0,
                     0.0, cos(diff_y / 400.0), -sin(diff_y / 400.0),
                     0.0, sin(diff_y / 400.0), cos(diff_y / 400.0));
    
    center = center - eye;
    center = rotY * rotX * center;
    center = center + eye;
}

// Press i to print the camera space information
void My_Keyboard(unsigned char key, int x, int y)
{
    vec3 move_lr = normalize(cross(center - eye, up));
    vec3 move_fb = normalize(center - eye);
    vec3 oldEye = charEye;
    mat3 rotRight = mat3(cos(deg2rad(360 / (maxFacing + 1))), 0.0, sin(deg2rad(360 / (maxFacing + 1))),
                         0.0, 1.0, 0.0,
                         -sin(deg2rad(360 / (maxFacing + 1))), 0.0, cos(deg2rad(360 / (maxFacing + 1))));
    mat3 rotLeft = mat3(cos(deg2rad(-360 / (maxFacing + 1))), 0.0, sin(deg2rad(-360 / (maxFacing + 1))),
                        0.0, 1.0, 0.0,
                        -sin(deg2rad(-360 / (maxFacing + 1))), 0.0, cos(deg2rad(-360 / (maxFacing + 1))));

    switch(key)
    {
        case 'q':
			charEye = vec3(charEye[0], charEye[1] - 0.01, charEye[2]);
			charCenter = vec3(charCenter[0], charCenter[1] - 0.01, charCenter[2]);
            eye = charEye;
            center = charCenter;
            charTrans = charTrans - vec3(0.0, 0.01, 0.0);
			break;
        case 'e':
			charEye = vec3(charEye[0], charEye[1] + 0.01, charEye[2]);
			charCenter = vec3(charCenter[0], charCenter[1] + 0.01, charCenter[2]);
            eye = charEye;
            center = charCenter;
            charTrans = charTrans + vec3(0.0, 0.01, 0.0);
			break;
		case 'a':
            charEye = charEye - charCenter;
            charEye = rotLeft * charEye;
            charEye = charEye + charCenter;
            rot_torso = rotate(rot_torso, float(deg2rad(360 / (maxFacing + 1))), vec3(0.0, 1.0, 0.0));
            if (facing == 0)
            {
                facing = maxFacing;
            }
            else
            {
                facing--;
            }
            if (charMode)
            {
                eye = charEye;
                center = charCenter;
            }
			printf("Left arrow is pressed at (%d, %d)\n", x, y);
			break;
		case 'd':
            charEye = charEye - charCenter;
            charEye = rotRight * charEye;
            charEye = charEye + charCenter;
            rot_torso = rotate(rot_torso, float(deg2rad(-360 / (maxFacing + 1))), vec3(0.0, 1.0, 0.0));
            if (facing == maxFacing)
            {
                facing = 0;
            }
            else
            {
                facing++;
            }
            if (charMode)
            {
                eye = charEye;
                center = charCenter;
            }
			printf("Right arrow is pressed at (%d, %d)\n", x, y);
			break;
		case 'w':
            if (motivation == 0)
            {
                motivation = 1;
            }
            charTrans = charTrans + vec3((charCenter.x - charEye.x) * 0.02, (charCenter.y - charEye.y) * 0.02, (charCenter.z - charEye.z) * 0.02);
            charEye = charEye + vec3((charCenter.x - charEye.x) * 0.02, (charCenter.y - charEye.y) * 0.02, (charCenter.z - charEye.z) * 0.02);
            charCenter = charCenter + vec3((charCenter.x - oldEye.x) * 0.02, (charCenter.y - oldEye.y) * 0.02, (charCenter.z - oldEye.z) * 0.02);
            if (charMode)
            {
                eye = charEye;
                center = charCenter;
            }
			printf("Up arrow is pressed at (%d, %d)\n", x, y);
			break;
		case 's':
            if (motivation == 0)
            {
                motivation = 2;
            }
            charTrans = charTrans - vec3((charCenter.x - charEye.x) * 0.02, (charCenter.y - charEye.y) * 0.02, (charCenter.z - charEye.z) * 0.02);
            charEye = charEye - vec3((charCenter.x - charEye.x) * 0.02, (charCenter.y - charEye.y) * 0.02, (charCenter.z - charEye.z) * 0.02);
            charCenter = charCenter - vec3((charCenter.x - oldEye.x) * 0.02, (charCenter.y - oldEye.y) * 0.02, (charCenter.z - oldEye.z) * 0.02);
            if (charMode)
            {
                eye = charEye;
                center = charCenter;
            }
			printf("Down arrow is pressed at (%d, %d)\n", x, y);
			break;
        case 'i':
			printf("Eye: %f, %f, %f\n", eye[0], eye[1], eye[2]);
			printf("Center: %f, %f, %f\n", center[0], center[1], center[2]);
			printf("Up: %f, %f, %f\n", up[0], up[1], up[2]);
			break;
        case 'j':
            jump = 0;
            break;
        case 27: // Esc
            exit(0);
        default:
			break;
    }
    printf("Key %c is pressed at (%d, %d)\n", key, x, y);
}

void My_SpecialKeys(int key, int x, int y)
{
    vec3 move_lr = normalize(cross(center - eye, up));
    vec3 move_fb = normalize(center - eye);
    move_lr = vec3(move_lr[0] / 40.0, move_lr[1] / 40.0, move_lr[2] / 40.0);
    move_fb = vec3(move_fb[0] / 40.0, move_fb[1] / 40.0, move_fb[2] / 40.0);
    
    switch(key)
    {
        case GLUT_KEY_LEFT:
            eye = eye - move_lr;
            center = center - move_lr;
			printf("Left arrow is pressed at (%d, %d)\n", x, y);
			break;
        case GLUT_KEY_RIGHT:
            eye = eye + move_lr;
            center = center + move_lr;
			printf("Right arrow is pressed at (%d, %d)\n", x, y);
			break;
        case GLUT_KEY_UP:
            eye = eye + move_fb;
            center = center + move_fb;
			printf("Up arrow is pressed at (%d, %d)\n", x, y);
			break;
        case GLUT_KEY_DOWN:
            eye = eye - move_fb;
            center = center - move_fb;
			printf("Down arrow is pressed at (%d, %d)\n", x, y);
			break;
        default:
			printf("Other special key is pressed at (%d, %d)\n", x, y);
			break;
    }
}

void My_Menu(int id)
{
    switch(id)
    {
        case MENU_TIMER_START:
			if(!timer_enabled)
			{
				timer_enabled = true;
				glutTimerFunc(timer_speed, My_Timer, 0);
			}
			break;
        case MENU_TIMER_STOP:
			timer_enabled = false;
			break;
        case MENU_IDLE:
			glUniform1i(mode, 0);
            fbo_mode = 0;
			break;
        case MENU_ABSTRACT:
			glUniform1i(mode, 1);
            fbo_mode = 1;
			break;
        case MENU_COMPARE:
			glUniform1i(compare, 1);
			break;
        case MENU_SAME:
			glUniform1i(compare, 0);
			break;
        case MENU_STEREO:
			glUniform1i(mode, 2);
            fbo_mode = 2;
			break;
        case MENU_PIXEL:
			glUniform1i(mode, 3);
            fbo_mode = 3;
			break;
        case MENU_LAPLACIAN:
			glUniform1i(mode, 4);
            fbo_mode = 4;
			break;
        case MENU_BLOOM:
			glUniform1i(mode, 5);
            fbo_mode = 5;
			break;
        // case MENU_MAGNIFIER:
		// 	   glUniform1i(mode, 6);
        //     fbo_mode = 6;
		//     break;
        case MENU_NOISE:
			glUniform1i(mode, 7);
            fbo_mode = 7;
			break;
        case MENU_TWIST:
			glUniform1i(mode, 8);
            fbo_mode = 8;
			break;
        case MENU_POSTERIZATION:
            glUniform1i(mode, 9);
            fbo_mode = 9;
            break;
        case MENU_EXIT:
			exit(0);
			break;
        default:
			break;
    }
}

int main(int argc, char *argv[])
{
#ifdef __APPLE__
    // Change working directory to source code path
    chdir(__FILEPATH__("/../Assets/"));
#endif
    // Initialize GLUT and GLEW, then create a window.
    ////////////////////
    glutInit(&argc, argv);
#ifdef _MSC_VER
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#endif
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Final Project"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
#ifdef _MSC_VER
    glewInit();
#endif
    // glPrintContextInfo();
    My_Init();
    
    // Create a menu and bind it to mouse right button.
    int menu_main = glutCreateMenu(My_Menu);
    int menu_compare = glutCreateMenu(My_Menu);
    
    glutSetMenu(menu_main);
    glutAddSubMenu("Comparison Bar", menu_compare);
    glutAddMenuEntry("Original", MENU_IDLE);
    glutAddMenuEntry("Abstraction", MENU_ABSTRACT);
    glutAddMenuEntry("Red-Blue Stereo", MENU_STEREO);
    glutAddMenuEntry("Pixelization", MENU_PIXEL);
    glutAddMenuEntry("Sharpen Filter", MENU_LAPLACIAN);
    glutAddMenuEntry("Bloom Effect", MENU_BLOOM);
    // glutAddMenuEntry("Magnifier", MENU_MAGNIFIER);
    glutAddMenuEntry("Add Noise", MENU_NOISE);
    glutAddMenuEntry("Twist Effect", MENU_TWIST);
    glutAddMenuEntry("Posterization", MENU_POSTERIZATION);
    glutAddMenuEntry("Exit", MENU_EXIT);
    
    glutSetMenu(menu_compare);
    glutAddMenuEntry("On", MENU_COMPARE);
    glutAddMenuEntry("Off", MENU_SAME);
    
    glutSetMenu(menu_main);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    
    // Register GLUT callback functions.
    glutDisplayFunc(My_Display);
    glutReshapeFunc(My_Reshape);
    glutMouseFunc(My_Mouse);
    glutMotionFunc(My_MouseMotion);
    glutKeyboardFunc(My_Keyboard);
    glutSpecialFunc(My_SpecialKeys);
    glutTimerFunc(timer_speed, My_Timer, 0);
    
    // Enter main event loop.
    glutMainLoop();
    
    return 0;
}

