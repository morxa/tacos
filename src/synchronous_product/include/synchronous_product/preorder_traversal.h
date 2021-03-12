#pragma once

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace synchronous_product {

/// Exception thrown if the tree is inconsistent
class InconsistentTreeException : public std::logic_error
{
public:
	/** Constructor.
	 * @param what The error message
	 */
	InconsistentTreeException(const std::string &what) : std::logic_error(what)
	{
	}
};

/**
 * @brief Preorder iteratior class.
 * @details Assumes that the tree is built via unique_ptr to child nodes and raw_ptr to parent node.
 * @tparam Node Node type
 */
template <class Node>
class preorder_iterator : public std::iterator<std::forward_iterator_tag, Node>
{
public:
	preorder_iterator() = default;
	preorder_iterator(Node *root) : root_(root), cur_(root)
	{
	}
	preorder_iterator(Node *root, Node *cur) : root_(root), cur_(cur)
	{
	}
	virtual ~preorder_iterator()
	{
	}
	/**
	 * @brief Postfix increment
	 * @return preorder_iterator<Node>
	 */
	preorder_iterator<Node>
	operator++(int)
	{
		auto tmp = cur_;
		increment();
		return tmp;
	}
	/**
	 * @brief Prefix increment
	 * @return preorder_iterator<Node>
	 */
	preorder_iterator<Node> &
	operator++()
	{
		increment();
		return *this;
	}

	Node &
	operator*()
	{
		return *cur_;
	}

	Node *
	operator->()
	{
		return cur_;
	}

	bool
	operator==(const preorder_iterator<Node> &rhs) const
	{
		return cur_ == rhs.cur_;
	}

	bool
	operator!=(const preorder_iterator<Node> &rhs) const
	{
		return !(*this == rhs);
	}

private:
	void
	increment()
	{
		// descend through children
		if (!cur_->children.empty()) {
			cur_ = cur_->children[0].get();
			return;
		} else {
			// if this is the last child, ascend
			if (cur_ == cur_->parent->children.back().get()) {
				while (cur_ != root_ && cur_ == cur_->parent->children.back().get()) {
					cur_ = cur_->parent;
				}
				// reached root - end reached
				if (cur_ == root_) {
					cur_ = nullptr;
					return;
				} else {
					// find pos of cur and increment
					for (auto cPtrIt = cur_->parent->children.begin(); cPtrIt != cur_->parent->children.end();
					     ++cPtrIt) {
						if (cPtrIt->get() == cur_) {
							cur_ = std::next(cPtrIt)->get();
							return;
						}
					}
					throw InconsistentTreeException(
					  "Parent-child relation between current and parent node is not bidirectional");
				}
			} else {
				// find pos of cur and increment
				for (auto cPtrIt = cur_->parent->children.begin(); cPtrIt != cur_->parent->children.end();
				     ++cPtrIt) {
					if (cPtrIt->get() == cur_) {
						cur_ = std::next(cPtrIt)->get();
						return;
					}
				}
				throw InconsistentTreeException(
				  "Parent-child relation between current and parent node is not bidirectional");
			}
		}
	}

	Node *root_ = nullptr;
	Node *cur_  = nullptr;
};

template <typename Node>
preorder_iterator<Node>
begin(Node *root)
{
	return preorder_iterator<Node>{root};
}

template <typename Node>
preorder_iterator<Node>
end(Node *root)
{
	return preorder_iterator<Node>{root, nullptr};
}

} // namespace synchronous_product
