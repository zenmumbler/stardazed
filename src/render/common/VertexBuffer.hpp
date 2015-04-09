// ------------------------------------------------------------------
// render::VertexBuffer - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#ifndef SD_RENDER_VERTEXBUFFER_H
#define SD_RENDER_VERTEXBUFFER_H

#include "system/Config.hpp"
#include "util/ConceptTraits.hpp"
#include "render/common/BufferFields.hpp"
#include "math/Vector.hpp"
#include "math/Matrix.hpp"

#include <array>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace stardazed {
namespace render {


enum class AttributeRole : uint16 {
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
	Field field;
	AttributeRole role;
};


using AttributeList = std::vector<Attribute>;


struct PositionedAttribute {
	Attribute attr;
	size32 offset;
};


constexpr Field getField(const Attribute& attr) { return attr.field; }
constexpr Field getField(const PositionedAttribute& posAttr) { return posAttr.attr.field; }


class VertexBuffer {
	size32 itemSizeBytes_ = 0, itemCount_ = 0;
	std::unique_ptr<uint8[]> storage_;
	std::vector<PositionedAttribute> attrs_;
	
	const PositionedAttribute* attrByPredicate(std::function<bool(const PositionedAttribute&)>) const;

public:
	VertexBuffer(const AttributeList&);

	// -- buffer data management

	size32 itemSizeBytes() const { return itemSizeBytes_; }
	size32 itemCount() const { return itemCount_; }
	size32 bufferSizeBytes() const { return itemSizeBytes_ * itemCount_; }
	
	size32 bytesRequired(size32 itemCount) const {
		return itemCount * itemSizeBytes_;
	}

	void allocate(size32 itemCount) {
		itemCount_ = itemCount;
		storage_ = std::make_unique<uint8[]>(bytesRequired(itemCount_));
	}
	
	// -- attribute metadata
	
	size32 attributeCount() const { return static_cast<size32>(attrs_.size()); }

	const PositionedAttribute* attrByRole(AttributeRole) const;
	const PositionedAttribute* attrByIndex(size32) const;
	
	// -- raw data pointers
	
	void* basePointer() const { return storage_.get(); }
	
	void* attrBasePointer(const PositionedAttribute&) const;
	void* attrBasePointer(AttributeRole) const;

	// -- iteration over attribute data

	template <typename NativeFieldType>
	class AttrIterator
	: public std::iterator<std::random_access_iterator_tag, NativeFieldType>
	, public FullyComparableTrait<AttrIterator<NativeFieldType>>
	{
		uint8* position_ = nullptr;
		size32 rowBytes_ = 0;
		
	public:
		constexpr AttrIterator() {}
		constexpr AttrIterator(const VertexBuffer& vb, const PositionedAttribute& attr)
		: position_{ static_cast<uint8*>(vb.basePointer()) + attr.offset }
		, rowBytes_{ vb.itemSizeBytes() }
		{}
		
		constexpr NativeFieldType& operator *() { return *reinterpret_cast<NativeFieldType*>(position_); }
		constexpr NativeFieldType* operator ->() { return reinterpret_cast<NativeFieldType*>(position_); }
		
		constexpr NativeFieldType& operator [](size32 index) {
			auto indexedPos = position_ + (rowBytes_ * index);
			return *reinterpret_cast<NativeFieldType*>(indexedPos);
		}
		
		constexpr const NativeFieldType& operator [](size32 index) const {
			auto indexedPos = position_ + (rowBytes_ * index);
			return *reinterpret_cast<const NativeFieldType*>(indexedPos);
		}
		
		constexpr const AttrIterator& operator ++() { position_ += rowBytes_; return *this; }
		constexpr AttrIterator operator ++(int) { auto ret = *this; position_ += rowBytes_; return ret; }
		
		constexpr const AttrIterator& operator --() { position_ -= rowBytes_; return *this; }
		constexpr AttrIterator operator --(int) { auto ret = *this; position_ -= rowBytes_; return ret; }
		
		constexpr bool operator ==(const AttrIterator& other) const { return position_ == other.position_; }
		constexpr bool operator <(const AttrIterator& other) const { return position_ < other.position_; }
		
		friend constexpr AttrIterator operator +(const AttrIterator& iter, size32 count) {
			auto ret = iter;
			ret.position_ += ret.rowBytes_ * count;
			return ret;
		}

		friend constexpr AttrIterator operator +(size32 count, const AttrIterator& iter) {
			auto ret = iter;
			ret.position_ += ret.rowBytes_ * count;
			return ret;
		}

		friend constexpr AttrIterator operator -(const AttrIterator& iter, size32 count) {
			auto ret = iter;
			ret.position_ -= ret.rowBytes_ * count;
			return ret;
		}

		friend AttrIterator& operator +=(AttrIterator& iter, size32 count) {
			iter.position_ += iter.rowBytes_ * count;
			return iter;
		}

		friend AttrIterator& operator -=(AttrIterator& iter, size32 count) {
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
};


} // ns render
} // ns stardazed

#endif
