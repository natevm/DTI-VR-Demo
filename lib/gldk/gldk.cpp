#include "gldk.hpp"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <stdlib.h>

bool GLDK::Verbose = false;

bool GLDK::error = false;

GLFWwindow *GLDK::DefaultWindow;
GLFWmonitor *GLDK::DefaultMonitor;
int GLDK::MajorVersion = 4;
int GLDK::MinorVersion = 5;
bool GLDK::FullScreenActive = false;
bool GLDK::WindowHidden = false;
int GLDK::CurrentWindowSize[2] = { 512 , 512 };
int GLDK::CurrentWindowPos[2];
int GLDK::PreviousWindowSize[2] = { 512 , 512 };
int GLDK::PreviousWindowPos[2];
std::string GLDK::DefaultWindowTitle = "";
std::vector<std::pair<std::string, std::string>> GLDK::DefaultSources;
std::string GLDK::BuildOptions = "";
std::unordered_map<std::string, GLuint> GLDK::Shaders;

void GLDK::print(std::string s, bool forced) {
	if (Verbose || error || forced)
		printf("GLDK: %-70s\n", s.c_str());
}

/* Source file management */
std::string GLDK::loadFile(std::string name)
{
	std::ifstream t(name, std::ios::binary);
	std::string str;

	t.seekg(0, std::ios::end);
	str.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	str.assign((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());
	return str;
}

void GLDK::ErrorCallback(int _error, const char* description)
{
	error = _error;
	print("Error: " + std::string(description));
}

void APIENTRY openglCallbackFunction(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam) {
	/* Kinda kludgy, but nvidia reports many "other" things that tend to spam console.*/
	if (type == GL_DEBUG_TYPE_OTHER) return;

	using namespace std;
	cout << "---------------------opengl-callback-start------------" << endl;
	cout << "message: " << message << endl;
	cout << "type: ";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		cout << "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		cout << "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		cout << "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		cout << "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		cout << "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_OTHER:
		cout << "OTHER";
		break;
	}
	cout << endl;

	cout << "id: " << id << endl;
	cout << "severity: ";
	switch (severity) {
	case GL_DEBUG_SEVERITY_LOW:
		cout << "LOW";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		cout << "MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		cout << "HIGH";
		break;
	}
	cout << endl;
	cout << "---------------------opengl-callback-end--------------" << endl;
}

/* Initializers */
bool GLDK::Initialize(InitializationParameters parameters) {
	CurrentWindowSize[0] = parameters.windowWidth;
	CurrentWindowSize[1] = parameters.windowHeight;
	DefaultWindowTitle = parameters.windowTitle;
	FullScreenActive = parameters.fullScreenActive;
	WindowHidden = parameters.windowHidden;
	MajorVersion = parameters.GLMajorVersion;
	MinorVersion = parameters.GLMinorVersion;

	if (!glfwInit()) {
		print("GLFW failed to initialize!", true);
		return false;
	}
	glfwSetErrorCallback(GLDK::ErrorCallback);

	
	GLFWmonitor *monitor;
	monitor = NULL;

	DefaultMonitor = glfwGetPrimaryMonitor();
	CreateNewWindow(CurrentWindowSize[0], CurrentWindowSize[1], DefaultWindowTitle, NULL, NULL);
	MakeContextCurrent(DefaultWindow);
	if (LoadFunctionPointers() != 1) {
		return false;
	};
	SetFullScreen(FullScreenActive, CurrentWindowPos, CurrentWindowSize);
	SetWindowHidden(WindowHidden);

	/* Changing this, now material components load/compile their own shaders */
	///* Get a list of all OpenGL Shaders to build*/
	//getSources(DefaultSources);
	//if (DefaultSources.size() == 0) return error;

	//buildShaders(Shaders, DefaultSources, BuildOptions);

	
#if _DEBUG
	if (glDebugMessageCallback) {
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, nullptr);
		GLuint unusedIds = 0;
		glDebugMessageControl(GL_DONT_CARE,
			GL_DONT_CARE,
			GL_DONT_CARE,
			0,
			&unusedIds,
			true);
	}
#endif

	return true;
}

