/***************************************************************************
 *  test_tree_iteration.cpp - Test the tree preorder iteration
 *
 *  Created:   Fri  12 Mar 14:17:00 CET 2021
 *  Copyright  2021  Stefan Schupp <stefan.schupp@cs.rwth-aachen.de>
 ****************************************************************************/
/*  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  Read the full text in the LICENSE.md file.
 */

#include "search/preorder_traversal.h"
#include "search/search_tree.h"

#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <ostream>

namespace {

struct TestNode
{
	TestNode(int d, TestNode *parent = nullptr) : data(d), parent(parent)
	{
	}
	int                                    data;
	TestNode *                             parent   = nullptr;
	std::vector<std::unique_ptr<TestNode>> children = {};

	bool
	operator==(const TestNode &rhs) const
	{
		return data == rhs.data;
	}
};

std::ostream &
operator<<(std::ostream &out, const TestNode &n)
{
	out << n.data;
	if (n.parent != nullptr) {
		out << " parent: " << n.parent->data;
	} else {
		out << " parent: NULL";
	}
	return out;
}

TEST_CASE("Simple Tree traversal", "[search]")
{
	spdlog::set_level(spdlog::level::trace);
	TestNode *root = new TestNode(0);
	root->children.emplace_back(std::make_unique<TestNode>(1, root));
	root->children.emplace_back(std::make_unique<TestNode>(2, root));
	root->children.emplace_back(std::make_unique<TestNode>(3, root));
	root->children.emplace_back(std::make_unique<TestNode>(4, root));

	SECTION("Tree iterator prefix increment")
	{
		synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);
		CHECK(*it == *root);
		CHECK((++it)->data == 1);
		CHECK((++it)->data == 2);
		CHECK((++it)->data == 3);
		CHECK((++it)->data == 4);
		CHECK((++it) == synchronous_product::end(root));
	}

	SECTION("Tree iterator postfix-increment")
	{
		synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);
		CHECK(*it == *root);
		it++;
		CHECK(it->data == 1);
		it++;
		CHECK(it->data == 2);
		it++;
		CHECK(it->data == 3);
		it++;
		CHECK(it->data == 4);
		it++;
		CHECK(it == synchronous_product::end(root));
	}
}

TEST_CASE("Traversal of a corrupted tree", "[search]")
{
	spdlog::set_level(spdlog::level::trace);
	SECTION("The parent of a leaf-node is corrupted")
	{
		TestNode *root = new TestNode(0);
		root->children.emplace_back(std::make_unique<TestNode>(1, root));
		root->children.emplace_back(std::make_unique<TestNode>(2, root));
		root->children.emplace_back(std::make_unique<TestNode>(3, nullptr));
		root->children.emplace_back(std::make_unique<TestNode>(4, root));
		synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);

		CHECK(*it == *root);
		CHECK((++it)->data == 1);
		CHECK((++it)->data == 2);
		CHECK((++it)->data == 3);
		CHECK_THROWS((++it));
		delete root;
	}

	SECTION("The parent of an intermediate, rightmost-node is corrupted")
	{
		TestNode *root = new TestNode(0);
		root->children.emplace_back(std::make_unique<TestNode>(1, root));
		root->children.emplace_back(std::make_unique<TestNode>(2, root));
		root->children.emplace_back(std::make_unique<TestNode>(3, root));
		root->children.emplace_back(std::make_unique<TestNode>(4, nullptr));
		root->children[3]->children.emplace_back(
		  std::make_unique<TestNode>(5, root->children[3].get()));
		synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);

		CHECK(*it == *root);
		CHECK((++it)->data == 1);
		CHECK((++it)->data == 2);
		CHECK((++it)->data == 3);
		CHECK((++it)->data == 4);
		CHECK((++it)->data == 5);
		CHECK_THROWS((++it));
		delete root;
	}

	SECTION("The parent of an intermediate-node is corrupted")
	{
		TestNode *root = new TestNode(0);
		root->children.emplace_back(std::make_unique<TestNode>(1, root));
		root->children.emplace_back(std::make_unique<TestNode>(2, root));
		root->children.emplace_back(std::make_unique<TestNode>(3, nullptr));
		root->children.emplace_back(std::make_unique<TestNode>(4, root));
		root->children[2]->children.emplace_back(
		  std::make_unique<TestNode>(5, root->children[2].get()));
		synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);

		CHECK(*it == *root);
		CHECK((++it)->data == 1);
		CHECK((++it)->data == 2);
		CHECK((++it)->data == 3);
		CHECK((++it)->data == 5);
		CHECK_THROWS((++it));
		delete root;
	}
}

TEST_CASE("Multilevel Tree traversal", "[search]")
{
	spdlog::set_level(spdlog::level::trace);
	TestNode *root = new TestNode(0);
	root->children.emplace_back(std::make_unique<TestNode>(1, root));
	root->children.emplace_back(std::make_unique<TestNode>(2, root));
	root->children.emplace_back(std::make_unique<TestNode>(3, root));
	root->children.emplace_back(std::make_unique<TestNode>(4, root));
	root->children[0]->children.emplace_back(std::make_unique<TestNode>(5, root->children[0].get()));
	root->children[0]->children.emplace_back(std::make_unique<TestNode>(6, root->children[0].get()));
	root->children[3]->children.emplace_back(std::make_unique<TestNode>(7, root->children[3].get()));
	root->children[3]->children.emplace_back(std::make_unique<TestNode>(8, root->children[3].get()));
	synchronous_product::preorder_iterator<TestNode> it = synchronous_product::begin(root);

	CHECK(*it == *root);
	CHECK((++it)->data == 1);
	CHECK((++it)->data == 5);
	CHECK((++it)->data == 6);
	CHECK((++it)->data == 2);
	CHECK((++it)->data == 3);
	CHECK((++it)->data == 4);
	CHECK((++it)->data == 7);
	CHECK((++it)->data == 8);
	CHECK((++it) == synchronous_product::end(root));
	// try past-end iteration
	CHECK((++it) == synchronous_product::end(root));
}

TEST_CASE("Multilevel Subtree traversal", "[search]")
{
	spdlog::set_level(spdlog::level::trace);
	TestNode *root = new TestNode(0);
	root->children.emplace_back(std::make_unique<TestNode>(1, root));
	root->children.emplace_back(std::make_unique<TestNode>(2, root));
	root->children.emplace_back(std::make_unique<TestNode>(3, root));
	root->children.emplace_back(std::make_unique<TestNode>(4, root));
	root->children[0]->children.emplace_back(std::make_unique<TestNode>(5, root->children[0].get()));
	root->children[0]->children.emplace_back(std::make_unique<TestNode>(6, root->children[0].get()));
	root->children[0]->children[1]->children.emplace_back(
	  std::make_unique<TestNode>(9, root->children[0]->children[1].get()));
	root->children[0]->children[1]->children.emplace_back(
	  std::make_unique<TestNode>(10, root->children[0]->children[1].get()));
	root->children[3]->children.emplace_back(std::make_unique<TestNode>(7, root->children[3].get()));
	root->children[3]->children.emplace_back(std::make_unique<TestNode>(8, root->children[3].get()));
	synchronous_product::preorder_iterator<TestNode> it =
	  synchronous_product::begin(root->children[0].get());

	CHECK(*it == *root->children[0].get());
	CHECK((++it)->data == 5);
	CHECK((++it)->data == 6);
	CHECK((++it)->data == 9);
	CHECK((++it)->data == 10);
	CHECK((++it) == synchronous_product::end(root));
}

} // namespace
