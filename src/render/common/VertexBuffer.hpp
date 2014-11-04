// ------------------------------------------------------------------
// render::VertexBuffer - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_RENDER_VERTEXBUFFER_H
#define SD_RENDER_VERTEXBUFFER_H

#include "system/Config.hpp"
#include "util/ConceptTraits.hpp"
#include "render/common/BufferFields.hpp"
#include "render/common/BufferStorage.hpp"
#include "math/Vector.hpp"
#include "math/Matrix.hpp"

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace stardazed {
namespace render {


// exposition only, no real support for Tri32 yet
using Tri16 = std::array<uint16_t, 3>;
using Tri32 = std::array<uint32_t, 3>;

using Tri = Tri16;



enum class AttributeRole : uint16_t {
	Generic,
	Position,
	Normal,
	Tangent,
	Colour,
	UV,
	UVW,
	Index
};


struct Attribute {
	NamedField namedField;
	AttributeRole role;
};


using AttributeList = std::vector<Attribute>;


struct PositionedAttribute {
	Attribute attr;
	size32_t offset;
};


constexpr Field getField(const Attribute& attr) { return attr.namedField.field; }
constexpr Field getField(const PositionedAttribute& posAttr) { return posAttr.attr.namedField.field; }


class VertexBuffer {
	size32_t itemSizeBytes_ = 0, itemCount_ = 0;
	std::unique_ptr<BufferStorage> storage_;
 	std::vector<PositionedAttribute> attrs_;
	
	const PositionedAttribute* attrByPredicate(std::function<bool(const PositionedAttribute&)>) const;

public:
	VertexBuffer(const AttributeList&);

	// -- buffer data management

	size32_t itemSizeBytes() const { return itemSizeBytes_; }
	size32_t bufferSizeBytes() const { return itemSizeBytes_ * itemCount_; }
	
	template <typename Storage>
	void allocate(size32_t itemCount) {
		itemCount_ = itemCount;
		storage_ = std::make_unique<Storage>(itemCount_ * itemSizeBytes_);
	}

	// -- attribute metadata

	const PositionedAttribute* attrByRole(AttributeRole) const;
	const PositionedAttribute* attrByName(const std::string&) const;
	const PositionedAttribute* attrByIndex(size32_t) const;

	// -- iteration over attribute data

	template <typename NativeFieldType>
	class AttrIterator
	: public std::iterator<std::random_access_iterator_tag, NativeFieldType>
	, public FullyComparableTrait<AttrIterator<NativeFieldType>>
	{
		uint8_t* position_ = nullptr;
		size32_t rowBytes_ = 0;
		
	public:
		constexpr AttrIterator() {}
		constexpr AttrIterator(const VertexBuffer& vb, const PositionedAttribute& attr)
		: position_{ vb.storage_->getAs<uint8_t>() + attr.offset }
		, rowBytes_{ vb.itemSizeBytes() }
		{}
		
		constexpr NativeFieldType& operator *() { return *reinterpret_cast<NativeFieldType*>(position_); }
		constexpr NativeFieldType* operator ->() { return reinterpret_cast<NativeFieldType*>(position_); }
		
		constexpr NativeFieldType& operator [](size32_t index) {
			auto indexedPos = position_ + (rowBytes_ * index);
			return *reinterpret_cast<NativeFieldType*>(indexedPos);
		}
		
		constexpr const NativeFieldType& operator [](size32_t index) const {
			auto indexedPos = position_ + (rowBytes_ * index);
			return *reinterpret_cast<const NativeFieldType*>(indexedPos);
		}
		
		constexpr const AttrIterator& operator ++() { position_ += rowBytes_; return *this; }
		constexpr AttrIterator operator ++(int) { auto ret = *this; position_ += rowBytes_; return ret; }
		
		constexpr const AttrIterator& operator --() { position_ -= rowBytes_; return *this; }
		constexpr AttrIterator operator --(int) { auto ret = *this; position_ -= rowBytes_; return ret; }
		
		constexpr bool operator ==(const AttrIterator& other) const { return position_ == other.position_; }
		constexpr bool operator <(const AttrIterator& other) const { return position_ < other.position_; }
		
		friend constexpr AttrIterator operator +(const AttrIterator& iter, size32_t count) {
			auto ret = iter;
			ret.position_ += ret.rowBytes_ * count;
			return ret;
		}

		friend constexpr AttrIterator operator +(size32_t count, const AttrIterator& iter) {
			auto ret = iter;
			ret.position_ += ret.rowBytes_ * count;
			return ret;
		}

		friend constexpr AttrIterator operator -(const AttrIterator& iter, size32_t count) {
			auto ret = iter;
			ret.position_ -= ret.rowBytes_ * count;
			return ret;
		}

		friend AttrIterator& operator +=(AttrIterator& iter, size32_t count) {
			iter.position_ += iter.rowBytes_ * count;
			return iter;
		}

		friend AttrIterator& operator -=(AttrIterator& iter, size32_t count) {
			iter.position_ -= iter.rowBytes_ * count;
			return iter;
		}
		
		constexpr ptrdiff_t operator -(const AttrIterator& b) {
			return (position_ - b.position_) / static_cast<ptrdiff_t>(rowBytes_);
		}
	};
	
	template <typename T>
	friend class AttrIterator;
	
	template <typename T>
	AttrIterator<T> attrBegin(const PositionedAttribute& attr) const { return { *this, attr }; }
	template <typename T>
	AttrIterator<T> attrEnd(const PositionedAttribute& attr) const { return attrBegin<T>(attr) + itemCount_; }
	
	template <typename T>
	AttrIterator<T> attrBegin(AttributeRole role) const { return attrBegin<T>(*attrByRole(role)); }
	template <typename T>
	AttrIterator<T> attrEnd(AttributeRole role) const { return attrEnd<T>(*attrByRole(role)); }

	template <typename T>
	AttrIterator<T> attrBegin(const std::string& name) const { return attrBegin<T>(*attrByName(name)); }
	template <typename T>
	AttrIterator<T> attrEnd(const std::string& name) const { return attrEnd<T>(*attrByName(name)); }
};


} // ns render
} // ns stardazed

#endif