#pragma once
#include <string>
#include <vector>
#include <filesystem>

struct FileBrowserState {
    std::string currentPath;
    std::vector<std::filesystem::path> entries;
    std::vector<std::string> selectedTxds;
    std::string selectedMeshContainer;
    bool showBrowser = false;
    bool selectingMesh = true;
    std::string errorMessage;
    // Deferred navigation: set inside the render loop, executed after it
    std::string pendingNavigate;
    bool pendingOpenTxd = false;

    void Open(bool forMesh);
    void RefreshEntries();
    void Render();
};

extern FileBrowserState g_FileBrowser;

// Structure / hierarchy panel
void RenderStructureWindow();

// TXD texture browser panel
void RenderTxdWindow();
