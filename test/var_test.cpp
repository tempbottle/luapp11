#include "catch.hpp"
#include "luapp11/lua.hpp"

using namespace luapp11;

TEST_CASE("var_test/copy", "var copy test") {
  int val = 10;
  auto node = global["test"] = val;
  auto node2(node);

  CHECK(node2.get<int>() == val);

  auto node3(global["dne"]);
  CHECK(node3 == global["dne"]);
}

TEST_CASE("var_test/move", "var move test") {
  int val = 10;
  auto node = global["test"] = val;
  auto node2(std::move(node));

  CHECK(node2.get<int>() == val);

  auto dne = global["dne"];
  auto node3(std::move(dne));

  CHECK(node3 == global["dne"]);
}

TEST_CASE("var_test/get_value", "get_value test") {
  int val = 10;
  auto node = global["test"] = val;
  auto val2 = node.get_value();

  CHECK(val2 == val);
  CHECK(global["dne"].get_value() == val::nil());
}

TEST_CASE("var_test/get", "get test") {
  int val = 10;
  auto node = global["test"] = val;
  CHECK(node.get<int>() == val);
  CHECK(node.get<std::string>() == "10");
  CHECK(node.get<bool>() == true);

  std::string val2 = "foo";
  auto node2 = global["test2"] = val2;
  CHECK(node2.get<std::string>() == val2);
  CHECK(node2.get<bool>() == true);
  CHECK_THROWS(node2.get<int>());

  bool val3 = true;
  auto node3 = global["test3"] = val3;
  CHECK(node3.get<bool>() == val3);
  CHECK(node3.get<int>() == 1);
  CHECK_THROWS(node3.get<std::string>());

  CHECK(global["dne"].get<int>() == 0);
}

TEST_CASE("var_test/is", "is test") {
  int val = 10;
  auto node = global["test"] = val;
  CHECK(node.is<int>());
  CHECK(node.is<bool>());
  CHECK(node.is<std::string>());
  CHECK(!node.is<std::function<void()>>());

  std::string val2 = "foo";
  auto node2 = global["test2"] = val2;
  CHECK(!node2.is<int>());
  CHECK(!node2.is<bool>());
  CHECK(node2.is<std::string>());
  CHECK(!node2.is<std::function<void()>>());

  bool val3 = true;
  auto node3 = global["test3"] = val3;
  CHECK(node3.is<int>());
  CHECK(node3.is<bool>());
  CHECK(!node3.is<std::string>());
  CHECK(!node3.is<std::function<void()>>());

  CHECK(!global["dne"].is<int>());
  CHECK(!global["dne"].is<bool>());
  CHECK(!global["dne"].is<std::string>());
  CHECK(!global["dne"].is<std::function<void()>>());
}

TEST_CASE("var_test/as", "as test") {
  int val = 10;
  auto node = global["test"] = val;
  CHECK(node.as<int>(100) == 10);
  CHECK(node.as<bool>(false) == true);
  CHECK(node.as<std::string>("shoes") == "10");

  std::string val2 = "foo";
  auto node2 = global["test2"] = val2;
  CHECK(node2.as<int>(100) == 100);
  CHECK(node2.as<bool>(false) == false);
  CHECK(node2.as<std::string>("shoes") == "foo");

  bool val3 = true;
  auto node3 = global["test3"] = val3;
  CHECK(node3.as<int>(100) == 1);
  CHECK(node3.as<bool>(false) == true);
  CHECK(node3.as<std::string>("shoes") == "shoes");

  CHECK(global["dne"].as<int>(100) == 100);
  CHECK(global["dne"].as<bool>(false) == false);
  CHECK(global["dne"].as<std::string>("shoes") == "shoes");
}

