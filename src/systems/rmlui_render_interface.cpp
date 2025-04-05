#include "rmlui_render_interface.hpp"
#include <iostream>
#include "stb_image.h"
#include "../common.hpp"  // For textures_path function

// Vertex shader
const char* vertex_shader_source = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 texcoord;
uniform vec2 translation;
uniform vec2 scale;
uniform vec2 offset;
uniform mat4 transform;
uniform float content_scale;
out vec4 v_color;
out vec2 v_texcoord;
void main() {
    // Apply translation in pixel space
    vec2 pos = position + translation;
    
    // Transform position by the transform matrix if provided
    vec4 transformed_pos = transform * vec4(pos, 0.0, 1.0);
    
    // Multiply by content_scale to convert from logical to physical pixels
    vec2 physical = transformed_pos.xy * content_scale;
    
    // Transform to NDC coordinates (simplified projection)
    vec2 ndc = physical * scale + offset;
    
    // Output final position
    gl_Position = vec4(ndc.x, ndc.y, 0.0, 1.0);
    
    // Pass through color and texture coordinates
    v_color = color;
    v_texcoord = texcoord;
}
)";

// Fragment shader
const char* fragment_shader_source = R"(
#version 330 core
in vec4 v_color;
in vec2 v_texcoord;
uniform sampler2D tex;
uniform bool has_texture;
out vec4 color;
void main() {
    if (has_texture)
        color = texture(tex, v_texcoord) * v_color;
    else
        color = v_color;
}
)";

// Forward declare a utility function to check for OpenGL errors
static void check_gl_error(const char* location);

// Compiled geometry structure
struct CompiledGeometry {
    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    int num_indices;
};

// Helper function to check for OpenGL errors
static void check_gl_error(const char* location) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL error at " << location << ": 0x" << std::hex << error << std::endl;
    }
}

RmlUiRenderInterface::RmlUiRenderInterface()
    : m_vao(0), m_vbo(0), m_ibo(0), m_shader_program(0),
      m_content_scale(1.0f),
      m_transform(Rml::Matrix4f::Identity()), m_transform_dirty(true)
{
    // Create shaders
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    check_gl_error("glCreateShader (vertex)");
    glShaderSource(vertex_shader, 1, &vertex_shader_source, nullptr);
    check_gl_error("glShaderSource (vertex)");
    glCompileShader(vertex_shader);
    check_gl_error("glCompileShader (vertex)");

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    check_gl_error("glCreateShader (fragment)");
    glShaderSource(fragment_shader, 1, &fragment_shader_source, nullptr);
    check_gl_error("glShaderSource (fragment)");
    glCompileShader(fragment_shader);
    check_gl_error("glCompileShader (fragment)");

    // Create program
    m_shader_program = glCreateProgram();
    check_gl_error("glCreateProgram");
    glAttachShader(m_shader_program, vertex_shader);
    check_gl_error("glAttachShader (vertex)");
    glAttachShader(m_shader_program, fragment_shader);
    check_gl_error("glAttachShader (fragment)");
    glLinkProgram(m_shader_program);
    check_gl_error("glLinkProgram");
    
    // Verify shader compilation and program linking
    GLint success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
    check_gl_error("glGetShaderiv (vertex)");
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(vertex_shader, 512, nullptr, info_log);
        std::cerr << "Vertex shader compilation failed:\n" << info_log << std::endl;
    }
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
    check_gl_error("glGetShaderiv (fragment)");
    if (!success) {
        GLchar info_log[512];
        glGetShaderInfoLog(fragment_shader, 512, nullptr, info_log);
        std::cerr << "Fragment shader compilation failed:\n" << info_log << std::endl;
    }
    
    glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
    check_gl_error("glGetProgramiv");
    if (!success) {
        GLchar info_log[512];
        glGetProgramInfoLog(m_shader_program, 512, nullptr, info_log);
        std::cerr << "Shader program linking failed:\n" << info_log << std::endl;
    }
    
    // Delete shaders as they're linked into the program now
    glDeleteShader(vertex_shader);
    check_gl_error("glDeleteShader (vertex)");
    glDeleteShader(fragment_shader);
    check_gl_error("glDeleteShader (fragment)");
}

RmlUiRenderInterface::~RmlUiRenderInterface()
{
    glDeleteProgram(m_shader_program);
}

