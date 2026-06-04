#ifndef OPENAI_CLIENT_H
#define OPENAI_CLIENT_H

#include <string>
#include <vector>

namespace planner {
struct Parameter {
    std::string name;
    std::string type;
    std::string description;
    std::vector<std::string> allowableValues;
    bool isRequired;
};
struct Tool {
    std::string name;
    std::string description;
    std::vector<Parameter> parameters;  
};
enum ResponseType {
    MESSAGE,
    TOOL_CALL
};
struct Argument {
    std::string name;
    std::string value;
};
struct ToolCall {
    std::string id;
    std::string name; 
    std::vector<Argument> args;
};
struct Message {
    std::string role;
    std::string content;
    std::vector<ToolCall> toolCalls;
    std::string toolCallID;  // tool_call_id for role="tool" messages
};
struct Response {
    std::string message;
    ResponseType type = TOOL_CALL;
    std::vector<ToolCall> toolCalls;
};
      

class OpenAIClient {
public:
    OpenAIClient(const std::string& apiKey, const std::string& model, const std::string& apiUrl);

    Response chat(const std::vector<Message>& messages, const std::vector<Tool>& tools);

private:
    std::string apiKey_;
    std::string model_;
    std::string apiUrl_;

    static size_t writeCallback(void* contents, size_t size, size_t nmemb, std::string* output);
};

} // namespace planner

#endif // OPENAI_CLIENT_H
