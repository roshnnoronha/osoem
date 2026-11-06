#include "reviewer_tools.h"
#include <sstream>
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <algorithm>
#include <cctype>
#include <iostream>

namespace fs = std::filesystem;

namespace reviewer {

ReviewerTools::ReviewerTools(data::Database& db, client::OpenAIClient& client,
                             const std::string& specsDir)
    : db_(db), specsDir_(specsDir), specIndexer_(specsDir, client) {}

void ReviewerTools::setWorkingActivity(int activityId) {
    workingActivityID_ = activityId;
}

std::vector<std::pair<int, std::string>> ReviewerTools::getActivityList() {
    auto res = db_.executeQuery(
        "SELECT a.activityid, p.projectname, a.activityname "
        "FROM Activities a "
        "JOIN ActivitySubcategories sc ON a.subcategoryid = sc.subcategoryid "
        "JOIN ActivityCategories c ON sc.categoryid = c.categoryid "
        "JOIN Projects p ON c.projectid = p.projectid "
        "ORDER BY p.projectname, a.activityname"
    );
    std::vector<std::pair<int, std::string>> activities;
    while (res->next()) {
        std::string display = res->getString("projectname") + " / " + res->getString("activityname");
        activities.emplace_back(res->getInt("activityid"), display);
    }
    return activities;
}

std::vector<client::Tool> ReviewerTools::getTools() {
    std::vector<client::Tool> toolList;

    toolList.push_back({
        "get_working_activity",
        "Returns the current working activity, including its project, category, subcategory, and description.",
        {}
    });
    toolList.push_back({
        "get_artefacts_in_activity",
        "Returns all artefacts linked to the working activity, including their type, title, and data fields.",
        {}
    });
    toolList.push_back({
        "read_document",
        "Reads the file content of a document artefact. The artefact must have a data field whose title contains 'path' with the file path as its value.",
        {
            {"artefact_id", "integer", "The ID of the document artefact to read.", {}, true}
        }
    });
    toolList.push_back({
        "get_spec_list",
        "Returns the list of available specification files.",
        {}
    });
    toolList.push_back({
        "read_spec",
        "Reads the content of a specification file.",
        {
            {"filename", "string", "The filename of the specification to read (as returned by get_spec_list).", {}, true}
        }
    });
    toolList.push_back({
        "search_specs",
        "Semantically searches all specification files for sections relevant to a query. "
        "Returns the top 5 most relevant chunks with file name and section heading. "
        "Use this first to find specific topics; use read_spec only when the full file is needed.",
        {
            {"query", "string",
             "A descriptive search query (e.g. 'weld filler metal CVN toughness requirements', "
             "'bolt pre-tensioning', 'deflection limits for floor beams').",
             {}, true}
        }
    });
    toolList.push_back({
        "add_review_comment",
        "Creates a Review Comment artefact linked to the working activity recording a non-compliance finding.",
        {
            {"title", "string", "A short title for the comment (e.g. the document title and section).", {}, true},
            {"location", "string", "The location in the document where the issue was found (e.g. 'Section 3.2, Table 1').", {}, true},
            {"issue", "string", "Description of the non-compliance or missing requirement.", {}, true},
            {"spec_reference", "string", "The specification clause or section that is not satisfied.", {}, true},
            {"severity", "string", "Severity of the finding.", {"Minor", "Major", "Critical"}, true}
        }
    });

    return toolList;
}

std::string ReviewerTools::callTool(const client::ToolCall& toolCall) {
    if (toolCall.name == "get_working_activity") {
        std::cout << "Fetching working activity ...\n";
        return getWorkingActivity();
    }
    if (toolCall.name == "get_artefacts_in_activity") {
        std::cout << "Fetching artefacts for activity ...\n";
        return getArtefactsInActivity(workingActivityID_);
    }
    if (toolCall.name == "read_document") {
        int artefactId = 0;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "artefact_id") artefactId = std::stoi(arg.value);
        }
        std::cout << "Reading document artefact " << artefactId << " ...\n";
        return readDocument(artefactId);
    }
    if (toolCall.name == "get_spec_list") {
        std::cout << "Listing specification files ...\n";
        return getSpecList();
    }
    if (toolCall.name == "read_spec") {
        std::string filename;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "filename") filename = arg.value;
        }
        std::cout << "Reading specification: " << filename << " ...\n";
        return readSpec(filename);
    }
    if (toolCall.name == "search_specs") {
        std::string query;
        for (const auto& arg : toolCall.args)
            if (arg.name == "query") query = arg.value;
        std::cout << "Searching specs: " << query << " ...\n";
        return specIndexer_.search(query);
    }
    if (toolCall.name == "add_review_comment") {
        std::string title, location, issue, specReference, severity;
        for (const auto& arg : toolCall.args) {
            if (arg.name == "title")          title = arg.value;
            else if (arg.name == "location")       location = arg.value;
            else if (arg.name == "issue")          issue = arg.value;
            else if (arg.name == "spec_reference") specReference = arg.value;
            else if (arg.name == "severity")       severity = arg.value;
        }
        std::cout << "Adding review comment ...\n";
        return addReviewComment(workingActivityID_, title, location, issue, specReference, severity);
    }
    throw std::runtime_error("Unknown tool: " + toolCall.name);
}

