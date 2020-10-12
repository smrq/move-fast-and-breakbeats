#pragma once
#include <string>
#include "deps/gl.h"

namespace gl {

static void checkCompileErrors(GLuint shader, const std::string &type) {
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			fprintf(stderr, "Shader compilation failed: %s\n%s\n", type.c_str(), infoLog);
		}
	} else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			fprintf(stderr, "Shader linking failed: %s\n%s\n", type.c_str(), infoLog);
		}
	}
}

struct Shader {
	unsigned int id;

	Shader(const char *vertexCode, const char *geometryCode, const char *fragmentCode) {
		id = glCreateProgram();

		unsigned int vertex, geometry, fragment;

		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vertexCode, NULL);
		glCompileShader(vertex);
		checkCompileErrors(vertex, "VERTEX");
		glAttachShader(id, vertex);

		if (geometryCode != NULL) {
			geometry = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderSource(geometry, 1, &geometryCode, NULL);
			glCompileShader(geometry);
			checkCompileErrors(geometry, "GEOMETRY");
			glAttachShader(id, geometry);
		}

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fragmentCode, NULL);
		glCompileShader(fragment);
		checkCompileErrors(fragment, "FRAGMENT");
		glAttachShader(id, fragment);

		glLinkProgram(id);
		checkCompileErrors(id, "PROGRAM");

		glDeleteShader(vertex);
		if (geometryCode != NULL) {
			glDeleteShader(geometry);
		}
		glDeleteShader(fragment);
	}

	Shader(const char *vertexCode, const char *fragmentCode): Shader(vertexCode, NULL, fragmentCode) {}

	Shader(const std::string &vertexCode, const std::string &geometryCode, const std::string &fragmentCode)
	: Shader(vertexCode.c_str(), geometryCode.c_str(), fragmentCode.c_str()) {}

	Shader(const std::string &vertexCode, const std::string &fragmentCode)
	: Shader(vertexCode.c_str(), NULL, fragmentCode.c_str()) {}

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
