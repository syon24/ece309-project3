// tests/p3/test_p3.cpp — YOUR P3 TEST SUITE.
//
// The five tests below are worked examples. Keep them, and add your own
// until you have AT LEAST 15 (spec §6 lists what must be covered).
// Name every test <area>_<what>: reg_ for the registry, ws_ for the
// workspace, calc_ for the calculator.
//
// Build & run:   cmake --build build && ./build/test_p3
// One area:      ./build/test_p3 reg_        (names starting with "reg_")
// One test:      ./build/test_p3 =reg_add_and_find

#include "../mini_test.h"
#include "../test_helpers.h"     // TempDir, write_text, args1, ...

#include "tools/calculator.h"
#include "tools/tool_registry.h"
#include "tools/workspace.h"

#include <algorithm>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

// A tool whose name you choose — use it to fill the registry in tests.
class NamedTool : public Tool {
public:
    explicit NamedTool(std::string n) : n_(std::move(n)), schema_(JsonValue::object()) {}
    std::string name() const override { return n_; }
    std::string description() const override { return "test tool"; }
    const JsonValue& schema() const override { return schema_; }
    ToolResult invoke(const JsonValue&, ToolContext&) override { return ToolResult::success(n_); }
private:
    std::string n_;
    JsonValue   schema_;   // an empty JSON object: {}
};

// Tokenize with the provided tokenizer, then run YOUR calc_evaluate.
bool eval(const std::string& expr, double* value, std::string* error) {
    std::vector<CalcToken> tokens;
    if (!calc_tokenize(expr, &tokens, error)) return false;
    return calc_evaluate(tokens, value, error);
}

}  // namespace

// ---------------------------------------------------------------- examples

TEST(reg_add_and_find) {
    ToolRegistry r;
    auto t = std::make_unique<NamedTool>("alpha");
    Tool* raw = t.get();                        // remember the address before moving
    REQUIRE(r.add(std::move(t)));
    CHECK(r.size() == 1);
    CHECK(r.find("alpha") == raw);              // same object, not a copy
    CHECK(r.find("beta") == nullptr);
}

TEST(reg_duplicate_keeps_original) {
    ToolRegistry r;
    auto first = std::make_unique<NamedTool>("alpha");
    Tool* original = first.get();
    REQUIRE(r.add(std::move(first)));
    CHECK(!r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.find("alpha") == original);
    CHECK(r.size() == 1);
}

TEST(ws_rejects_parent_escape) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("notes/todo.txt", &out, &reason));   // sanity: accepts *something*
    CHECK(out == ws.root() / "notes" / "todo.txt");
    CHECK(!ws.resolve("../etc/passwd", &out, &reason));
    CHECK(!reason.empty());
}

TEST(calc_precedence) {
    double v = 0;
    std::string err;
    REQUIRE(eval("2 + 3 * 4", &v, &err));
    CHECK(v == 14);
}

TEST(calc_division_by_zero) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / 0", &v, &err));
    CHECK(err == "division by zero");
}


TEST(reg_empty) {
    ToolRegistry r;
    CHECK(r.size() == 0);
    CHECK(r.bucket_count() >= 1);
    CHECK(r.bucket_count() <= 64);
    CHECK(r.load_factor() == 0.0);
    CHECK(r.max_probe_length() == 0);
    CHECK(r.find("missing") == nullptr);
    int visits = 0;
    r.for_each([&](const Tool&) { ++visits; });
    CHECK(visits == 0);
}

TEST(reg_null_rejected) {
    ToolRegistry r;
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 0);
    REQUIRE(r.add(std::make_unique<NamedTool>("alpha")));
    CHECK(r.size() == 1);
    CHECK(!r.add(nullptr));
    CHECK(r.size() == 1);
    CHECK(r.find("missing") == nullptr);
    CHECK(r.size() == 1);
}

TEST(reg_for_each_visits_each_once) {
    ToolRegistry r;
    for (int i = 0; i < 100; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
    }
    std::vector<int> visits(100, 0);
    r.for_each([&](const Tool& tool) {
        const std::string name = tool.name();
        REQUIRE(name.substr(0, 5) == "tool_");
        int index = std::stoi(name.substr(5));
        REQUIRE(index >= 0 && index < 100);
        ++visits[index];
        CHECK(r.find(name) == &tool);
    });
    for (int count : visits) CHECK(count == 1);
}

TEST(reg_collision_keeps_both_tools) {
    ToolRegistry r;
    std::vector<std::string> seen(r.bucket_count());
    std::string first, second;
    for (int i = 0; i < 1000 && second.empty(); ++i) {
        std::string name = "collision_" + std::to_string(i);
        std::size_t index = ToolRegistry::hash(name) % r.bucket_count();
        if (seen[index].empty()) seen[index] = name;
        else {
            first = seen[index];
            second = name;
        }
    }
    REQUIRE(!second.empty());
    REQUIRE(r.add(std::make_unique<NamedTool>(first)));
    Tool* original = r.find(first);
    REQUIRE(r.add(std::make_unique<NamedTool>(second)));
    CHECK(r.find(first) == original);
    REQUIRE(r.find(second) != nullptr);
    CHECK(r.find(second)->name() == second);
    CHECK(r.max_probe_length() == 2);
}

