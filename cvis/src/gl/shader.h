#pragma once
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include "deps/gl.h"

namespace gl {

struct Shader {
	typedef std::map<std::string, std::string> defines_t;
	unsigned int id;

	static void compileShader(const unsigned int shader, const std::string &filename, const defines_t &defines) {
		std::stringstream stream;

		stream << "#version 330 core" << std::endl;

		for (auto element: defines) {
			stream << "#define " << element.first << " " << element.second << std::endl;
		}

		std::ifstream file;
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		try {
			file.open(filename);
			stream << file.rdbuf();
			file.close();
		} catch (std::ifstream::failure &err) {
			fprintf(stderr, "Failed to read file %s\n", filename.c_str());
			exit(1);
		}
		
		std::string string = stream.str();
		const char *code = string.c_str();

		glShaderSource(shader, 1, &code, NULL);
		glCompileShader(shader);
		checkCompileErrors(shader, filename);
	}

	static void checkCompileErrors(const unsigned int shader, const std::string &filename) {
		GLint success;
		GLchar infoLog[1024];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			fprintf(stderr, "Shader compilation failed: %s\n%s\n", filename.c_str(), infoLog);
		}
	}

	static void checkLinkErrors(const unsigned int program) {
		GLint success;
		GLchar infoLog[1024];
		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, 1024, NULL, infoLog);
			fprintf(stderr, "Shader linking failed\n%s\n", infoLog);
		}
	}

	Shader(const char *vertexFilename, const char *geometryFilename, const char *fragmentFilename, const defines_t &defines) {
		id = glCreateProgram();

		unsigned int vertex, geometry, fragment;

		vertex = glCreateShader(GL_VERTEX_SHADER);
		compileShader(vertex, vertexFilename, defines);
		glAttachShader(id, vertex);

		if (geometryFilename != NULL) {
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			compileShader(geometry, geometryFilename, defines);
			glAttachShader(id, geometry);
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		compileShader(fragment, fragmentFilename, defines);
		glAttachShader(id, fragment);

		glLinkProgram(id);
		checkLinkErrors(id);

		glDeleteShader(vertex);
		if (geometryFilename != NULL) {
			glDeleteShader(geometry);
		}
		glDeleteShader(fragment);
	}

	Shader(const char *vertexFilename, const char *geometryFilename, const char *fragmentFilename)
	: Shader(vertexFilename, geometryFilename, fragmentFilename, defines_t()) {}

	Shader(const char *vertexFilename, const char *fragmentFilename, const defines_t &defines)
	: Shader(vertexFilename, NULL, fragmentFilename, defines) {}

	Shader(const char *vertexFilename, const char *fragmentFilename)
	: Shader(vertexFilename, NULL, fragmentFilename, defines_t()) {}

	void use() const { 
		glUseProgram(id); 
	}

	void setBool(const std::string &name, bool value) const {
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value); 
	}

	void setBoolArray(const std::string &name, size_t count, const bool *value) const {
		glUniform1iv(glGetUniformLocation(id, name.c_str()), count, (const int *)value); 
	}

	void setInt(const std::string &name, int value) const { 
		glUniform1i(glGetUniformLocation(id, name.c_str()), value); 
	}

	void setIntArray(const std::string &name, size_t count, const int *value) const { 
		glUniform1iv(glGetUniformLocation(id, name.c_str()), count, value); 
	}

	void setFloat(const std::string &name, float value) const { 
		glUniform1f(glGetUniformLocation(id, name.c_str()), value); 
	}

	void setFloatArray(const std::string &name, size_t count, const float *value) const {
		glUniform1fv(glGetUniformLocation(id, name.c_str()), count, value);
	}

	void setVec2(const std::string &name, const glm::vec2 &value) const { 
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}
	
	void setVec2(const std::string &name, float x, float y) const { 
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y); 
	}

	void setVec2Array(const std::string &name, size_t count, const glm::vec2 *value) const { 
		glUniform2fv(glGetUniformLocation(id, name.c_str()), count, (const float *)value); 
	}

	void setVec3(const std::string &name, const glm::vec3 &value) const { 
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}

	void setVec3(const std::string &name, float x, float y, float z) const { 
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z); 
	}

	void setVec3Array(const std::string &name, size_t count, const glm::vec3 *value) const { 
		glUniform3fv(glGetUniformLocation(id, name.c_str()), count, (const float *)value); 
	}

	void setVec4(const std::string &name, const glm::vec4 &value) const { 
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]); 
	}

	void setVec4(const std::string &name, float x, float y, float z, float w) const { 
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w); 
	}

	void setVec4Array(const std::string &name, size_t count, const glm::vec4 *value) const { 
		glUniform4fv(glGetUniformLocation(id, name.c_str()), count, (const float *)value); 
	}

	void setMat2(const std::string &name, const glm::mat2 &mat) const {
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setMat3(const std::string &name, const glm::mat3 &mat) const {
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setMat4(const std::string &name, const glm::mat4 &mat) const {
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
};

}