Rml::CompiledGeometryHandle RmlUiRenderInterface::CompileGeometry(Rml::Span<const Rml::Vertex> vertices, 
                                                                 Rml::Span<const int> indices)
{
    CompiledGeometry* geometry = new CompiledGeometry();
    
    // Create VAO, VBO, IBO
    glGenVertexArrays(1, &geometry->vao);
    check_gl_error("glGenVertexArrays");
    
    glGenBuffers(1, &geometry->vbo);
    check_gl_error("glGenBuffers (vbo)");
    
    glGenBuffers(1, &geometry->ibo);
    check_gl_error("glGenBuffers (ibo)");
    
    // Bind VAO first
    glBindVertexArray(geometry->vao);
    check_gl_error("glBindVertexArray");
    
    // Set up VBO
    glBindBuffer(GL_ARRAY_BUFFER, geometry->vbo);
    check_gl_error("glBindBuffer (array buffer)");
    
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Rml::Vertex), vertices.data(), GL_STATIC_DRAW);
    check_gl_error("glBufferData (array buffer)");
    
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, position));
    check_gl_error("glVertexAttribPointer (position)");
    glEnableVertexAttribArray(0);
    check_gl_error("glEnableVertexAttribArray (position)");
    
    // Color
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, colour));
    check_gl_error("glVertexAttribPointer (color)");
    glEnableVertexAttribArray(1);
    check_gl_error("glEnableVertexAttribArray (color)");
    
    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Rml::Vertex), (void*)offsetof(Rml::Vertex, tex_coord));
    check_gl_error("glVertexAttribPointer (texcoord)");
    glEnableVertexAttribArray(2);
    check_gl_error("glEnableVertexAttribArray (texcoord)");
    
    // Set up IBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, geometry->ibo);
    check_gl_error("glBindBuffer (element array buffer)");
    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);
    check_gl_error("glBufferData (element array buffer)");
    
    geometry->num_indices = (int)indices.size();
    
    // Unbind VAO but keep element buffer bound to it
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    check_gl_error("glBindBuffer (unbind array buffer)");
    
    glBindVertexArray(0);
    check_gl_error("glBindVertexArray (unbind)");
    
    return (Rml::CompiledGeometryHandle)geometry;
}

void RmlUiRenderInterface::RenderGeometry(Rml::CompiledGeometryHandle geometry, 
                                         Rml::Vector2f translation, 
                                         Rml::TextureHandle texture)
{
    // Immediately return for null geometry
    if (!geometry) {
        return;
    }
    
    CompiledGeometry* compiled_geometry = (CompiledGeometry*)geometry;
    
    // Clear any existing OpenGL errors
    while (glGetError() != GL_NO_ERROR) {}
    
    // Use our shader program
    glUseProgram(m_shader_program);
    
    // Bind vertex array
    glBindVertexArray(compiled_geometry->vao);
    
    // Get viewport dimensions for creating the projection matrix
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Create a simplified orthographic projection matrix:
    // - X goes from [0, width] to [-1, 1]
    // - Y goes from [0, height] to [1, -1] (flipped Y-axis)
    float width = (float)viewport[2];
    float height = (float)viewport[3];
    
    // Skip creating full matrix - just directly calculate scale and offset
    float x_scale = 2.0f / width;
    float y_scale = -2.0f / height;
    float x_offset = -1.0f;
    float y_offset = 1.0f;
    
    // Find shader uniforms
    GLint loc_translation = glGetUniformLocation(m_shader_program, "translation");
    GLint loc_scale = glGetUniformLocation(m_shader_program, "scale");
    GLint loc_offset = glGetUniformLocation(m_shader_program, "offset");
    GLint loc_content_scale = glGetUniformLocation(m_shader_program, "content_scale");
    GLint loc_has_texture = glGetUniformLocation(m_shader_program, "has_texture");
    GLint loc_tex = glGetUniformLocation(m_shader_program, "tex");
    GLint loc_transform = glGetUniformLocation(m_shader_program, "transform");
    
    // Set uniforms (checking for -1 to avoid OpenGL errors)
    if (loc_translation != -1) {
        glUniform2f(loc_translation, translation.x, translation.y);
    }
    
    if (loc_scale != -1) {
        glUniform2f(loc_scale, x_scale, y_scale);
    }
    
    if (loc_offset != -1) {
        glUniform2f(loc_offset, x_offset, y_offset);
    }
    
    if (loc_content_scale != -1) {
        glUniform1f(loc_content_scale, m_content_scale);
    }
    
    // Set transform matrix if dirty
    if (loc_transform != -1 && m_transform_dirty) {
        glUniformMatrix4fv(loc_transform, 1, GL_FALSE, m_transform.data());
        m_transform_dirty = false;
    }
    
    // Handle texture
    if (texture) {
        if (loc_has_texture != -1) {
            glUniform1i(loc_has_texture, 1);
        }
        
        if (loc_tex != -1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
            glUniform1i(loc_tex, 0);
        }
    } else if (loc_has_texture != -1) {
        glUniform1i(loc_has_texture, 0);
    }
    
    // Draw elements
    glDrawElements(GL_TRIANGLES, compiled_geometry->num_indices, GL_UNSIGNED_INT, 0);
    
    // Minimal cleanup (avoid unnecessary state changes)
    glBindVertexArray(0);
    
    // Clear any errors that might have occurred
    while (glGetError() != GL_NO_ERROR) {}
}

