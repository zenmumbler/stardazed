// ------------------------------------------------------------------
// render::opengl::Buffer - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_RENDER_OPENGLBUFFER_H
#define SD_RENDER_OPENGLBUFFER_H

#include "system/Config.hpp"
#include "util/ConceptTraits.hpp"

#include "math/Vector.hpp"
#include "render/common/BufferStorage.hpp"
#include "render/common/Mesh.hpp"
#include "render/opengl/OpenGL.hpp"

#include <cassert>


namespace stardazed {
namespace render {


enum class BufferClientAccess {
	None,
	ReadOnly,
	WriteOnly,
	ReadWrite
};


enum class BufferUpdateFrequency {
	Never,
	Occassionally,
	Frequently
};


enum class GLArrayType {
	Attribute,
	Index
};


namespace detail {
	// internal buffer options to gl options
	constexpr GLbitfield glAccessFlagsForBCA(BufferClientAccess access) {
		switch (access) {
			case BufferClientAccess::ReadOnly:
				return GL_MAP_READ_BIT;
			case BufferClientAccess::WriteOnly:
				return GL_MAP_WRITE_BIT;
			case BufferClientAccess::ReadWrite:
				return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
			default:
				return 0;
		}
	}
	
	
	constexpr GLenum glUsageHint(BufferUpdateFrequency frequency, BufferClientAccess typicalAccess) {
		if (typicalAccess == BufferClientAccess::None) {
			switch (frequency) {
				case BufferUpdateFrequency::Never:
					return GL_STATIC_COPY;
				case BufferUpdateFrequency::Occassionally:
					return GL_DYNAMIC_COPY;
				case BufferUpdateFrequency::Frequently:
					return GL_STREAM_COPY;
			}
		}
		
		if (typicalAccess == BufferClientAccess::ReadWrite || typicalAccess == BufferClientAccess::WriteOnly) {
			switch (frequency) {
				case BufferUpdateFrequency::Never:
					return GL_STATIC_DRAW;
				case BufferUpdateFrequency::Occassionally:
					return GL_DYNAMIC_DRAW;
				case BufferUpdateFrequency::Frequently:
					return GL_STREAM_DRAW;
			}
		}
		
		if (typicalAccess == BufferClientAccess::ReadOnly) {
			switch (frequency) {
				case BufferUpdateFrequency::Never:
					return GL_STATIC_READ;
				case BufferUpdateFrequency::Occassionally:
					return GL_DYNAMIC_READ;
				case BufferUpdateFrequency::Frequently:
					return GL_STREAM_READ;
			}
		}
		
		assert(false && "Invalid frequency or access level");
	}
	
	
	constexpr GLenum glTargetForArrayType(GLArrayType type) {
		switch (type) {
			case GLArrayType::Attribute:
				return GL_ARRAY_BUFFER;
			case GLArrayType::Index:
				return GL_ELEMENT_ARRAY_BUFFER;
		}
		
		assert(false && "Invalid array type");
	}
	
	
	constexpr GLenum glBindingNameForTarget(GLenum target) {
		switch (target) {
			case GL_ARRAY_BUFFER:              return GL_ARRAY_BUFFER_BINDING;
			case GL_ELEMENT_ARRAY_BUFFER:      return GL_ELEMENT_ARRAY_BUFFER_BINDING;
			case GL_UNIFORM_BUFFER:            return GL_UNIFORM_BUFFER_BINDING;
			case GL_TRANSFORM_FEEDBACK_BUFFER: return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;
			case GL_PIXEL_PACK_BUFFER:         return GL_PIXEL_PACK_BUFFER_BINDING;
			case GL_PIXEL_UNPACK_BUFFER:       return GL_PIXEL_UNPACK_BUFFER;
			case GL_TEXTURE_BUFFER:            return GL_TEXTURE_BINDING_BUFFER;
				//		case GL_COPY_READ_BUFFER:          return GL_COPY_READ_BUFFER_BINDING;   not available in gl3.h?
				//		case GL_COPY_WRITE_BUFFER:         return GL_COPY_WRITE_BUFFER_BINDING;
			default:
				assert(false && "Unknown buffer target name");
		}
	}
}


class GLBuffer {
	GLuint name_ {0};
	GLenum target_ {0}, usage_ {0};
	size32_t byteSize_ {0};

public:
	GLBuffer(GLenum target, BufferUpdateFrequency frequency, BufferClientAccess access)
	: target_{ target }
	, usage_{ detail::glUsageHint(frequency, access) }
	{
		glGenBuffers(1, &name_);
	}

	GLBuffer(GLArrayType type, BufferUpdateFrequency frequency, BufferClientAccess access)
	: GLBuffer(detail::glTargetForArrayType(type), frequency, access)
	{}
	
	~GLBuffer() {
		if (name_)
			glDeleteBuffers(1, &name_);
	}

	// -- initialization

