#pragma once

#include <RmlUi/Core.h>
#include <gl3w/gl3w.h>

class RmlUiRenderInterface : public Rml::RenderInterface {
public:
    RmlUiRenderInterface();
    virtual ~RmlUiRenderInterface();

    // Called by RmlUi when it wants to render geometry.
    void RenderGeometry(Rml::CompiledGeometryHandle geometry, 
                        Rml::Vector2f translation, 
                        Rml::TextureHandle texture) override;

    // Compile geometry into a hardware buffer for efficient rendering
    Rml::CompiledGeometryHandle CompileGeometry(Rml::Span<const Rml::Vertex> vertices, 
                                               Rml::Span<const int> indices) override;

    // Release compiled geometry
    void ReleaseGeometry(Rml::CompiledGeometryHandle geometry) override;

    // Called by RmlUi when it wants to enable or disable scissoring.
    void EnableScissorRegion(bool enable) override;
    
    // Called by RmlUi when it wants to change the scissor region.
    void SetScissorRegion(Rml::Rectanglei region) override;

    // Called by RmlUi when a texture is required by the library.
    Rml::TextureHandle LoadTexture(Rml::Vector2i& texture_dimensions,
                                  const Rml::String& source) override;

    // Called by RmlUi when a texture is required to be built from an internally-generated sequence of pixels.
    Rml::TextureHandle GenerateTexture(Rml::Span<const Rml::byte> source,
                                      Rml::Vector2i source_dimensions) override;

    // Called by RmlUi when a loaded texture is no longer required.
    void ReleaseTexture(Rml::TextureHandle texture_handle) override;

private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ibo;
    GLuint m_shader_program;

    // Helper method to render geometry
    void RenderGeometryInternal(Rml::Span<const Rml::Vertex> vertices, 
                               Rml::Span<const int> indices,
                               Rml::TextureHandle texture,
                               const Rml::Vector2f& translation);
}; 