TEST(reg_resize_at_threshold) {
    ToolRegistry r;
    std::size_t initial = r.bucket_count();
    for (std::size_t i = 0; i < initial; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>(std::to_string(i))));
        CHECK(r.bucket_count() == initial);
    }
    CHECK(r.load_factor() == 1.0);
    CHECK(!r.add(std::make_unique<NamedTool>("0")));
    CHECK(r.bucket_count() == initial);
    CHECK(r.size() == initial);
    REQUIRE(r.add(std::make_unique<NamedTool>("next")));
    CHECK(r.bucket_count() == 2 * initial);
    CHECK(r.size() == initial + 1);
    CHECK(r.find("0") != nullptr);
    CHECK(r.find("next") != nullptr);
}

TEST(reg_ownership) {
    class CountedTool : public NamedTool {
    public:
        CountedTool(std::string name, int& count)
            : NamedTool(std::move(name)), count_(count) {}
        ~CountedTool() override { ++count_; }
    private:
        int& count_;
    };

    int destroyed = 0;
    {
        ToolRegistry r;
        auto first = std::make_unique<CountedTool>("alpha", destroyed);
        Tool* original = first.get();
        REQUIRE(r.add(std::move(first)));
        CHECK(destroyed == 0);
        CHECK(!r.add(std::make_unique<CountedTool>("alpha", destroyed)));
        CHECK(destroyed == 1);
        CHECK(r.find("alpha") == original);
        for (int i = 0; i < 100; ++i) {
            REQUIRE(r.add(std::make_unique<CountedTool>(std::to_string(i), destroyed)));
        }
        CHECK(destroyed == 1);
    }
    CHECK(destroyed == 102);
}

TEST(reg_hash_uses_order_and_all_characters) {
    CHECK(ToolRegistry::hash("ab") != ToolRegistry::hash("ba"));
    CHECK(ToolRegistry::hash("tool_1234") != ToolRegistry::hash("tool_1243"));
    std::vector<std::size_t> hashes;
    for (int i = 0; i < 10000; ++i) {
        hashes.push_back(ToolRegistry::hash("tool_" + std::to_string(i)));
    }
    std::sort(hashes.begin(), hashes.end());
    std::size_t distinct = 1;
    for (std::size_t i = 1; i < hashes.size(); ++i) {
        if (hashes[i] != hashes[i - 1]) ++distinct;
    }
    CHECK(distinct >= 9990);
}

TEST(reg_10000_tools_and_alternative_hash) {
    ToolRegistry r;
    Tool* first = nullptr;
    for (int i = 0; i < 10000; ++i) {
        REQUIRE(r.add(std::make_unique<NamedTool>("tool_" + std::to_string(i))));
        if (i == 0) first = r.find("tool_0");
        CHECK(r.size() == static_cast<std::size_t>(i + 1));
        CHECK(r.load_factor() <= 1.0);
    }
    REQUIRE(first != nullptr);
    CHECK(r.find("tool_0") == first);
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        Tool* tool = r.find(name);
        REQUIRE(tool != nullptr);
        CHECK(tool->name() == name);
    }
    CHECK(r.find("tool_10000") == nullptr);
    CHECK(r.max_probe_length() <= 64);

    std::vector<int> weak_counts(r.bucket_count(), 0);
    std::vector<int> real_counts(r.bucket_count(), 0);
    for (int i = 0; i < 10000; ++i) {
        std::string name = "tool_" + std::to_string(i);
        std::size_t weak = static_cast<unsigned char>(name[0]) + name.size();
        ++weak_counts[weak % r.bucket_count()];
        ++real_counts[ToolRegistry::hash(name) % r.bucket_count()];
    }
    int weak_longest = *std::max_element(weak_counts.begin(), weak_counts.end());
    int real_longest = *std::max_element(real_counts.begin(), real_counts.end());
    CHECK(r.max_probe_length() == static_cast<std::size_t>(real_longest));
    CHECK(weak_longest == 9000);
    std::printf("10000 tools: buckets=%zu load=%.8f longest_chain=%zu weak_longest_chain=%d\n",
                r.bucket_count(), r.load_factor(), r.max_probe_length(), weak_longest);
}

TEST(ws_normalizes_relative_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("./notes//todo.txt", &out, &reason));
    CHECK(out == ws.root() / "notes" / "todo.txt");
    REQUIRE(ws.resolve("a/../b", &out, &reason));
    CHECK(out == ws.root() / "b");
}

