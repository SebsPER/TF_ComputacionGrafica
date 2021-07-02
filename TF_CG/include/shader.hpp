#ifndef __SHADER_H__
#define __SHADER_H__

#include <glutil.hpp>
#include <files.hpp>
#include <path.h>

class Shader {
public:
	Files* files;
	u32   pid;
	Path* path;

	Shader(Files* files, std::string vertexFileName, std::string fragmentFileName)
			: files(files) {
		path = new Path("bin", "resources/textures");
		std::ifstream vertexFile(files->shaderFile(vertexFileName));
		std::string vertexSrc;
		std::getline(vertexFile, vertexSrc, '\0');

		std::ifstream fragmentFile(files->shaderFile(fragmentFileName));
		std::string fragmentSrc;
		std::getline(fragmentFile, fragmentSrc, '\0');

		u32 vertex = mkShader(vertexSrc.c_str(), GL_VERTEX_SHADER);
		u32 fragment = mkShader(fragmentSrc.c_str(), GL_FRAGMENT_SHADER);

		pid = glCreateProgram();
		glAttachShader(pid, vertex);
		glAttachShader(pid, fragment);
		glLinkProgram(pid);
		glGetProgramiv(pid, GL_LINK_STATUS, &ok);
		if (!ok) {
			glGetProgramInfoLog(pid, 512, nullptr, infoLog);
			std::cout << "Error::shader::program::link_failed\n"
				<< infoLog << std::endl;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}
	~Shader() {
		glDeleteProgram(pid);
		glfwTerminate();
	}
	void use() {
		glUseProgram(pid);
	}

	// Set uniforms
	void setI32(const i8* name, const i32& i) const {
		glUniform1i(glGetUniformLocation(pid, name), i);
	}
	void setF32(const i8* name, const f32& f) const {
		glUniform1f(glGetUniformLocation(pid, name), f);
	}
	void setVec3(const i8* name, glm::vec3& vec) const {
		glUniform3fv(glGetUniformLocation(pid, name), 1, &vec[0]);
	}
	void setVec3(const i8* name, f32 a, f32 b, f32 c) const {
		glUniform3f(glGetUniformLocation(pid, name), a, b, c);
	}
	void setMat4(const i8* name, const glm::mat4& mat) const {
		glUniformMatrix4fv(glGetUniformLocation(pid,name), 1, GL_FALSE, &mat[0][0]);
	}

	// Texture loading
	u32 loadTexture(const std::string& textureFile,
		const std::string& uniformName = "",
		i32 param = GL_LINEAR) {
		u32 texture;
		std::string fileName = path->tp(textureFile);

		glGenTextures(1, &texture);

		i32 w, h, nrChannels;

		stbi_set_flip_vertically_on_load(true); // porque en opgl el eje Y invertio
		u8* data = stbi_load(fileName.c_str(), &w, &h, &nrChannels, 0);
		if (data != nullptr) {
			GLenum fmt;

			if (nrChannels == 4) {
				fmt = GL_RGBA;
			}
			else if (nrChannels == 3) {
				fmt = GL_RGB;
			}
			else {
				fmt = GL_RED;
			}

			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, param);
		}
		else {
			std::cerr << "Can't load texture\n";
			return -1;
		}
		stbi_image_free(data);
		/*use();
		if (uniformName != "") {
			setI32(uniformName.c_str(), texture);
		}
		//textures.push_back(texture);
		*/
		return texture;
	}

private:

	i32 ok;
	i8  infoLog[512];

	u32 mkShader(const i8* source, GLenum type) {
		u32 shader = glCreateShader(type);
		glShaderSource(shader, 1, &source, nullptr);
		glCompileShader(shader);
		glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
		if (!ok) {
			glGetShaderInfoLog(shader, 512, nullptr, infoLog);
			std::cerr << "Error: shader compilation failed\n" << infoLog << std::endl;
			return 0;
		}
		return shader;
	}
};

#endif

/* vim: set tabstop=2:softtabstop=2:shiftwidth=2:noexpandtab */

