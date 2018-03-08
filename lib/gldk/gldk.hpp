#pragma once 

/*
	BLOCK COMMENT HERE
*/

#define OPENGL_SOURCES_PATH "./opengl_sources.txt"

#define GLEW_STATIC

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif 

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_DEPTH_ZERO_TO_ONE
#endif

#include <GL/glew.h>
#ifdef __MAC__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <string>
#include <stdarg.h>
#include <vector>
#include <unordered_map>
#include <iostream>

class GLDK {
public:
	struct InitializationParameters {
		int windowWidth; 
		int windowHeight;
		std::string windowTitle;
		bool fullScreenActive;
		bool windowHidden;
		int GLMajorVersion;
		int GLMinorVersion;
	};

private:
	static void print(std::string s, bool forced = false);
	static std::string loadFile(std::string name);
	static void ErrorCallback(int _error, const char* description);
	static bool error;

public:
	static bool Verbose;
	static GLFWwindow *DefaultWindow;
	static GLFWmonitor *DefaultMonitor;
	static int MajorVersion;
	static int MinorVersion;
	static bool FullScreenActive;
	static bool WindowHidden;
	static int DefaultWindowSize[2];
	static int DefaultWindowPos[2];
	static int PreviousWindowSize[2];
	static int PreviousWindowPos[2];
	static std::string DefaultWindowTitle;
	static std::vector<std::pair<std::string, std::string>> DefaultSources;
	static std::string BuildOptions;

/* Maps*/
	static std::unordered_map<std::string, GLuint> Shaders;

	/* Initializers */
	static bool Initialize(InitializationParameters parameters);
	static void Terminate();

	static void MakeContextCurrent(GLFWwindow *window);
	static void SetFullScreen(bool fullscreen, int windowPos[2] = PreviousWindowPos, int windowSize[2] = PreviousWindowSize);
	static void SetWindowHidden(bool hidewindow);
	static bool ShouldClose();
	static int CreateNewWindow(int width, int height, std::string title, GLFWmonitor *monitor, GLFWwindow *share);
	static int LoadFunctionPointers();
	static void getSources(std::vector<std::pair<std::string, std::string>> &sources);
	static void buildShaders(std::unordered_map<std::string, GLuint> &shaders, std::vector<std::pair<std::string, std::string>> sources, std::string buildOptions);
	static std::string get_gl_error_msg(GLenum error) {
		std::string msg;

		switch (error) {
		case GL_NO_ERROR:
			msg = "GL_NO_ERROR - No error has been recorded. The value of this symbolic constant is guaranteed to be 0.";
			break;
		case GL_INVALID_ENUM:
			msg = "GL_INVALID_ENUM - An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_VALUE:
			msg = "GL_INVALID_VALUE - A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_INVALID_OPERATION:
			msg = "GL_INVALID_OPERATION - The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_STACK_OVERFLOW:
			msg = "GL_STACK_OVERFLOW - This command would cause a stack overflow. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_STACK_UNDERFLOW:
			msg = "GL_STACK_UNDERFLOW - This command would cause a stack underflow. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		case GL_OUT_OF_MEMORY:
			msg = "GL_OUT_OF_MEMORY - There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";
			break;
		case GL_TABLE_TOO_LARGE:
			msg = "GL_TABLE_TOO_LARGE - The specified table exceeds the implementation's maximum supported table size. The offending command is ignored and has no other side effect than to set the error flag.";
			break;
		default:
			msg = "Unrecognized error";
			break;
		}
		return msg;
	}
	static void print_error(const GLenum error, const char* file, int line_number) {
		using namespace std;
		if (error == GL_NO_ERROR) {
			return;
		}

		using namespace std;
		string msg = get_gl_error_msg(error);
		cout << file << " line " << line_number << ": glError: " << msg << endl;
	}
};

#ifndef print_gl_error
#define print_gl_error() { \
  GLenum error = glGetError(); \
  GLDK::print_error(error, __FILE__, __LINE__); \
}
#endif