TEST(ws_accepts_root) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    for (const char* path : {".", "a/.."}) {
        REQUIRE(ws.resolve(path, &out, &reason));
        CHECK(out == ws.root());
    }
}

TEST(ws_accepts_new_file_without_creating_it) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("new/dir/file.txt", &out, &reason));
    CHECK(out == ws.root() / "new" / "dir" / "file.txt");
    CHECK(!fs::exists(ws.root() / "new"));
}

TEST(ws_accepts_dots_in_filename) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    REQUIRE(ws.resolve("my..notes.txt", &out, &reason));
    CHECK(out == ws.root() / "my..notes.txt");
}

TEST(ws_rejects_empty_and_nul) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("", &out, &reason));
    CHECK(reason == "empty path");
    reason.clear();
    std::string path = std::string("notes") + '\0' + "/x";
    CHECK(!ws.resolve(path, &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_absolute_paths) {
    TempDir d;
    Workspace ws(d.path());
    fs::path out;
    std::string reason;
    CHECK(!ws.resolve("/etc/passwd", &out, &reason));
    CHECK(!reason.empty());
    reason.clear();
    CHECK(!ws.resolve((ws.root() / "inside.txt").string(), &out, &reason));
    CHECK(!reason.empty());
}

TEST(ws_rejects_normalized_escapes) {
    TempDir d;
    Workspace ws(d.path() / "ws");
    fs::path out;
    for (const char* path : {"..", "../etc/passwd", "a/../../b",
                             "notes/../../ws/notes/todo.txt"}) {
        std::string reason;
        CHECK(!ws.resolve(path, &out, &reason));
        CHECK(!reason.empty());
    }
}

TEST(calc_left_to_right) {
    double v = 0;
    std::string err;
    REQUIRE(eval("10 - 4 - 3", &v, &err));
    CHECK(v == 3);
    REQUIRE(eval("100 / 10 / 5", &v, &err));
    CHECK(v == 2);
}

TEST(calc_parentheses) {
    double v = 0;
    std::string err;
    REQUIRE(eval("(2 + 3) * 4", &v, &err));
    CHECK(v == 20);
    REQUIRE(eval("2 * (3 + 4) - 5", &v, &err));
    CHECK(v == 9);
}

TEST(calc_negative_numbers) {
    double v = 0;
    std::string err;
    REQUIRE(eval("-3 * -3", &v, &err));
    CHECK(v == 9);
    REQUIRE(eval("2*-3 + (-1)", &v, &err));
    CHECK(v == -7);
}

TEST(calc_decimals) {
    double v = 0;
    std::string err;
    REQUIRE(eval("7 / 2", &v, &err));
    CHECK(v == 3.5);
    REQUIRE(eval("1.25 + 2.5 * 2", &v, &err));
    CHECK(v == 6.25);
}

TEST(calc_zero_from_subexpression) {
    double v = 0;
    std::string err;
    CHECK(!eval("1 / (2 - 2)", &v, &err));
    CHECK(err == "division by zero");
    err.clear();
    CHECK(!eval("5 / -0", &v, &err));
    CHECK(err == "division by zero");
}

TEST(calc_nested_mixed_expression) {
    double v = 0;
    std::string err;
    REQUIRE(eval("18 / (2 * (1 + 2)) + 4 * 0.5", &v, &err));
    CHECK(v == 5);
    REQUIRE(eval("20 - 6 / 2 * 3", &v, &err));
    CHECK(v == 11);
}

TEST(calc_long_sum) {
    std::string expression = "1";
    for (int i = 1; i < 20000; ++i) expression += "+1";
    double v = 0;
    std::string err;
    REQUIRE(eval(expression, &v, &err));
    CHECK(v == 20000);
}

TEST(calc_deep_parentheses) {
    std::string expression(5000, '(');
    expression += "7";
    expression += std::string(5000, ')');
    double v = 0;
    std::string err;
    REQUIRE(eval(expression, &v, &err));
    CHECK(v == 7);
}

TEST(calc_bad_tokens_do_not_underflow_stacks) {
    using K = CalcToken::Kind;
    const std::vector<std::vector<CalcToken>> cases = {
        {},
        {{K::Plus, 0}},
        {{K::Number, 1}, {K::Divide, 0}},
        {{K::RParen, 0}},
        {{K::LParen, 0}, {K::Number, 1}},
        {{K::Number, 1}, {K::Number, 2}},
        {{K::LParen, 0}, {K::RParen, 0}},
        {{K::Number, 1}, {K::Plus, 0}, {K::RParen, 0}}
    };
    for (const auto& tokens : cases) {
        double v = 0;
        std::string err;
        CHECK(!calc_evaluate(tokens, &v, &err));
        CHECK(!err.empty());
    }
}

int main(int argc, char** argv) { return mt::run_all(argc, argv); }