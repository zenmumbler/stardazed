// ------------------------------------------------------------------
// geom::Generators.cpp - stardazed
// (c) 2014 by Arthur Langereis
// ------------------------------------------------------------------

#include "geom/Generators.hpp"
#include "math/Constants.hpp"
#include "math/Vector.hpp"
#include "math/Algorithm.hpp"

namespace stardazed {
namespace geom {
namespace gen {


render::MeshDescriptor plane(float width, float height, float tileMaxDim) {
	using namespace render;

	MeshDescriptor m({
		{ { render::fieldVec3(), "position" }, AttributeRole::Position },
		{ { render::fieldVec3(), "normal" }, AttributeRole::Normal }
	});
	
	size32 tilesWide = math::max(1.0f, width / tileMaxDim),
		   tilesHigh = math::max(1.0f, height / tileMaxDim),
		   tileDimX  = width / tilesWide,
		   tileDimZ  = height / tilesHigh;

	size32 vertexCount = (tilesWide + 1) * (tilesHigh + 1);
	size32 faceCount = 2 * tilesWide * tilesHigh;
	
	m.vertexBuffer.allocate<OwnedBufferStorage>(vertexCount);
	m.faces.reserve(faceCount);
	auto vit = m.vertexBuffer.attrBegin<math::Vec3>(AttributeRole::Position);
	
	float halfWidth = (tilesWide * tileDimX) / 2,
		  halfHeight = (tilesHigh * tileDimZ) / 2;

	// -- vertexes
	for (auto z = 0u; z <= tilesHigh; ++z) {
		float posZ = -halfHeight + (z * tileDimZ);

		for (auto x = 0u; x <= tilesWide; ++x) {
			float posX = -halfWidth	+ (x * tileDimX);
			*vit++ = { posX, 0, posZ };
		}
	}

	// -- faces
	uint16 baseIndex = 0,
		   vertexRowCount = tilesWide + 1;

	for (auto z = 0u; z < tilesHigh; ++z) {
		for (auto x = 0u; x < tilesWide; ++x) {
			m.faces.push_back({
				static_cast<uint16>(baseIndex + 1),
				static_cast<uint16>(baseIndex + vertexRowCount),
				static_cast<uint16>(baseIndex + vertexRowCount + 1)
			});
			m.faces.push_back({
				static_cast<uint16>(baseIndex),
				static_cast<uint16>(baseIndex + vertexRowCount),
				static_cast<uint16>(baseIndex + 1)
			});
		}
		
		baseIndex += vertexRowCount;
	}
	
	m.genVertexNormals();
	
	return m;
}

 
render::MeshDescriptor arc(float minRadius, float maxRadius, int radiusSteps,
						   math::Angle fromAng, math::Angle toAng, int angleSteps) {
	using math::Radians;
	using math::Tau;
	using namespace render;
	
	MeshDescriptor m({
		{ { render::fieldVec3(), "position" }, AttributeRole::Position },
		{ { render::fieldVec3(), "normal" }, AttributeRole::Normal }
	});
	
	// -- arc shape
	Radians angA = fromAng.rad(), angB = toAng.rad();
	if (angB < angA)
		angB += Tau;
	Radians angStep = (angB - angA) / angleSteps;

	auto radiusVerts = radiusSteps + 1;
	auto angleVerts  = angleSteps + 1;
	
	// -- radius steps
	std::vector<float> radii(radiusVerts);
	float radStep = (maxRadius - minRadius) / radiusSteps;
	std::generate(begin(radii), end(radii), [minRadius, radStep, i=0]() mutable {
		return minRadius + (i++ * radStep);
	});
	
	// -- buffers
	size32 vertexCount = radiusVerts * angleVerts;
	size32 faceCount = (radiusSteps * 2) * angleSteps;

	m.vertexBuffer.allocate<OwnedBufferStorage>(vertexCount);
	m.faces.reserve(faceCount);
	
	// -- vertexes
	auto vit = m.vertexBuffer.attrBegin<math::Vec3>(AttributeRole::Position);
	for (int step=0; step < angleVerts; ++step) {
		auto ang = angA + (step * angStep);
		std::transform(begin(radii), end(radii), vit, [ang](float r) {
			return math::Vec3{ r * math::cos(ang), 0, r * math::sin(ang) };
		});
		vit += radiusVerts;
	}
	
	// -- faces
	uint16_t vix = 0;
	for (int seg=0; seg < angleSteps; ++seg) {
		for (int track=0; track < radiusSteps; ++track) {
			m.faces.push_back({
				static_cast<uint16_t>(vix + track),
				static_cast<uint16_t>(vix + track + 1),
				static_cast<uint16_t>(vix + track + 1 + radiusVerts)
			});
			m.faces.push_back({
				static_cast<uint16_t>(vix + track),
				static_cast<uint16_t>(vix + track + 1 + radiusVerts),
				static_cast<uint16_t>(vix + track + radiusVerts)
			});
		}

		vix += radiusVerts;
	}

	m.genVertexNormals();
	
	return m;
}


render::MeshDescriptor cube(float diameter) {
	using namespace render;

	MeshDescriptor mesh({
		{ { render::fieldVec3(), "position" }, AttributeRole::Position },
		{ { render::fieldVec3(), "normal" }, AttributeRole::Normal }
	});
	mesh.vertexBuffer.allocate<OwnedBufferStorage>(8);
	mesh.faces.reserve(12);
	
	// vertexes
	auto hd = diameter / 2.f;
	auto posIter = mesh.vertexBuffer.attrBegin<math::Vec3>(AttributeRole::Position);

	*posIter++ = { -hd, -hd, -hd };
	*posIter++ = {  hd, -hd, -hd };
	*posIter++ = {  hd,  hd, -hd };
	*posIter++ = { -hd,  hd, -hd };

	*posIter++ = { -hd, -hd,  hd };
	*posIter++ = {  hd, -hd,  hd };
	*posIter++ = {  hd,  hd,  hd };
	*posIter++ = { -hd,  hd,  hd };

	// faces
	mesh.faces.push_back({ 0, 2, 1 }); // front
	mesh.faces.push_back({ 2, 0, 3 });
	mesh.faces.push_back({ 1, 6, 5 }); // right
	mesh.faces.push_back({ 6, 1, 2 });
	mesh.faces.push_back({ 5, 7, 4 }); // back
	mesh.faces.push_back({ 7, 5, 6 });
	mesh.faces.push_back({ 4, 3, 0 }); // left
	mesh.faces.push_back({ 3, 4, 7 });
	mesh.faces.push_back({ 4, 1, 5 }); // top
	mesh.faces.push_back({ 1, 4, 0 });
	mesh.faces.push_back({ 3, 6, 2 }); // bottom
	mesh.faces.push_back({ 6, 3, 7 });
	
	mesh.genVertexNormals();

	return mesh;
}



render::MeshDescriptor sphere(const int rows, const int segs, const float radius, float sliceFrom, float sliceTo) {
	using math::Pi; using math::Tau;
	using namespace render;
	
	assert(rows >= 2);
	assert(segs >= 4);
	sliceFrom = math::clamp(sliceFrom, 0.f, 1.f);
	sliceTo = math::clamp(sliceTo, 0.f, 1.f);
	assert(sliceTo > sliceFrom);

	bool hasTopDisc = sliceFrom == 0.f,
	hasBottomDisc = sliceTo == 1.f;
	
	MeshDescriptor mesh({
		{ { render::fieldVec3(), "position" }, AttributeRole::Position },
		{ { render::fieldVec3(), "normal" }, AttributeRole::Normal }
	});
	
	size32 vertexCount = segs * (rows - 1);
	if (hasTopDisc) ++vertexCount;
	if (hasBottomDisc) ++vertexCount;
	mesh.vertexBuffer.allocate<OwnedBufferStorage>(vertexCount);
	mesh.faces.reserve(2u * segs * rows);
	
	auto posIter = mesh.vertexBuffer.attrBegin<math::Vec3>(AttributeRole::Position),
		beginIter = posIter;
	
	auto slice = sliceTo - sliceFrom,
		piFrom = sliceFrom * Pi.val(),
		piSlice = slice * Pi.val();
	
	for (int row=0; row <= rows; ++row) {
		float y = std::cos(piFrom + (piSlice / rows) * row) * radius;
		float segRad = std::sin(piFrom + (piSlice / rows) * row) * radius;

		if (
			(hasTopDisc && row == 0) ||
			(hasBottomDisc && row == rows)
		) {
			// center top or bottom
			*posIter++ = { 0, y, 0 };
		}
		else {
			for (int seg=0; seg < segs; ++seg) {
				float x = math::sin((Tau / segs) * seg) * segRad;
				float z = math::cos((Tau / segs) * seg) * segRad;
				*posIter++ = { x, y, z };
			}
		}

		// construct row of faces
		if (row > 0) {
			int raix = static_cast<int>(posIter - beginIter),
				rbix = static_cast<int>(posIter - beginIter),
				ramul, rbmul;
			
			if (hasTopDisc && row == 1) {
				raix -= segs + 1;
				rbix -= segs;
				ramul = 0;
				rbmul = 1;
			}
			else if (hasBottomDisc && row == rows) {
				raix -= segs + 1;
				rbix -= 1;
				ramul = 1;
				rbmul = 0;
			}
			else {
				raix -= segs * 2;
				rbix -= segs;
				ramul = 1;
				rbmul = 1;
			}
			
			for (int seg=0; seg < segs; ++seg) {
				int ral = ramul * seg,
					rar = ramul * ((seg + 1) % segs),
					rbl = rbmul * seg,
					rbr = rbmul * ((seg + 1) % segs);
				
				mesh.faces.push_back({static_cast<uint16_t>(raix + ral), static_cast<uint16_t>(rbix + rbl), static_cast<uint16_t>(raix + rar)});
				mesh.faces.push_back({static_cast<uint16_t>(raix + rar), static_cast<uint16_t>(rbix + rbl), static_cast<uint16_t>(rbix + rbr)});
			}
		}
	}
	
	mesh.genVertexNormals();
	return mesh;
}


} // ns gen
} // ns geom
} // ns stardazed