std::string ReviewerTools::getWorkingActivity() {
    if (workingActivityID_ == 0) return "No working activity is currently set.";
    auto stmt = db_.prepareStatement(
        "SELECT a.activityid, a.activityname, "
        "COALESCE(a.activitydescription, '') AS activitydescription, "
        "p.projectid, p.projectname, "
        "c.categoryname, sc.subcategoryname, "
        "CONCAT(e.firstname, ' ', e.lastname) AS managername "
        "FROM Activities a "
        "JOIN ActivitySubcategories sc ON a.subcategoryid = sc.subcategoryid "
        "JOIN ActivityCategories c ON sc.categoryid = c.categoryid "
        "JOIN Projects p ON c.projectid = p.projectid "
        "JOIN Employees e ON a.activitymanagerid = e.employeeid "
        "WHERE a.activityid = ?"
    );
    stmt->setInt(1, workingActivityID_);
    auto res = stmt->executeQuery();
    if (!res->next()) return "Error: working activity ID " + std::to_string(workingActivityID_) + " not found.";
    std::ostringstream ss;
    ss << "## Current Working Activity\n";
    ss << "| Field | Value |\n";
    ss << "| --- | --- |\n";
    ss << "| Activity ID | " << res->getInt("activityid") << " |\n";
    ss << "| Activity Name | " << res->getString("activityname") << " |\n";
    ss << "| Project | " << res->getString("projectname") << " (ID: " << res->getInt("projectid") << ") |\n";
    ss << "| Category | " << res->getString("categoryname") << " |\n";
    ss << "| Subcategory | " << res->getString("subcategoryname") << " |\n";
    ss << "| Manager | " << res->getString("managername") << " |\n";
    ss << "| Description | " << res->getString("activitydescription") << " |\n";
    return ss.str();
}

