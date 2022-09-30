#include <string>
#include <gl/glew.h>
#include <glm/glm.hpp>
inline void Set(GLuint program_handle, std::string uniform_name, glm::mat4 mat)
{
    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, &(mat[0].x));

}

inline void Set(GLuint program_handle, std::string uniform_name, glm::vec4 vec)
{

    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform4fv(loc, 1, &vec.x);
}

inline void Set(GLuint program_handle, std::string uniform_name, glm::vec3 vec)
{
    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform3fv(loc, 1, &vec.x);

}
inline void Set(GLuint program_handle, std::string uniform_name, glm::vec2 vec)
{
    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform2fv(loc, 1, &vec.x);

}
inline void Set(GLuint program_handle, std::string uniform_name, float f)
{

    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform1f(loc, f);
}

inline void Set(GLuint program_handle, std::string uniform_name, int i)
{
    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform1i(loc, i);
}


inline void Set(GLuint program_handle, std::string uniform_name, bool b)
{
    GLint loc = glGetUniformLocation(program_handle, uniform_name.c_str());
    if (loc >= 0)
        glUniform1i(loc, b);
}
