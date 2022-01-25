/***************************************************************************
 *  test_gocos_symbols.cpp - Tests for Golog symbol parsing
 *
 *  Created:   Tue 25 Jan 14:46:49 CET 2022
 *  Copyright  2022  Till Hofmann <hofmann@kbsg.rwth-aachen.de>
 *  SPDX-License-Identifier: LGPL-3.0-or-later
 ****************************************************************************/

#include "NamedType/named_type_impl.hpp"
#include "NamedType/underlying_functionalities.hpp"

#include <fmt/format.h>
#include <gocos/golog_symbols.h>

#include <NamedType/named_type.hpp>
#include <catch2/catch_test_macros.hpp>

namespace {

using tacos::search::split_symbol;

using ParsedSymbol = fluent::NamedType<std::pair<std::string, std::vector<std::string>>,
                                       struct ParsedSymbolTag,
                                       fluent::Callable,
                                       fluent::Comparable,
                                       fluent::Printable>;

[[maybe_unused]] std::ostream &
operator<<(std::ostream &os, const ParsedSymbol &symbol)
{
	const auto args = fmt::format("({})", fmt::join(symbol->second, ", "));
	os << "{" << symbol->first << ", " << args << "}";
	return os;
}

TEST_CASE("Parse Golog symbols", "[gocos]")
{
	CHECK(ParsedSymbol{split_symbol("foo()")} == ParsedSymbol({"foo", {}}));
	CHECK(ParsedSymbol{split_symbol("foo(bar)")} == ParsedSymbol({"foo", {"bar"}}));
	CHECK(ParsedSymbol{split_symbol("unfoo(bar)")} == ParsedSymbol({"unfoo", {"bar"}}));
	CHECK(ParsedSymbol{split_symbol("foo(bar, baz)")} == ParsedSymbol({"foo", {"bar", "baz"}}));
	CHECK(ParsedSymbol{split_symbol("foo(bar,baz)")} == ParsedSymbol({"foo", {"bar", "baz"}}));
	CHECK(ParsedSymbol{split_symbol("foo(bar, bar)")} == ParsedSymbol({"foo", {"bar", "bar"}}));
	CHECK(ParsedSymbol{split_symbol("  foo (    bar  ,  baz   ) ")}
	      == ParsedSymbol({"foo", {"bar", "baz"}}));
	CHECK(ParsedSymbol{split_symbol("foo")} == ParsedSymbol({"foo", {}}));
}

} // namespace
