/*
 *	Author		: Jina Hyun
 *	Date		: 09/30/22
 *	File Name	: Shader.cpp
 *	Desc		: Create shader and shader program
 */
#include "Shader.h"

#include <iostream>	// std::cout
#include <fstream>	// std::ifstream
#include <gl/glew.h>	// gl functions

namespace
{
	constexpr GLenum ToGLenum(const ShaderType type)
	{
		switch (type)
		{
		case ShaderType::None: return GL_NONE;
		case ShaderType::Vertex: return GL_VERTEX_SHADER;
		case ShaderType::Fragment: return GL_FRAGMENT_SHADER;
		case ShaderType::Geometry: return GL_GEOMETRY_SHADER;
		case ShaderType::Tessellation_Control: return GL_TESS_CONTROL_SHADER;
		case ShaderType::Tessellation_Evaluation: return GL_TESS_EVALUATION_SHADER;
		}
		return GL_NONE;
	}
	
}

/* Shader - start -------------------------------------------------------------------------------*/

Shader::Shader(ShaderType type, const std::filesystem::path& file_path) noexcept
	: m_type(type), m_filePath(file_path)
{
	Compile();
}

Shader::~Shader() noexcept
{
	Clear();
}

void Shader::Compile() noexcept
{
	if (m_isCompiled)
		return;

	std::ifstream ifs(m_filePath, std::ios::in);
	if (!ifs.is_open())
	{
		std::cout << "[Shader]: Cannot open " + m_filePath.string() << std::endl;
		return;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	ifs.close();

	// create shader
	if (m_handle <= 0)
	{
		m_handle = glCreateShader(ToGLenum(m_type));
		if (m_handle == 0)
		{
			std::cout << "[Shader]: Unable to create a shader from " + m_filePath.string() << std::endl;
			return;
		}
	}

	// compile shader
	const std::string& contents = buffer.str();
	GLchar const* source[]{ contents.c_str() };
	glShaderSource(m_handle, 1, source, nullptr);
	glCompileShader(m_handle);

	// check compilation status
	GLint compiled = 0;
	glGetShaderiv(m_handle, GL_COMPILE_STATUS, &compiled);
	if (compiled == GL_FALSE)
	{
		GLint log_length = 0;
		glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetShaderInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[Shader]: <" << m_filePath << "> :Compile Failure" << std::endl;
		std::cout << std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	m_isCompiled = compiled;
}

void Shader::Clear() noexcept
{
	if (m_handle > 0)
	{
		glDeleteShader(m_handle);
		m_handle = 0;
		m_isCompiled = false;
	}
}

unsigned Shader::GetHandle() const noexcept
{
	return m_handle;
}

bool Shader::IsCompiled() const noexcept
{
	return m_isCompiled;
}

/* Shader - end ---------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------*/
/* ShaderProgram - start ------------------------------------------------------------------------*/

ShaderProgram::ShaderProgram(const std::vector<std::pair<ShaderType, std::filesystem::path>>& shader_files) noexcept
{
	for(const auto& shader : shader_files)
	{
		m_shader.push_back(new Shader(shader.first, shader.second));
	}
	LinkAndValidate();
}

ShaderProgram::~ShaderProgram() noexcept
{
	Clear();
	for (const auto& shader : m_shader)
		delete shader;
	m_shader.clear();
}

void ShaderProgram::Use() const noexcept
{
	glUseProgram(m_handle);
}

void ShaderProgram::UnUse() const noexcept
{
	glUseProgram(0);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, bool value) const noexcept
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, int value) const noexcept
{
	glUniform1i(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, float value) const noexcept
{
	glUniform1f(GetUniformLocation(uniform_name), value);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec2& value) const noexcept
{
	glUniform2f(GetUniformLocation(uniform_name), value.x, value.y);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec3& value) const noexcept
{
	glUniform3f(GetUniformLocation(uniform_name), value.x, value.y, value.z);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::vec4& value) const noexcept
{
	glUniform4fv(GetUniformLocation(uniform_name), 1, &value[0]);
}

void ShaderProgram::SendUniform(const std::string& uniform_name, const glm::mat4& value) const noexcept
{
	glUniformMatrix4fv(GetUniformLocation(uniform_name), 1, GL_FALSE, &value[0][0]);
}


void ShaderProgram::Clear() noexcept
{
	if (m_handle > 0)
	{
		glDeleteProgram(m_handle);
		m_handle = 0;
		m_isLinked = false;
	}
	uniforms.clear();
}

int ShaderProgram::GetUniformLocation(const std::string& uniform_name) const noexcept
{
	if (const auto find = uniforms.find(uniform_name); find != uniforms.end())
		return find->second;

	int location = glGetUniformLocation(m_handle, uniform_name.c_str());
	if (location < 0)
	{
		std::cout << "[ShaderProgram]: <tag=" << m_tag << "> There isn't uniform variable <" << uniform_name << ">" << std::endl;
		location = -1;
	}
	uniforms[uniform_name] = location;
	return location;
}

void ShaderProgram::LinkAndValidate() noexcept
{
	if (m_isLinked)
		return;

	if (m_handle <= 0)
	{
		m_handle = glCreateProgram();
		if (m_handle == 0)
		{
			std::cout << "[ShaderProgram]: <tag=" << m_tag << "> Unable to create shader program" << std::endl;
			return;
		}
	}

	for (const auto& shader : m_shader)
	{
		if (shader->IsCompiled() == false)
		{
			std::cout << "[ShaderProgram]: <tag=" << m_tag << "> shader isn't compiled" << std::endl;
			return;
		}
 		glAttachShader(m_handle, shader->GetHandle());
	}

	// verify link status
	glLinkProgram(m_handle);
	GLint linked = 0;
	glGetProgramiv(m_handle, GL_LINK_STATUS, &linked);
	if (linked == GL_FALSE)
	{
		GLint log_length = 0;
		glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetProgramInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[ShaderProgram]: <tag=" << m_tag << "> Link Failure\n" + std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	// Check validate
	glValidateProgram(m_handle);
	GLint is_validate = 0;
	glGetProgramiv(m_handle, GL_VALIDATE_STATUS, &is_validate);
	if (is_validate == GL_FALSE)
	{
		GLint log_length = 0;
		glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &log_length);
		std::vector<char> error_log(log_length);
		glGetProgramInfoLog(m_handle, log_length, nullptr, error_log.data());
		Clear();
		std::cout << "[ShaderProgram]: <tag=" << m_tag << "> Validate Failure\n" + std::string(error_log.begin(), error_log.end()) << std::endl;
		return;
	}
	m_isLinked = true;
}

void ShaderProgram::PrintActiveAttributes() const noexcept
{
	GLint max_length = 0, number = 0;
	glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTES, &number);
	if (number <= 0)
		return;
	glGetProgramiv(m_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);
	std::vector<char> attrib;
	std::cout << "Location\t| Attribute Name\n------------------------------------" << std::endl;
	for (GLint i = 0; i < number; ++i)
	{
		attrib.resize(max_length);
		GLint size; GLenum type;
		glGetActiveAttrib(m_handle, i, max_length, nullptr, &size, &type, attrib.data());
		std::string str(attrib.begin(), attrib.end());
		std::cout << glGetAttribLocation(m_handle, str.c_str()) << "\t\t" << str << std::endl;
		attrib.clear();
	}
	std::cout << std::endl;
}

void ShaderProgram::PrintActiveUniforms() const noexcept
{
	GLint max_length = 0, number = 0;
	glGetProgramiv(m_handle, GL_ACTIVE_UNIFORMS, &number);
	if (number <= 0)
		return;
	glGetProgramiv(m_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &max_length);
	std::vector<char> unifs;
	std::cout << "Location\t| Uniform Name\n------------------------------------" << std::endl;
	for (GLint i = 0; i < number; ++i)
	{
		unifs.resize(max_length);
		GLint size; GLenum type;
		glGetActiveUniform(m_handle, i, max_length, nullptr, &size, &type, unifs.data());
		const std::string str(unifs.begin(), unifs.end());
		std::cout << glGetUniformLocation(m_handle, str.c_str()) << "\t\t" << str << std::endl;
		unifs.clear();
	}
	std::cout << std::endl;
}