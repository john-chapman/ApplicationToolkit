#pragma once

#include <apt/apt.h>
#include <apt/memory.h>
#include <apt/types.h>
#include <apt/math.h>

#include <EASTL/functional.h>
#include <EASTL/fixed_vector.h>

namespace apt {

///////////////////////////////////////////////////////////////////////////////
// Quadtree
// Generic linear quadtree.
//
// tIndex is the type used for indexing nodes and determines the absolute max
// level of subdivision possible. This should be a uint* type (uint8, uint16, 
// uint32, uint64).
//
// tNode is the node type. Typically this will be a pointer or index into a
// separate node data pool. Use the _init arg of the ctor to init the quadtree
// with 'invalid' nodes.
//
// Internally each level is stored sequentially with the root level at index 0.
// Within each level, nodes are laid out in Morton order:
//  +---+---+
//  | 0 | 2 |
//  +---+---+
//  | 1 | 3 |
//  +---+---+
// Use linearize()/delinearize() functions to convert to/from a linear layout
// e.g. for conversion to a texture.
//
// \todo Make static functions private?
// \todo Better implementation of FindNeighbor()?
// \todo Implement linearize/delinearize.
///////////////////////////////////////////////////////////////////////////////
template <typename tIndex, typename tNode>
class Quadtree
{
	int    m_levelCount;
	tNode* m_nodes;

public:
	typedef tIndex Index;
	typedef tNode  Node;
	typedef eastl::function<bool(Index _nodeIndex, int _nodeLevel)> OnVisit;
	static constexpr Index Index_Invalid  = ~Index(0);

	// Absolute max number of levels given number of index bits = bits/2.
	static constexpr int    GetAbsoluteMaxLevelCount()                                        { return (int)(sizeof(Index) * CHAR_BIT) / 2; }

	// Node count at _level = 4^_level.
	static constexpr Index  GetNodeCount(int _level)                                          { return 1 << (2 * _level); }

	// Width (in nodes) at _level = sqrt(GetNodeCount(_level)).
	static constexpr Index  GetWidth(int _level)                                              { return 1 << _level; }

	// Total node count = 4*(leafCount - 1)/3+1.
	static constexpr Index  GetTotalNodeCount(int _levelCount)                                { return 4 * (GetNodeCount(_levelCount - 1) - 1) / 3 + 1; }

	// Index of first node at _level.
	static constexpr Index  GetLevelStartIndex(int _level)                                    { return (_level == 0) ? 0 : GetTotalNodeCount(_level); } 

	// Index of the first child.
	static           Index  GetFirstChildIndex(Index _parentIndex, int _parentLevel);

	// Index of the parent.
	static           Index  GetParentIndex(Index _childIndex, int _childLevel);

	// Neighbor at signed offset from _nodeIndex (or Index_Invalid if offset is outside the quadtree).
	static           Index  FindNeighbor(Index _nodeIndex, int _nodeLevel, int _offsetX, int _offsetY);

	// Given _index, find the quadtree level.
	static           int    FindLevel(Index _nodeIndex);

	// Convert _nodeIndex to a Cartesian offset relative to the quadtree origin.
	static           uvec2  ToCartesian(Index _nodeIndex, int _nodeLevel);

	// Convert Cartesian coordinates to an index.
	static           Index  ToIndex(Index _x, Index _y, int _nodeLevel);


	Quadtree(int _levelCount = GetAbsoluteMaxLevelCount(), Node _init = Node());
	~Quadtree();

	// Depth-first traversal of the quadtree starting at _root, call _onVisit for each node. Traversal proceeds to a node's children only if _onVisit returns true.
	void        traverse(OnVisit _onVisit, Index _rootIndex = 0);

	// Find a valid neighbor at _offsetX, _offsetY from the given node.
	Index       findValidNeighbor(Index _nodeIndex, int _nodeLevel, int _offsetX, int _offsetY, Node _invalidNode = Node());

	// Width of a node in leaf nodes at _levelIndex (e.g. quadtree width at level 0, 1 at max level).
	Index       getNodeWidth(int _levelIndex) const                                          { return GetWidth(APT_MAX(m_levelCount - _levelIndex - 1, 0)); }

