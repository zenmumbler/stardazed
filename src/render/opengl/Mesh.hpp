// ------------------------------------------------------------------
// render::opengl::Mesh - stardazed
// (c) 2015 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_RENDER_OPENGL_MESH_H
#define SD_RENDER_OPENGL_MESH_H

#include "system/Config.hpp"
#include "util/ConceptTraits.hpp"
#include "render/common/Mesh.hpp"
#include "render/opengl/Buffer.hpp"

namespace stardazed {
namespace render {


class Mesh {
	GLuint glVAO_ = 0;
	std::vector<Buffer> buffers_;

public:
	Mesh();
	Mesh(const MeshDescriptor&);
	~Mesh();
	SD_DEFAULT_MOVE_OPS(Mesh)
	
	void initWithDescriptor(const MeshDescriptor&);
	
	// -- observers
	GLuint name() const { return glVAO_; }

	void bind() const {
		glBindVertexArray(glVAO_);
	}
};


// ---- Mesh binding specializations

template <>
inline GLuint saveAndBind(const Mesh& mesh) {
	GLuint currentlyBound;
	glGetIntegerv(GL_VERTEX_ARRAY_BINDING, reinterpret_cast<GLint*>(&currentlyBound));
	if (currentlyBound != mesh.name())
		mesh.bind();
	
	return currentlyBound;
}

template <>
inline void unbindAndRestore(const Mesh& mesh, GLuint savedVAOName) {
	if (savedVAOName != mesh.name()) {
		glBindVertexArray(savedVAOName);
	}
}


} // ns render
} // ns stardazed


#endif
