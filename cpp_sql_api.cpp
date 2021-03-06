// cpp_sql_api.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "cpp_sql_api.h"

#include <algorithm>
#include <iostream>

CURL* cpp_sql_api::curl_handle = NULL;

cpp_sql_api::cpp_sql_api(std::string  url)
{
	this->script_url = url;
}

cpp_sql_api::cpp_sql_api()
{
	init();
}

void cpp_sql_api::init()
{
	if (curl_handle == nullptr)
	{
		curl_global_init(CURL_GLOBAL_DEFAULT);
		cpp_sql_api::curl_handle = curl_easy_init();
	}
}

void cpp_sql_api::cleanup()
{
	if (curl_handle != nullptr)
	{
		curl_easy_cleanup(cpp_sql_api::curl_handle);
		curl_global_cleanup();
		curl_handle = nullptr;
	}
}

void cpp_sql_api::set_script_url(std::string  url)
{
	this->script_url = url;
}

std::string cpp_sql_api::sql_query(std::string query_, std::string db_, std::string fetch_cmd_)
{
	std::string str_result;

	std::replace(query_.begin(), query_.end(), ' ', '+');

	std::string q_str = this->script_url + "?db=" + db_ + "&query=" + query_ + "&fetch_cmd=" + fetch_cmd_;
	cpp_sql_api::curl_api(q_str, str_result);

	return str_result;
}

void
cpp_sql_api::curl_api(std::string &url, std::string &result_json) {
	std::vector <std::string> v;
	std::string action = "GET";
	std::string post_data = "";
	cpp_sql_api::curl_api_with_header(url, result_json, v, post_data, action);
}

void
cpp_sql_api::curl_api_with_header(std::string &url, std::string &result_json, std::vector <std::string> &extra_http_header, std::string &post_data, std::string &action)
{

	CURLcode res;

	if (cpp_sql_api::curl_handle) {
		curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_URL, url.c_str());
		curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_WRITEFUNCTION, cpp_sql_api::curl_cb);
		curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_WRITEDATA, &result_json);
		curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_ENCODING, "gzip");

		if (extra_http_header.size() > 0) {
			struct curl_slist *chunk = NULL;
			for (int i = 0; i < extra_http_header.size(); i++) {
				chunk = curl_slist_append(chunk, extra_http_header[i].c_str());
			}
			curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_HTTPHEADER, chunk);
		}

		if (post_data.size() > 0 || action == "POST" || action == "PUT" || action == "DELETE") {
			if (action == "PUT" || action == "DELETE") {
				curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_CUSTOMREQUEST, action.c_str());
			}
			curl_easy_setopt(cpp_sql_api::curl_handle, CURLOPT_POSTFIELDS, post_data.c_str());
		}

		res = curl_easy_perform(cpp_sql_api::curl_handle);

		if (res != CURLE_OK) {
			std::cerr << curl_easy_strerror(res) << std::endl;;
		}
	}

}

size_t
cpp_sql_api::curl_cb(void *content, size_t size, size_t nmemb, std::string *buffer)
{

	buffer->append((char*)content, size*nmemb);

	return size * nmemb;
}

cpp_sql_api::~cpp_sql_api()
{
	cleanup();
}