	// Node access.
	Node&       operator[](Index _index)                                                     { APT_STRICT_ASSERT(_index < GetTotalNodeCount(m_levelCount)); return m_nodes[_index]; }
	const Node& operator[](Index _index) const                                               { APT_STRICT_ASSERT(_index < GetTotalNodeCount(m_levelCount)); return m_nodes[_index]; }
	int         getTotalNodeCount() const                                                    { return GetTotalNodeCount(m_levelCount); }
	Index       getIndex(const Node& _node) const                                            { return &_node - m_nodes; }
	Index       getParentIndex(Index _childIndex, int _childLevel) const                     { return (_childLevel == 0) ? Index_Invalid : GetParentIndex(_childIndex, _childLevel); }
	Index       getFirstChildIndex(Index _parentIndex, int _parentLevel) const               { return (_parentLevel >= m_levelCount - 1) ? Index_Invalid : GetFirstChildIndex(_parentIndex, _parentLevel); }

	// Level access.
	const Node* getLevel(int _levelIndex) const                                              { APT_STRICT_ASSERT(_levelIndex < m_levelCount); return m_nodes + GetLevelStartIndex(_level); }
	Node*       getLevel(int _levelIndex)                                                    { APT_STRICT_ASSERT(_levelIndex < m_levelCount); return m_nodes + GetLevelStartIndex(_level); }
	Index       getNodeCount(int _levelIndex) const                                          { return GetNodeCount(_levelIndex); }
	int         getLevelCount() const                                                        { return m_levelCount; }

