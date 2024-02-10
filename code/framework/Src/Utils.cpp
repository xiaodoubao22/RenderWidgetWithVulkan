#include "Utils.h"
#include "Log.h"

#include <iostream>
#include <unordered_set>
#include <fstream>

namespace utils {
std::vector<const char*> StringToCstr(const std::vector<std::string>& strings) {
	std::vector<const char*> res;
	for (int i = 0; i < strings.size(); i++) {
		int length = strings[i].size() + 1;
		const char* temp = new char[length];
		memcpy_s((void*)temp, length, strings[i].data(), length);
		res.push_back(temp);
	}
	return res;
}

std::vector<std::string> CstrToString(const std::vector<const char*>& cstrs) {
	std::vector<std::string> res;
	for (int i = 0; i < cstrs.size(); i++) {
		res.push_back(std::string(cstrs[i]));
	}
	return res;
}

void PrintStringList(const std::vector<std::string>& stringList, const std::string& head) {
	LOGI("%s", head.c_str());
	for (int i = 0; i < stringList.size(); i++) {
		LOGI("  - %s", stringList[i].c_str());
	}
	LOGI("  - end");
	return;
}

void PrintStringList(const std::vector<const char*>& stringList, const std::string& head) {
	LOGI("%s", head.c_str());
	for (int i = 0; i < stringList.size(); i++) {
		LOGI("  - %s", stringList[i]);
	}
	LOGI("  - end");
	return;
}

bool CheckSupported(const std::vector<const char*>& componentList, const std::vector<const char*>& availableList) {
	std::vector<std::string> componentStrList = CstrToString(componentList);

	std::unordered_set<std::string> availableSet;
	for (const char* name : availableList) {
		availableSet.insert(std::string(name));
	}

	for (const std::string& name : componentStrList) {
		if (availableSet.find(name) == availableSet.end()) {
			return false;
		}
	}

	return true;
}

std::vector<char> ReadFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		LOGE("filed to open shader file: %s", filename.c_str());
		throw std::runtime_error("filed to open shader file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}
}