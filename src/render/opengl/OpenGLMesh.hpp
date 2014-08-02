// ------------------------------------------------------------------
// render::OpenGLMesh - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_RENDER_OPENGLMESH_H
#define SD_RENDER_OPENGLMESH_H

#include "render/Mesh.hpp"
#include "render/opengl/OpenGLBuffer.hpp"
#include "util/ConceptTraits.hpp"

namespace stardazed {
namespace render {


class OpenGLMesh : public Mesh {
	GLsizei drawCount;
	GLuint vao_ {0};
	GLenum winding_;
	
	math::AABB aabb_;

	OpenGLBuffer<math::Vec3> vertexBuffer_, vertexNormalBuffer_;
	OpenGLBuffer<render::Tri, GL_ELEMENT_ARRAY_BUFFER> faceBuffer_;

	OpenGLBuffer<math::Vec3> vertexTangentBuffer_;
	OpenGLBuffer<math::Vec2> vertexUVBuffer_;

public:
	OpenGLMesh(const MeshDescriptor&);
	~OpenGLMesh();

	virtual math::AABB aabb() const override;
	virtual void draw() const override;
};


} // ns render
} // ns stardazed

#endif
