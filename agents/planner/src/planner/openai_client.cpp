#include "openai_client.h"
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <iostream>

namespace planner {

OpenAIClient::OpenAIClient(const std::string& apiKey, const std::string& model, const std::string& apiUrl)
    : apiKey_(apiKey), model_(model), apiUrl_(apiUrl) {}

size_t OpenAIClient::writeCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    output->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

Response OpenAIClient::chat(const std::vector<Message>& messages, const std::vector<Tool>& tools) {
    nlohmann::json payloadJSON;
    payloadJSON["model"] = model_;

    payloadJSON["messages"] = nlohmann::json::array();
    for (const auto& msg : messages) {
        payloadJSON["messages"].push_back({{"role", msg.role}, {"content", msg.content}});
        if (msg.role == "tool") payloadJSON["messages"].back()["tool_call_id"] = msg.toolCallID;
        if (!msg.toolCalls.empty()) { 
            payloadJSON["messages"].back()["tool_calls"] = nlohmann::json::array();
            for (const auto& toolCall : msg.toolCalls) {
                nlohmann::json t = {
                    {"id", toolCall.id},
                    {"type", "function"},
                    {"function", {
                        {"name", toolCall.name}   
                    }} 
                };    
                nlohmann::json argsObj;
                if (!toolCall.args.empty()) {
                      for (const auto& arg : toolCall.args) {
                          argsObj[arg.name] = arg.value;
                      }
                }
                t["function"]["arguments"] = argsObj.dump();
                payloadJSON["messages"].back()["tool_calls"].push_back(t);
            }
        }
    }

    if (!tools.empty()) {
        payloadJSON["tools"] = nlohmann::json::array();
        for (const auto& tool : tools) {
            nlohmann::json t = {
                {"type", "function"},
                {"function", {
                    {"name", tool.name},
                    {"description", tool.description},
                    {"parameters", {
                        {"type", "object"}
                    }} 
                }} 
            };
            t["function"]["parameters"]["properties"] = nlohmann::json::object();
            t["function"]["parameters"]["required"] = nlohmann::json::array();
            for (const auto& parameter : tool.parameters) {
                nlohmann::json p = {
                    {"type", parameter.type},
                    {"description", parameter.description}
                };
                if (!parameter.allowableValues.empty()) {
                    p["enum"] = nlohmann::json::array();
                    for (const auto& allowableValue : parameter.allowableValues) {
                        p["enum"].push_back(allowableValue);        
                    }
                }
                t["function"]["parameters"]["properties"][parameter.name] = p;
                if (parameter.isRequired) {
                    t["function"]["parameters"]["required"].push_back(parameter.name);
                }
            }
            payloadJSON["tools"].push_back(t);
        }
    }

    std::string body = payloadJSON.dump();
    std::string authHeader = "Authorization: Bearer " + apiKey_;
    std::string responseString;
    //DEBUG - print the body JSON
    //std::cout << "Debug:\n" << body << "\n === \n";
    CURL* curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize CURL");

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, apiUrl_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, static_cast<long>(body.size()));
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

    CURLcode rc = curl_easy_perform(curl);

    long httpCode = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (rc != CURLE_OK) {
        throw std::runtime_error(std::string("CURL error: ") + curl_easy_strerror(rc));
    }
    if (httpCode < 200 || httpCode >= 300) {
        throw std::runtime_error("HTTP error " + std::to_string(httpCode) + ": " + responseString);
    }

    auto responseJSON = nlohmann::json::parse(responseString);
    if (responseJSON.contains("error")) {
        const auto& err = responseJSON["error"];
        std::string msg = err.contains("message") && err["message"].is_string()
            ? err["message"].get<std::string>()
            : err.dump();
        throw std::runtime_error("OpenAI API error: " + msg);
    }

    if (!responseJSON.contains("choices") || responseJSON["choices"].empty()) {
        throw std::runtime_error("OpenAI API returned no choices: " + responseJSON.dump());
    }

    Response response;
    const auto& choice = responseJSON["choices"][0];
    if (choice["finish_reason"].get<std::string>() == "tool_calls") {
        response.type = TOOL_CALL;
        for (const auto& toolCallJSON : choice["message"]["tool_calls"]) {
            ToolCall tc;
            tc.id = toolCallJSON["id"].get<std::string>();
            tc.name = toolCallJSON["function"]["name"].get<std::string>();
            auto argsJSON = nlohmann::json::parse(toolCallJSON["function"]["arguments"].get<std::string>());
            for (const auto& [key, value] : argsJSON.items()) {
                Argument arg;
                arg.name = key;
                arg.value = value.is_string() ? value.get<std::string>() : value.dump();
                tc.args.push_back(arg);
            }
            response.toolCalls.push_back(tc);
        }
    } else {
        const auto& content = choice["message"]["content"];
        response.message = content.is_string() ? content.get<std::string>() : "";
        response.type = MESSAGE;
    }
    return response;
}
} //namespace planner