void RmlUiRenderInterface::ReleaseGeometry(Rml::CompiledGeometryHandle geometry)
{
    if (!geometry)
        return;
        
    CompiledGeometry* compiled_geometry = (CompiledGeometry*)geometry;
    
    glDeleteVertexArrays(1, &compiled_geometry->vao);
    glDeleteBuffers(1, &compiled_geometry->vbo);
    glDeleteBuffers(1, &compiled_geometry->ibo);
    
    delete compiled_geometry;
}

void RmlUiRenderInterface::EnableScissorRegion(bool enable)
{
    if (enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void RmlUiRenderInterface::SetScissorRegion(Rml::Rectanglei region)
{
    int left   = static_cast<int>(region.Left() * m_content_scale);
    int top    = static_cast<int>(region.Top() * m_content_scale);
    int width  = static_cast<int>(region.Width() * m_content_scale);
    int height = static_cast<int>(region.Height() * m_content_scale);
    glScissor(left, top, width, height);
}

Rml::TextureHandle RmlUiRenderInterface::LoadTexture(Rml::Vector2i& texture_dimensions,
                                                   const Rml::String& source)
{
    // Convert the source path to use our textures_path helper
    std::string fixed_path = textures_path(source);
    
    // Load image using stb_image
    int width, height, channels;
    unsigned char* image_data = stbi_load(fixed_path.c_str(), &width, &height, &channels, STBI_rgb_alpha);
    
    if (!image_data) {
        std::cerr << "Failed to load texture from " << fixed_path << ": " << stbi_failure_reason() << std::endl;
        return 0;
    }
    
    // Set output dimensions
    texture_dimensions.x = width;
    texture_dimensions.y = height;
    
    // Generate OpenGL texture using our existing method
    Rml::TextureHandle handle = GenerateTexture(
        Rml::Span<const Rml::byte>(image_data, width * height * 4),
        texture_dimensions
    );
    
    // Free the loaded image data
    stbi_image_free(image_data);
    
    return handle;
}

Rml::TextureHandle RmlUiRenderInterface::GenerateTexture(Rml::Span<const Rml::byte> source,
                                                        Rml::Vector2i source_dimensions)
{
    GLuint texture_id = 0;
    glGenTextures(1, &texture_id);
    
    if (texture_id == 0) {
        std::cerr << "Failed to generate OpenGL texture" << std::endl;
        return 0;
    }
    
    glBindTexture(GL_TEXTURE_2D, texture_id);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, source_dimensions.x, source_dimensions.y, 
                 0, GL_RGBA, GL_UNSIGNED_BYTE, source.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return (Rml::TextureHandle)texture_id;
}

void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle texture_handle)
{
    if (texture_handle) {
        GLuint texture_id = (GLuint)texture_handle;
        glDeleteTextures(1, &texture_id);
    }
}

// Helper method implementation
void RmlUiRenderInterface::RenderGeometryInternal(Rml::Span<const Rml::Vertex> vertices, 
                                                Rml::Span<const int> indices,
                                                Rml::TextureHandle texture,
                                                const Rml::Vector2f& translation)
{
    // Set up GL state
    glUseProgram(m_shader_program);
    glBindVertexArray(m_vao);
    
    // Upload vertex and index data
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Rml::Vertex), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_DYNAMIC_DRAW);
    
    // Set uniform for translation
    GLint translation_location = glGetUniformLocation(m_shader_program, "translation");
    glUniform2f(translation_location, translation.x, translation.y);
    
    // Set texture
    GLint has_texture_location = glGetUniformLocation(m_shader_program, "has_texture");
    if (texture) {
        glUniform1i(has_texture_location, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, (GLuint)texture);
    } else {
        glUniform1i(has_texture_location, 0);
    }
    
    // Draw
    glDrawElements(GL_TRIANGLES, (GLsizei)indices.size(), GL_UNSIGNED_INT, nullptr);
    
    // Clean up
    glBindVertexArray(0);
    glUseProgram(0);
    if (texture) {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void RmlUiRenderInterface::SetTransform(const Rml::Matrix4f* transform)
{
    if (transform) {
        m_transform = *transform;
    } else {
        m_transform = Rml::Matrix4f::Identity();
    }
    m_transform_dirty = true;
} 