TEST_CASE("var_test/assign", "assign test") {
  int val = 10;
  auto node = global["test"] = val;
  auto node2 = global["test2"] = node;

  CHECK(node.get<int>() == node2.get<int>());
  CHECK(node != node2);

  std::vector<std::string> words({
    "foo", "bar", "baz"
  });
  auto vec = global["vec"] = words;
  CHECK(vec[1] == words[0]);
  CHECK(vec[2] == words[1]);
  CHECK(vec[3] == words[2]);

  auto init = global["init"] = { "foo", "bar", "baz" };
  CHECK(init[1] == words[0]);
  CHECK(init[2] == words[1]);
  CHECK(init[3] == words[2]);

  auto init2 = global["init2"] = { {"foo",1} , {"bar", 13}, {"baz",7} };
  CHECK(init2["foo"] == 1);
  CHECK(init2["bar"] == 13);
  CHECK(init2["baz"] == 7);

  auto init3 = global["init3"] = { 1, "foo", true};
  CHECK(init3[1] == 1);
  CHECK(init3[2] == std::string("foo"));
  CHECK(init3[3] == true);

  std::set<float> floats({
    .25f, .5f, .75f
  });
  auto set = global["set"] = floats;
  CHECK(set[.25f].get<bool>());
  CHECK(set[.5f].get<bool>());
  CHECK(set[.75f].get<bool>());

  std::unordered_set<int> ints({
    3, 2, 15
  });
  auto set2 = global["set2"] = ints;
  CHECK(set2[3].get<bool>());
  CHECK(set2[2].get<bool>());
  CHECK(set2[15].get<bool>());

  std::map<std::string, int> mapped({
    { "foo", 3 }
    , { "bar", 2 }
    , { "baz", 15 }
  });
  auto map = global["map"] = mapped;
  CHECK(map["foo"] == mapped["foo"]);
  CHECK(map["bar"] == mapped["bar"]);
  CHECK(map["baz"] == mapped["baz"]);

  std::unordered_map<int, std::string> mapped2({
    { 3, "foo" }
    , { 2, "bar" }
    , { 15, "baz" }
  });
  auto map2 = global["map2"] = mapped2;
  CHECK(map2[3] == mapped2[3]);
  CHECK(map2[2] == mapped2[2]);
  CHECK(map2[15] == mapped2[15]);
}

TEST_CASE("var_test/equality", "equality tests") {
  auto node = global["test"]["foo"];
  CHECK(node == node);
  CHECK(node == global["test"]["foo"]);
  CHECK(global["test"][1] == global["test"][1]);
  CHECK(global["test"] != global["test"]["foo"]);
  CHECK(global["test"]["foo"] != global["test"][1]);
}

TEST_CASE("var_test/do_chunk", "do_chunk test") {
  auto node = global["test"];
  auto err = node.do_chunk("return 15");
  CHECK(!(bool) err);
  CHECK(node.get<int>() == 15);

  auto node2 = global["test2"];
  auto err2 = node2.do_chunk("Invalid LUA;;");
  CHECK((bool) err2);
}

TEST_CASE("var_test/do_file", "do_file test") {
  auto node = global["test"];
  auto err = node.do_file("../test/lua/test.lua");
  CHECK(!(bool) err);
  CHECK(node.get<int>() == 5 * 4 * 3 * 2 * 1);

  auto node2 = global["test2"];
  auto err2 = node2.do_chunk("../test/lua/fails.lua");
  CHECK((bool) err2);
}

TEST_CASE("var_test/operator()", "operator() test") {
  auto func = global["func"];
  func.do_chunk(
      R"PREFIX(
    return function ()
      local v = 5
    end
    )PREFIX");
  CHECK_NOTHROW(func());
  CHECK_THROWS(global["foo"]());
}

TEST_CASE("var_test/invoke", "invoke test") {
  auto func = global["func"];
  func.do_chunk(
      R"PREFIX(
    return function (i)
      return i + 5
    end
    )PREFIX");
  auto result = func.invoke<int>(7);
  CHECK(result.success());
  CHECK(result.value() == 12);
  CHECK_THROWS(global["foo"].invoke<int>());

  auto func2 = global["func2"];
  func2.do_chunk(
      R"PREFIX(
    return function (i)
      return i + 5, "foo"
    end
    )PREFIX");
  auto result2 = func2.invoke<std::tuple<int, std::string>>(7);
  CHECK(result2.success());
  CHECK(result2.value() == std::make_tuple(12, "foo"));
}

int add(int a, int b) { return a + b; }

TEST_CASE("var_test/cfunc", "Calling c functions from lua test") {
  global["plusser"] = &add;
  auto func = global["func"];
  func.do_chunk(
      R"PREFIX(
    return function (i)
      return plusser(i, 5)
    end
    )PREFIX");
  auto result = func.invoke<int>(7);
  if (!result.success()) {
    std::cout << result.error() << std::endl;
  }
  CHECK(result.success());
  CHECK(result.value() == 12);

  global["lambda"] = [](int a, int b) { return a + b; }
  ;
  auto func2 = global["func2"];
  func2.do_chunk(
      R"PREFIX(
    return function (i)
      return lambda(i, 5);
    end
    )PREFIX");
  auto result2 = func2.invoke<int>(7);
  CHECK(result2.success());
  CHECK(result2.value() == 12);
}