	// Linearize/delinearize nodes for a level. This is useful e.g. when converting to/from a texture representation.
	void        linearize(int _levelIndex, Node* out_) const                                 { APT_ASSERT(false); } // \todo
	void        delinearize(int _levelIndex, const Node* _in)                                { APT_ASSERT(false); } // \todo
};


/*******************************************************************************

                                  Quadtree

*******************************************************************************/

#define APT_QUADTREE_TEMPLATE_DECL template <typename tIndex, typename tNode>
#define APT_QUADTREE_CLASS_DECL    Quadtree<tIndex, tNode>

APT_QUADTREE_TEMPLATE_DECL 
inline tIndex APT_QUADTREE_CLASS_DECL::GetFirstChildIndex(Index _parentIndex, int _parentLevel)
{
	auto   parentOffset = GetLevelStartIndex(_parentLevel);
	auto   childOffset  = GetLevelStartIndex(_parentLevel + 1);
	return childOffset + ((_parentIndex - parentOffset) << 2);
}

APT_QUADTREE_TEMPLATE_DECL 
inline tIndex APT_QUADTREE_CLASS_DECL::GetParentIndex(Index _childIndex, int _childLevel)
{
	auto   childOffset  = GetLevelStartIndex(_childLevel);
	auto   parentOffset = GetLevelStartIndex(APT_MAX(_childLevel - 1, 0));
	return parentOffset + ((_childIndex - childOffset) >> 2);
}

APT_QUADTREE_TEMPLATE_DECL 
inline tIndex APT_QUADTREE_CLASS_DECL::FindNeighbor(Index _nodeIndex, int _nodeLevel, int _offsetX, int _offsetY)
{
	if (_nodeIndex == Index_Invalid) {
		return Index_Invalid;
	}
	uvec2 offset = ToCartesian(_nodeIndex, _nodeLevel) + uvec2(_offsetX, _offsetY);
	return ToIndex(offset.x, offset.y, _nodeLevel);
}

APT_QUADTREE_TEMPLATE_DECL 
inline int APT_QUADTREE_CLASS_DECL::FindLevel(Index _nodeIndex)
{
	for (int i = 0, n = GetAbsoluteMaxLevelCount(); i < n; ++i) {
		if (_nodeIndex < GetLevelStartIndex(i + 1)) {
			return i;
		}
	}
	return -1;
}

APT_QUADTREE_TEMPLATE_DECL 
inline uvec2 APT_QUADTREE_CLASS_DECL::ToCartesian(Index _nodeIndex, int _nodeLevel)
{
 // traverse the index LSB -> MSB summing node width (node width = number of leaf nodes covered, start at 1) 
	_nodeIndex -= GetLevelStartIndex(_nodeLevel);
	Index width = 1;
	uvec2 ret = uvec2(0u);
	for (int i = 0; i < _nodeLevel; ++i, width *= 2) {
		ret.y += (_nodeIndex & 1) * width; 
		_nodeIndex = _nodeIndex >> 1;
		ret.x += (_nodeIndex & 1) * width;
		_nodeIndex = _nodeIndex >> 1;
	}
	return ret;
}

APT_QUADTREE_TEMPLATE_DECL 
inline tIndex APT_QUADTREE_CLASS_DECL::ToIndex(Index _x, Index _y, int _nodeLevel)
{
 // _x or _y are outside the quadtree
	auto w = GetWidth(_nodeLevel);
	if (_x >= w || _y >= w) {
		return Index_Invalid;
	}

 // interleave _x and _y to produce the Morton code, add level offset
	Index ret = 0;
	for (int i = 0; i < sizeof(Index) * CHAR_BIT; ++i) {
		ret = ret | (_y & 1 << i) << i | (_x & 1 << i) << (i + 1);
	}
	return ret + GetLevelStartIndex(_nodeLevel);
}


APT_QUADTREE_TEMPLATE_DECL 
inline APT_QUADTREE_CLASS_DECL::Quadtree(int _levelCount, Node _init)
	: m_levelCount(_levelCount)
{
	APT_STATIC_ASSERT(!DataTypeIsSigned(APT_DATA_TYPE_TO_ENUM(Index))); // use an unsigned type
	APT_ASSERT(_levelCount <= GetAbsoluteMaxLevelCount()); // not enough bits in tIndex

	m_nodes = (Node*)APT_MALLOC_ALIGNED(sizeof(Node) * GetTotalNodeCount(_levelCount), alignof(Node));
	for (Index i = 0, n = GetTotalNodeCount(_levelCount); i < n; ++i) {
		m_nodes[i] = _init;
	}
}

APT_QUADTREE_TEMPLATE_DECL 
inline APT_QUADTREE_CLASS_DECL::~Quadtree()
{
	APT_FREE_ALIGNED(m_nodes);
}


APT_QUADTREE_TEMPLATE_DECL
inline tIndex APT_QUADTREE_CLASS_DECL::findValidNeighbor(Index _nodeIndex, int _nodeLevel, int _offsetX, int _offsetY, Node _invalidNode)
{
	Index ret = FindNeighbor(_nodeIndex, _nodeLevel, _offsetX, _offsetY); // get neighbor index at the same level
	while (ret != Index_Invalid && m_nodes[ret] == _invalidNode) { // search up the tree until a valid node is found
		ret = getParentIndex(ret, _nodeLevel--);
	}
	return ret;
}

APT_QUADTREE_TEMPLATE_DECL 
inline void APT_QUADTREE_CLASS_DECL::traverse(OnVisit _onVisit, Index _root)
{
	struct NodeAddr { Index m_index; int m_level; }; // store level in the stack, avoid calling FindLevel()
	eastl::fixed_vector<NodeAddr, GetAbsoluteMaxLevelCount() * 4> tstack; // depth-first traversal has a small upper limit on the stack size
	tstack.push_back({ _root, FindLevel(_root) });
	while (!tstack.empty()) {
		auto node = tstack.back();
		tstack.pop_back();

		if (_onVisit(node.m_index, node.m_level) && node.m_level < m_levelCount - 1) {
			auto i = GetFirstChildIndex(node.m_index, node.m_level);
			tstack.push_back({ i + 0u, node.m_level + 1 });
			tstack.push_back({ i + 1u, node.m_level + 1 });
			tstack.push_back({ i + 2u, node.m_level + 1 });
			tstack.push_back({ i + 3u, node.m_level + 1 });
		}
	}
}

#undef APT_QUADTREE_TEMPLATE_DECL
#undef APT_QUADTREE_CLASS_DECL

} // namespace apt