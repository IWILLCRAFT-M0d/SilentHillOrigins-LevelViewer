#pragma once
#include <string>
#include <vector>

void LoadTexturesFromTxd(const std::string& txdPath, const std::vector<std::string>& allowedNames, bool fallback = false);
void LoadGeometry(const std::string& geomPath);
void LoadLevel(const std::string& meshContainerPath, const std::vector<std::string>& txdPaths);
void ParseContainerStructure(const std::string& path);