void GLDK::buildShaders(std::unordered_map<std::string, GLuint> &shaders, std::vector<std::pair<std::string, std::string>> sources, std::string buildOptions) {
	for (int i = 0; i < sources.size(); ++i) {
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		const GLchar *myshader = sources[i].second.c_str();
		const GLchar *shaderversion = "#version 410\n";
		const GLchar *vsdefines = "#define VERTEX_SHADER\n";
		const GLchar *fsdefines = "#define FRAGMENT_SHADER\n";
		const GLchar *vshader[3] = { shaderversion, vsdefines,  myshader};
		const GLchar *fshader[3] = { shaderversion, fsdefines, myshader };

		glShaderSource(vertexShader, 3, vshader, 0);
		glShaderSource(fragmentShader, 3, fshader, 0);

		/* compile vshader and fshader */
		glCompileShader(vertexShader);
		glCompileShader(fragmentShader);

		GLint isCompiled = 0;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(vertexShader);

			// Use the infoLog as you see fit.
			std::cout << "In vertex shader " << sources[i].first << std::endl;
			printf("%s", infoLog.data());
			continue;
		}

		isCompiled = 0;
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
		if (isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);

			// We don't need the shader anymore.
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			std::cout << "In fragment shader " << sources[i].first << std::endl;
			printf("%s", infoLog.data());
			continue;
		}


		// Vertex and fragment shaders are successfully compiled.
		// Now time to link them together into a program.
		// Get a program object.
		GLuint program = glCreateProgram();

		// Attach our shaders to our program
		glAttachShader(program, vertexShader);
		glAttachShader(program, fragmentShader);

		// Link our program
		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);
			// Don't leak shaders either.
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);

			// Use the infoLog as you see fit.
			std::cout << "In linking shader " << sources[i].first << std::endl;
			printf("%s", infoLog.data());

			// In this simple program, we'll just leave
			continue;
		}

		shaders.emplace(sources[i].first, program);

		// Always detach shaders after a successful link.
		glDetachShader(program, vertexShader);
		glDetachShader(program, fragmentShader);
	}
}

std::string getFileName(const std::string& s) {
	using namespace std;
	char sep = '/';

	/* cmake 
#ifdef _WIN32
	sep = '\\';
#endif*/

	size_t i = s.rfind(sep, s.length());
	if (i != std::string::npos) {
		string fullname = s.substr(i + 1, s.length() - i);
		size_t lastindex = fullname.find_last_of(".");
		string rawname = fullname.substr(0, lastindex);

		return(rawname);
	}

	return("");
}

void GLDK::getSources(std::vector<std::pair<std::string, std::string>> &sources) {
	using namespace std;
	ifstream sourceList;
	sourceList.open(OPENGL_SOURCES_PATH);
	while (sourceList) {
		string pathname;
		getline(sourceList, pathname);
		if (pathname.size() == 0) continue;
		string filename = getFileName(pathname);
		print("adding " + string(filename) + " to shader sources.");

		string source = loadFile(pathname);
		pair<string, string> shaderEntry;
		shaderEntry.first = filename;
		shaderEntry.second = source;
		sources.push_back(shaderEntry);
	}

	if (sources.size() == 0) print("Warning: no glsl source files provided.");
}

void GLDK::Terminate() {
	glfwTerminate();
}

int GLDK::CreateNewWindow(int width, int height, std::string title, GLFWmonitor *monitor, GLFWwindow *share) {
	

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, MajorVersion);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, MinorVersion);
#if _DEBUG
//	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
	DefaultWindow = glfwCreateWindow(width, height, title.c_str(), monitor, share);
	if (!DefaultWindow) {
		return -1;
	}
	return 0;
}

int GLDK::LoadFunctionPointers() {
	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		print("GLEW initialization failed...", true);
		return -1;
	}
	return true;
}

void GLDK::MakeContextCurrent(GLFWwindow *window) {
	glfwMakeContextCurrent(window);
}

bool GLDK::ShouldClose() {
	return glfwWindowShouldClose(DefaultWindow);
}

void GLDK::SetFullScreen(bool fullscreen, int windowPos[2], int windowSize[2]) {
	if (!FullScreenActive && fullscreen == false) return;

	FullScreenActive = fullscreen;
	if (fullscreen)
	{
		// backup windwo position and window size
		glfwGetWindowPos(DefaultWindow, &windowPos[0], &windowPos[1]);
		glfwGetWindowSize(DefaultWindow, &windowSize[0], &windowSize[1]);

		// get reolution of monitor
		const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		
		CurrentWindowSize[0] = mode->width;
		CurrentWindowSize[1] = mode->height;

		// switch to full screen
		glfwSetWindowMonitor(DefaultWindow, DefaultMonitor, 0, 0, mode->width, mode->height, 0);
	}
	else
	{
		CurrentWindowSize[0] = windowSize[0];
		CurrentWindowSize[1] = windowSize[1];

		// restore last window size and position
		glfwSetWindowMonitor(DefaultWindow, nullptr, windowPos[0], windowPos[1], windowSize[0], windowSize[1], 0);
	}
	glViewport(0, 0, CurrentWindowSize[0], CurrentWindowSize[1]);

/*
	_updateViewport = true;*/
}

void GLDK::SetWindowHidden(bool hideWindow) {
	if (!WindowHidden && hideWindow == false) return;
	WindowHidden = hideWindow;
	if (hideWindow) {
		glfwHideWindow(DefaultWindow);
	}
	else {
		glfwShowWindow(DefaultWindow);
	}
}