std::string ReviewerTools::getArtefactsInActivity(int activityId) {
    if (activityId == 0) return "Error: no working activity set.";
    auto stmt = db_.prepareStatement(
        "SELECT ar.artefactid, at.artefactname AS artefacttype, ar.artefacttitle, "
        "COALESCE(adf.fieldtitle, '') AS fieldtitle, "
        "COALESCE(ad.value, '') AS fieldvalue "
        "FROM ArtefactToActivityLink al "
        "JOIN Artefacts ar ON al.artefactid = ar.artefactid "
        "JOIN ArtefactTypes at ON ar.artefacttypeid = at.artefacttypeid "
        "LEFT JOIN ArtefactData ad ON ar.artefactid = ad.artefactid "
        "LEFT JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid "
        "WHERE al.activityid = ? "
        "ORDER BY ar.artefactid, adf.fieldtitle"
    );
    stmt->setInt(1, activityId);
    auto res = stmt->executeQuery();

    std::ostringstream ss;
    ss << "## Artefacts in Activity\n";
    int lastId = -1;
    bool hasRows = false;
    while (res->next()) {
        hasRows = true;
        int id = res->getInt("artefactid");
        if (id != lastId) {
            ss << "\n### Artefact ID " << id << "\n";
            ss << "- **Type:** " << res->getString("artefacttype") << "\n";
            ss << "- **Title:** " << res->getString("artefacttitle") << "\n";
            lastId = id;
        }
        std::string field = res->getString("fieldtitle");
        std::string value = res->getString("fieldvalue");
        if (!field.empty()) {
            ss << "- **" << field << ":** " << value << "\n";
        }
    }
    if (!hasRows) ss << "No artefacts linked to this activity.\n";
    return ss.str();
}

std::string ReviewerTools::readDocument(int artefactId) {
    auto stmt = db_.prepareStatement(
        "SELECT adf.fieldtitle, ad.value "
        "FROM ArtefactData ad "
        "JOIN ArtefactDataFields adf ON ad.artefactdatafieldid = adf.artefactdatafieldid "
        "WHERE ad.artefactid = ?"
    );
    stmt->setInt(1, artefactId);
    auto res = stmt->executeQuery();

    std::string filePath;
    while (res->next()) {
        std::string title = res->getString("fieldtitle");
        std::string titleLower = title;
        std::transform(titleLower.begin(), titleLower.end(), titleLower.begin(), ::tolower);
        if (titleLower.find("path") != std::string::npos) {
            filePath = res->getString("value");
            break;
        }
    }

    if (filePath.empty()) {
        return "Error: artefact " + std::to_string(artefactId) + " has no field containing 'path' with a file location.";
    }

    std::ifstream file(filePath);
    if (!file.is_open()) {
        return "Error: could not open file at path: " + filePath;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string content = ss.str();

    const size_t maxLen = 100000;
    if (content.size() > maxLen) {
        content = content.substr(0, maxLen) + "\n\n[... content truncated at 100,000 characters ...]";
    }
    return "## Document Content\nFile: " + filePath + "\n\n" + content;
}

std::string ReviewerTools::getSpecList() {
    std::ostringstream ss;
    ss << "## Available Specifications\n";

    fs::path dir(specsDir_);
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        return "Error: specifications directory not found at: " + specsDir_;
    }

    bool found = false;
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            ss << "- " << entry.path().filename().string() << "\n";
            found = true;
        }
    }
    if (!found) ss << "No specification files found.\n";
    return ss.str();
}