	void allocate(size32_t bytes, void* data) {
		byteSize_ = bytes;
		glBufferData(target_, bytes, data, usage_);
	}
	
	void allocate(size32_t bytes) {
		allocate(bytes, nullptr);
	}
	
	void allocate(const BufferStorage& storage) {
		allocate(static_cast<size32_t>(storage.byteSize()), storage.getAs<void*>());
	}

	// -- direct updates
	
	void write(size32_t bytes, void* data, size32_t offset) {
		glBufferSubData(target_, offset, bytes, data);
	}

	// -- memory mapped access

private:
	template <typename T>
	T* mapForUpdates(size32_t offset, size32_t bytes, GLbitfield flags) {
		assert(offset + bytes < byteSize_);
		return static_cast<T*>(glMapBufferRange(target_, offset, bytes, flags));
	}

public:
	template <typename T>
	const T* mapRangeForReading(size32_t offset, size32_t bytes) {
		assert(offset + bytes < byteSize_);
		return static_cast<const T*>(glMapBufferRange(target_, offset, bytes, GL_MAP_READ_BIT));
	}
	
	template <typename T>
	const T* mapBufferForReading() {
		return mapRangeForReading<T>(0, byteSize_);
	}
	
	template <typename T>
	T* mapRangeForWriting(size32_t offset, size32_t bytes) {
		return mapForUpdates<T>(offset, bytes, GL_MAP_WRITE_BIT);
	}
	
	template <typename T>
	T* mapBufferForWriting() {
		return mapRangeForWriting<T>(0, byteSize_);
	}
	
	template <typename T>
	T* invalidateAndMapRangeForWriting(size32_t offset, size32_t bytes) {
		return mapForUpdates<T>(offset, bytes, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
	}
	
	template <typename T>
	T* invalidateAndMapBufferForWriting() {
		return mapForUpdates<T>(0, byteSize_, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	}

	template <typename T>
	T* mapRangeForFullAccess(size32_t offset, size32_t bytes) {
		return mapForUpdates<T>(offset, bytes, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
	}
	
	template <typename T>
	T* mapBufferForFullAccess() {
		return mapRangeForFullAccess<T>(0, byteSize_);
	}
	
	// -- observers

	GLuint name() const { return name_; }
	GLenum target() const { return target_; }
	size32_t byteSize() const { return byteSize_; }
	
	// -- binding
	
	void bind() const { glBindBuffer(target_, name_); }
};


// ---- Buffer Binding Specializations

template <>
inline GLuint saveAndBind(const GLBuffer& buffer) {
	GLuint currentlyBound;
	glGetIntegerv(detail::glBindingNameForTarget(buffer.target()), reinterpret_cast<GLint*>(&currentlyBound));
	if (currentlyBound != buffer.name())
		buffer.bind();
	
	return currentlyBound;
}

template <>
inline void unbindAndRestore(const GLBuffer& buffer, GLuint savedBufferName) {
	if (savedBufferName != buffer.name()) {
		glBindBuffer(buffer.target(), savedBufferName);
	}
}


// ---- Indexed Buffers

namespace detail {

	constexpr GLenum maxBufferBindingsNameForTarget(GLenum target) {
		if (target == GL_UNIFORM_BUFFER)
			return GL_MAX_UNIFORM_BUFFER_BINDINGS;
//		if (target == GL_TRANSFORM_FEEDBACK_BUFFER);  not in os x gl?
//			return GL_MAX_TRANSFORM_FEEDBACK_BUFFER_BINDINGS;
		assert(false && "Unknown indexed buffer target");
	}

	template <GLenum target>
	class IndexedBufferOps {
		static_assert(target == GL_UNIFORM_BUFFER || target == GL_TRANSFORM_FEEDBACK_BUFFER, "Invalid index buffer target");
		IndexedBufferOps() {}
		
		static GLint maxIndex_;

	public:
		static GLint maxIndex() {
			if (maxIndex_ < 0) {
				glGetIntegerv(maxBufferBindingsNameForTarget(target), &maxIndex_);
			}
			
			return maxIndex_;
		}

		static void bindBufferToIndex(const GLBuffer& buffer, uint32_t index) {
			assert(static_cast<GLint>(index) < maxIndex());
			glBindBufferBase(target, index, buffer.name());
		}

		static void bindBufferRangeToIndex(const GLBuffer& buffer, size32_t offset, size32_t bytes, uint32_t index) {
			assert(static_cast<GLint>(index) < maxIndex());
			assert(offset + bytes < buffer.byteSize());
			glBindBufferRange(target, index, buffer.name(), static_cast<GLintptr>(offset), bytes);
		}
	};

} // ns detail

// global GL array of indexed uniform blocks
template class detail::IndexedBufferOps<GL_UNIFORM_BUFFER>;
using UniformBlockArray = detail::IndexedBufferOps<GL_UNIFORM_BUFFER>;



} // ns render
} // ns stardazed

#endif
