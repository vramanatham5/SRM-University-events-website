
#include "supabase_client.h"

#include <curl/curl.h>

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace
{

    std::string trimValue(const std::string &value)
    {
        const auto first = value.find_first_not_of(" \t\r\n");
        if (first == std::string::npos)
            return "";

        const auto last = value.find_last_not_of(" \t\r\n");
        return value.substr(first, last - first + 1);
    }

    std::string readEnvValue(const std::string &key)
    {
        std::ifstream file(".env");
        if (!file)
            return "";

        std::string line;

        while (std::getline(file, line))
        {
            line = trimValue(line);

            if (line.empty() || line[0] == '#')
                continue;

            const auto separator = line.find('=');
            if (separator == std::string::npos)
                continue;

            const std::string name =
                trimValue(line.substr(0, separator));

            if (name == key)
            {
                std::string value =
                    trimValue(line.substr(separator + 1));

                if (value.size() >= 2 &&
                    ((value.front() == '"' && value.back() == '"') ||
                     (value.front() == '\'' && value.back() == '\'')))
                {
                    value = value.substr(1, value.size() - 2);
                }

                return value;
            }
        }

        return "";
    }

    size_t receiveResponse(
        char *contents,
        size_t size,
        size_t count,
        void *output)
    {
        const size_t total = size * count;

        auto *response = static_cast<std::string *>(output);
        response->append(contents, total);

        return total;
    }

} // namespace

SupabaseClient::SupabaseClient()
{
    project_url = readEnvValue("SUPABASE_URL");
    publishable_key = readEnvValue("SUPABASE_PUBLISHABLE_KEY");

    while (!project_url.empty() && project_url.back() == '/')
    {
        project_url.pop_back();
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
}

bool SupabaseClient::isConfigured() const
{
    return !project_url.empty() && !publishable_key.empty();
}

SupabaseResponse SupabaseClient::request(
    const std::string &method,
    const std::string &endpoint,
    const std::string &body,
    const std::string &access_token)
{
    SupabaseResponse result;

    if (!isConfigured())
    {
        result.error = "Supabase configuration is missing.";
        return result;
    }

    CURL *curl = curl_easy_init();

    if (!curl)
    {
        result.error = "Unable to initialize libcurl.";
        return result;
    }

    std::string url = project_url;

    if (endpoint.empty() || endpoint.front() != '/')
    {
        url += "/";
    }

    url += endpoint;

    const std::string bearer_token =
        access_token.empty() ? publishable_key : access_token;

    struct curl_slist *headers = nullptr;

    headers = curl_slist_append(
        headers,
        ("apikey: " + publishable_key).c_str());

    headers = curl_slist_append(
        headers,
        ("Authorization: Bearer " + bearer_token).c_str());

    headers = curl_slist_append(
        headers,
        "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, receiveResponse);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result.body);

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);

    if (method == "POST" || method == "PATCH" ||
        method == "PUT")
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,
                         static_cast<long>(body.size()));
    }
    else if (method == "DELETE")
    {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    else if (method != "GET")
    {
        result.error = "Unsupported HTTP method.";
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return result;
    }

    const CURLcode code = curl_easy_perform(curl);

    if (code != CURLE_OK)
    {
        result.error = curl_easy_strerror(code);
    }
    else
    {
        curl_easy_getinfo(
            curl,
            CURLINFO_RESPONSE_CODE,
            &result.status_code);
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return result;
}