std::string ReviewerTools::readSpec(const std::string& filename) {
    fs::path specPath = fs::path(specsDir_) / filename;

    // Prevent directory traversal
    fs::path canonical;
    try {
        canonical = fs::canonical(specPath);
    } catch (...) {
        return "Error: specification file not found: " + filename;
    }
    fs::path specsCanonical = fs::weakly_canonical(fs::path(specsDir_));
    std::string canonStr = canonical.string();
    std::string dirStr = specsCanonical.string();
    if (canonStr.find(dirStr) != 0) {
        return "Error: access denied — file is outside the specifications directory.";
    }

    std::ifstream file(canonical);
    if (!file.is_open()) {
        return "Error: could not open specification file: " + filename;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    return "## Specification: " + filename + "\n\n" + ss.str();
}

int ReviewerTools::findOrCreateCommentArtefactType(int projectId) {
    auto check = db_.prepareStatement(
        "SELECT artefacttypeid FROM ArtefactTypes "
        "WHERE projectid = ? AND artefactname = 'Review Comment'"
    );
    check->setInt(1, projectId);
    auto res = check->executeQuery();
    if (res->next()) return res->getInt("artefacttypeid");

    auto ins = db_.prepareStatement(
        "INSERT INTO ArtefactTypes (projectid, artefactname, artefactdescription) "
        "VALUES (?, 'Review Comment', 'Document review comment recording a non-compliance finding')"
    );
    ins->setInt(1, projectId);
    ins->executeUpdate();

    auto id = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    id->next();
    return id->getInt("id");
}

int ReviewerTools::findOrCreateField(int artefactTypeId, const std::string& fieldTitle) {
    auto check = db_.prepareStatement(
        "SELECT artefactdatafieldid FROM ArtefactDataFields "
        "WHERE artefacttypeid = ? AND fieldtitle = ?"
    );
    check->setInt(1, artefactTypeId);
    check->setString(2, fieldTitle);
    auto res = check->executeQuery();
    if (res->next()) return res->getInt("artefactdatafieldid");

    auto ins = db_.prepareStatement(
        "INSERT INTO ArtefactDataFields (artefacttypeid, fieldtitle, valuetype) VALUES (?, ?, 0)"
    );
    ins->setInt(1, artefactTypeId);
    ins->setString(2, fieldTitle);
    ins->executeUpdate();

    auto id = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    id->next();
    return id->getInt("id");
}

std::string ReviewerTools::addReviewComment(int activityId, const std::string& title,
    const std::string& location, const std::string& issue,
    const std::string& specReference, const std::string& severity) {

    if (activityId == 0) return "Error: no working activity set.";

    // Find the project for this activity
    auto projStmt = db_.prepareStatement(
        "SELECT p.projectid FROM Activities a "
        "JOIN ActivitySubcategories sc ON a.subcategoryid = sc.subcategoryid "
        "JOIN ActivityCategories c ON sc.categoryid = c.categoryid "
        "JOIN Projects p ON c.projectid = p.projectid "
        "WHERE a.activityid = ?"
    );
    projStmt->setInt(1, activityId);
    auto projRes = projStmt->executeQuery();
    if (!projRes->next()) return "Error: activity ID " + std::to_string(activityId) + " not found.";
    int projectId = projRes->getInt("projectid");

    int typeId = findOrCreateCommentArtefactType(projectId);

    int fieldLocation  = findOrCreateField(typeId, "Location");
    int fieldIssue     = findOrCreateField(typeId, "Issue");
    int fieldSpecRef   = findOrCreateField(typeId, "Specification Reference");
    int fieldSeverity  = findOrCreateField(typeId, "Severity");

    // Create artefact
    auto artStmt = db_.prepareStatement(
        "INSERT INTO Artefacts (artefacttypeid, artefacttitle) VALUES (?, ?)"
    );
    artStmt->setInt(1, typeId);
    artStmt->setString(2, title);
    artStmt->executeUpdate();

    auto artIdRes = db_.executeQuery("SELECT LAST_INSERT_ID() AS id");
    artIdRes->next();
    int artefactId = artIdRes->getInt("id");

    // Link to activity
    auto linkStmt = db_.prepareStatement(
        "INSERT INTO ArtefactToActivityLink (activityid, artefactid) VALUES (?, ?)"
    );
    linkStmt->setInt(1, activityId);
    linkStmt->setInt(2, artefactId);
    linkStmt->executeUpdate();

    // Insert field values
    auto dataStmt = db_.prepareStatement(
        "INSERT INTO ArtefactData (artefactid, artefactdatafieldid, value) VALUES (?, ?, ?)"
    );
    struct { int fieldId; const std::string& value; } fields[] = {
        {fieldLocation, location},
        {fieldIssue, issue},
        {fieldSpecRef, specReference},
        {fieldSeverity, severity}
    };
    for (const auto& f : fields) {
        dataStmt->setInt(1, artefactId);
        dataStmt->setInt(2, f.fieldId);
        dataStmt->setString(3, f.value);
        dataStmt->executeUpdate();
    }

    std::ostringstream ss;
    ss << "Review comment created. Artefact ID: " << artefactId
       << ", Title: " << title
       << ", Severity: " << severity;
    return ss.str();
}

} // namespace reviewer
