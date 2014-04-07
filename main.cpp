/* Written by Jeffrey Chastine, 2012 */
/* Lazily edited by Mark Elsinger 2014 */
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <string>
#include <fstream>

typedef unsigned char Uint8; //close enough

#define BUFFER_OFFSET(i) ((char *)NULL + (i))
GLuint shaderProgramID;
GLuint vao = 0;
GLuint vbo;
GLuint positionID, colorID, texposID, textureID, samplerID;

#pragma region SHADER_FUNCTIONS
static char* readFile(const char* filename) {
	/*// Open the file
	FILE* fp = fopen (filename, "r");
	// Move the file pointer to the end of the file and determing the length
	fseek(fp, 0, SEEK_END);
	long file_length = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* contents = new char[file_length+1];*/

	//custom
	std::ifstream in(filename);
	// Move the file pointer to the end of the file and determing the length
	in.seekg(0, std::ios::end);
	long file_length = (long)in.tellg();
	in.clear();
	in.seekg(0, std::ios::beg);
	char* contents = new char[file_length + 1];

	// zero out memory
	for (int i = 0; i < file_length + 1; i++) {
		contents[i] = 0;
	}
	// Here's the actual read
	//fread (contents, 1, file_length, fp);
	in.read(contents, file_length);
	// This is how you denote the end of a string in C
	contents[file_length + 1] = '\0';


	//if (!in.good()) { printf("bad file"); } //debug line
	//fclose(fp);
	in.close();
	return contents;
}

bool compiledStatus(GLint shaderID){
	GLint compiled = 0;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compiled);
	if (compiled) {
		return true;
	}
	else {
		GLint logLength;
		glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
		char* msgBuffer = new char[logLength];
		glGetShaderInfoLog(shaderID, logLength, NULL, msgBuffer);
		printf("%s\n", msgBuffer);
		delete (msgBuffer);
		return false;
	}
}

GLuint makeVertexShader(const char* shaderSource) {
	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(vertexShaderID);
	bool compiledCorrectly = compiledStatus(vertexShaderID);
	if (compiledCorrectly) {
		return vertexShaderID;
	}
	return -1;
}

GLuint makeFragmentShader(const char* shaderSource) {
	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, (const GLchar**)&shaderSource, NULL);
	glCompileShader(fragmentShaderID);
	bool compiledCorrectly = compiledStatus(fragmentShaderID);
	if (compiledCorrectly) {
		return fragmentShaderID;
	}
	return -1;
}

GLuint makeShaderProgram(GLuint vertexShaderID, GLuint fragmentShaderID) {
	GLuint shaderID = glCreateProgram();
	glAttachShader(shaderID, vertexShaderID);
	glAttachShader(shaderID, fragmentShaderID);
	glLinkProgram(shaderID);
	return shaderID;
}
#pragma endregion SHADER_FUNCTIONS

//kinda need windows for this bit. easily ported to linux though, just need libraries...
//also nabbed this from http://www.cplusplus.com/articles/GwvU7k9E/, thanks Fredbill =]
int loadBMP(const char* location, GLuint *texture)
{
	Uint8* datBuff[2] = { nullptr, nullptr }; // Header buffers

	Uint8* pixels = nullptr; // Pixels

	BITMAPFILEHEADER* bmpHeader = nullptr; // Header
	BITMAPINFOHEADER* bmpInfo = nullptr; // Info 

	// The file... We open it with it's constructor
	std::ifstream file(location, std::ios::binary);
	if (!file)
	{
		printf("Failure to open bitmap file.\n");

		return 1;
	}

	// Allocate byte memory that will hold the two headers
	datBuff[0] = new Uint8[sizeof(BITMAPFILEHEADER)];
	datBuff[1] = new Uint8[sizeof(BITMAPINFOHEADER)];

	file.read((char*)datBuff[0], sizeof(BITMAPFILEHEADER));
	file.read((char*)datBuff[1], sizeof(BITMAPINFOHEADER));

	// Construct the values from the buffers
	bmpHeader = (BITMAPFILEHEADER*)datBuff[0];
	bmpInfo = (BITMAPINFOHEADER*)datBuff[1];

	// Check if the file is an actual BMP file
	if (bmpHeader->bfType != 0x4D42)
	{
		printf("File \"%s\" isn't a bitmap file\n", location);
		return 2;
	}

	// First allocate pixel memory
	pixels = new Uint8[bmpInfo->biSizeImage];

	// Go to where image data starts, then read in image data
	file.seekg(bmpHeader->bfOffBits);
	file.read((char*)pixels, bmpInfo->biSizeImage);

	// We're almost done. We have our image loaded, however it's not in the right format.
	// .bmp files store image data in the BGR format, and we have to convert it to RGB.
	// Since we have the value in bytes, this shouldn't be to hard to accomplish
	Uint8 tmpRGB = 0; // Swap buffer
	for (unsigned long i = 0; i < bmpInfo->biSizeImage; i += 3)
	{
		tmpRGB = pixels[i];
		pixels[i] = pixels[i + 2];
		pixels[i + 2] = tmpRGB;
	}

	// Set width and height to the values loaded from the file
	GLuint w = bmpInfo->biWidth;
	GLuint h = bmpInfo->biHeight;

	/*******************GENERATING TEXTURES*******************/

	//enable that texturing?
	//glEnable(GL_TEXTURE_2D);
	//glActiveTexture(GL_TEXTURE0);

	glGenTextures(1, texture);             // Generate a texture
	glBindTexture(GL_TEXTURE_2D, *texture); // Bind that texture temporarily

	GLint mode = GL_RGB;                   // Set the mode

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// Create the texture. We get the offsets from the image, then we use it with the image's
	// pixel data to create it.
	glTexImage2D(GL_TEXTURE_2D, 0, mode, w, h, 0, mode, GL_UNSIGNED_BYTE, pixels);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, NULL);

	// Output a successful message
	printf("Texture \"%s\" successfully loaded.\n", location);

	// Delete the two buffers.
	delete[] datBuff[0];
	delete[] datBuff[1];
	delete[] pixels;

	return 0; // Return success code 
}

