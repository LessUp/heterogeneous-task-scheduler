#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path repo_root() {
    return std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
}

std::string read_text(const std::filesystem::path &path) {
    std::ifstream input(path);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // namespace

TEST(RepositorySurfaceTest, DoesNotShipRepositoryLevelAiGovernanceFrameworks) {
    const auto root = repo_root();
    const std::vector<std::filesystem::path> forbidden_paths = {
        root / "AGENTS.md", root / "CLAUDE.md", root / ".github" / "copilot-instructions.md",
        root / ".claude",   root / ".opencode", root / "openspec",
        root / "specs",
    };

    for (const auto &path : forbidden_paths) {
        EXPECT_FALSE(std::filesystem::exists(path)) << path.string();
    }
}

TEST(RepositorySurfaceTest, PublicDocsDoNotReferenceOpenSpecWorkflow) {
    const auto root = repo_root();
    const std::vector<std::filesystem::path> public_docs = {
        root / "README.md",
        root / "README.zh-CN.md",
        root / "CONTRIBUTING.md",
        root / "CHANGELOG.md",
        root / ".github" / "ISSUE_TEMPLATE" / "feature_request.md",
        root / ".github" / "PULL_REQUEST_TEMPLATE.md",
    };
    const std::vector<std::string> forbidden_tokens = {
        "OpenSpec", "openspec/", "/opsx:", "spec-driven", "/review",
    };

    for (const auto &doc : public_docs) {
        const std::string content = read_text(doc);
        for (const auto &token : forbidden_tokens) {
            EXPECT_EQ(content.find(token), std::string::npos)
                << "Found forbidden token '" << token << "' in " << doc.string();
        }
    }
}

TEST(RepositorySurfaceTest, PagesWorkflowPublishesOnlyStableProjectFiles) {
    const auto root = repo_root();
    const auto changelog = root / "CHANGELOG.md";
    const auto docs_workflow = root / ".github" / "workflows" / "docs.yml";
    const std::string workflow = read_text(docs_workflow);

    EXPECT_TRUE(std::filesystem::exists(changelog));
    EXPECT_EQ(workflow.find("cp CHANGELOG.md website/.vitepress/dist/CHANGELOG.md"),
              std::string::npos);
}