// Any time the window is resized, this function gets called.  It's setup to the
// "glutReshapeFunc" in main.
void changeViewport(int w, int h){
	glViewport(0, 0, w, h);
}

// Here is the function that gets called each time the window needs to be redrawn.
// It is the "paint" method for our program, and is set up from the glutDisplayFunc in main
void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(samplerID, 0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glutSwapBuffers();
}

int main(int argc, char** argv) {
	// Standard stuff...
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Shaders");
	glutReshapeFunc(changeViewport);
	glutDisplayFunc(render);
	glewInit();

	// Vertices and colors of a triangle
	// notice, position values are between -1.0f and +1.0f
	/*GLfloat vertices[] = { -0.5f, -0.5f, 0.0f,	// Lower-left
		0.5f, -0.5f, 0.0f,				// Lower-right
		0.0f, 0.5f, 0.0f };				// Top
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f };*/

	//vertices of square
	GLfloat vertices[] = { 0.5f, 0.5f, 0.0f,	
		-0.5f, 0.5f, 0.0f,				
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f };				
	GLfloat colors[] = { 1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat texPos[] = { 1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f };

	// Make a shader
	char* vertexShaderSourceCode = readFile("vertexShader.glsl");
	char* fragmentShaderSourceCode = readFile("fragmentShader.glsl");
	GLuint vertShaderID = makeVertexShader(vertexShaderSourceCode);
	GLuint fragShaderID = makeFragmentShader(fragmentShaderSourceCode);
	shaderProgramID = makeShaderProgram(vertShaderID, fragShaderID);

	printf("vertShaderID is %d\n", vertShaderID);
	printf("fragShaderID is %d\n", fragShaderID);
	printf("shaderProgramID is %d\n", shaderProgramID);


	// Create the "remember all"
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Create the buffer, but don't load anything yet
	glBufferData(GL_ARRAY_BUFFER, 9 * 4 * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	// Load the vertex points
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * 4 * sizeof(GLfloat), vertices);
	// Load the colors right after that
	glBufferSubData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), 4 * 4 * sizeof(GLfloat), colors);
	// load the tex coords
	glBufferSubData(GL_ARRAY_BUFFER, 7 * 4 * sizeof(GLfloat), 2 * 4 * sizeof(GLfloat), texPos);

	//texture stuff
	loadBMP("breaddog.bmp", &textureID);
	samplerID = glGetUniformLocation(shaderProgramID, "texSampler");
	printf("textureID is %d\n", textureID);
	printf("samplerID is %d\n", samplerID);

	// Find the position of the variables in the shader
	positionID = glGetAttribLocation(shaderProgramID, "s_vPosition");
	colorID = glGetAttribLocation(shaderProgramID, "s_vColor");
	texposID = glGetAttribLocation(shaderProgramID, "s_vTexPos");
	printf("s_vPosition's ID is %d\n", positionID);
	printf("s_vColor's ID is %d\n", colorID);
	printf("s_vTexPos's ID is %d\n", texposID);

	glVertexAttribPointer(positionID, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(3 * 4 * sizeof(GLfloat)));
	//glVertexAttribPointer(colorID, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glVertexAttribPointer(texposID, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(7 * 4 * sizeof(GLfloat)));
	glUseProgram(shaderProgramID);
	glEnableVertexAttribArray(positionID);
	glEnableVertexAttribArray(colorID);
	glEnableVertexAttribArray(textureID);


	glutMainLoop();

	return 0;
}
