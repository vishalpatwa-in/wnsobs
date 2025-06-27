#include "multistream-dock.h"
#include "obs-multistream.h"
#include <obs-frontend-api.h>
#include <obs-properties.h>
#include <util/platform.h>
#include <Windows.h>
#include <CommCtrl.h>
#include <string>
#include <sstream>

// Resource IDs for dialog controls
#define IDC_NAME_EDIT           1001
#define IDC_PRESET_COMBO        1002
#define IDC_URL_EDIT            1003
#define IDC_KEY_EDIT            1004
#define IDC_ENABLED_CHECK       1005
#define IDC_MAIN_ENCODER_CHECK  1006
#define IDC_BITRATE_SPIN        1007
#define IDC_OK_BUTTON           1008
#define IDC_CANCEL_BUTTON       1009

// Dialog template resource ID
#define IDD_DESTINATION_DIALOG  2001

// ============================================================================
// Win32 Helper Functions
// ============================================================================

namespace Win32Helpers {

void SetWindowText(HWND hWnd, int controlId, const std::string& text) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        ::SetWindowTextA(control, text.c_str());
    }
}

std::string GetWindowText(HWND hWnd, int controlId) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (!control) return "";
    
    char buffer[1024];
    GetWindowTextA(control, buffer, sizeof(buffer));
    return std::string(buffer);
}

void SetCheckBox(HWND hWnd, int controlId, bool checked) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        SendMessage(control, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

bool GetCheckBox(HWND hWnd, int controlId) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (!control) return false;
    
    return SendMessage(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
}

void SetSpinBox(HWND hWnd, int controlId, int value) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        SetDlgItemInt(hWnd, controlId, value, FALSE);
    }
}

int GetSpinBox(HWND hWnd, int controlId) {
    BOOL translated;
    int value = GetDlgItemInt(hWnd, controlId, &translated, FALSE);
    return translated ? value : 0;
}

void SetComboBox(HWND hWnd, int controlId, int index) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        SendMessage(control, CB_SETCURSEL, index, 0);
    }
}

int GetComboBox(HWND hWnd, int controlId) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (!control) return -1;
    
    return (int)SendMessage(control, CB_GETCURSEL, 0, 0);
}

void AddComboBoxItem(HWND hWnd, int controlId, const std::string& text) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        SendMessageA(control, CB_ADDSTRING, 0, (LPARAM)text.c_str());
    }
}

void ClearComboBox(HWND hWnd, int controlId) {
    HWND control = GetDlgItem(hWnd, controlId);
    if (control) {
        SendMessage(control, CB_RESETCONTENT, 0, 0);
    }
}

} // namespace Win32Helpers

// ============================================================================
// Stream Destination Dialog
// ============================================================================

StreamDestinationDialog::StreamDestinationDialog(HWND parent)
    : currentDest(nullptr), dialogOK(false), parentWindow(parent) {
}

StreamDestinationDialog::~StreamDestinationDialog() {
}

bool StreamDestinationDialog::ShowDialog(StreamDestination* dest) {
    currentDest = dest;
    dialogOK = false;
    
    // For now, use a simple message box approach
    // In a full implementation, you would create a proper dialog resource
    
    // Simple input simulation for demonstration
    if (dest) {
        // Edit mode - show current values
        blog(LOG_INFO, "[obs-multistream] Edit destination: %s", dest->name.c_str());
    } else {
        // Add mode - create new destination
        dialogResult = StreamDestination();
        dialogResult.name = "New Stream";
        dialogResult.url = "rtmp://live.twitch.tv/live/";
        dialogResult.key = "";
        dialogResult.enabled = true;
        dialogResult.useMainEncoder = true;
        dialogResult.bitrate = 2500;
        
        dialogOK = true;
    }
    
    return dialogOK;
}

StreamDestination StreamDestinationDialog::GetDestination() const {
    return dialogResult;
}

INT_PTR CALLBACK StreamDestinationDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    StreamDestinationDialog* dialog = nullptr;
    
    if (message == WM_INITDIALOG) {
        dialog = reinterpret_cast<StreamDestinationDialog*>(lParam);
        SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)dialog);
        dialog->InitializeDialog(hDlg);
        return TRUE;
    } else {
        dialog = reinterpret_cast<StreamDestinationDialog*>(GetWindowLongPtr(hDlg, DWLP_USER));
    }
    
    if (!dialog) return FALSE;
    
    switch (message) {
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_OK_BUTTON:
            if (dialog->ValidateAndSave(hDlg)) {
                dialog->dialogOK = true;
                EndDialog(hDlg, IDOK);
            }
            return TRUE;
            
        case IDC_CANCEL_BUTTON:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
            
        case IDC_PRESET_COMBO:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                dialog->OnPresetChanged(hDlg);
            }
            return TRUE;
        }
        break;
        
    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }
    
    return FALSE;
}

void StreamDestinationDialog::InitializeDialog(HWND hDlg) {
    PopulatePresets(hDlg);
    
    if (currentDest) {
        LoadDestination(hDlg, *currentDest);
    } else {
        // Set defaults for new destination
        Win32Helpers::SetWindowText(hDlg, IDC_NAME_EDIT, "New Stream");
        Win32Helpers::SetCheckBox(hDlg, IDC_ENABLED_CHECK, true);
        Win32Helpers::SetCheckBox(hDlg, IDC_MAIN_ENCODER_CHECK, true);
        Win32Helpers::SetSpinBox(hDlg, IDC_BITRATE_SPIN, 2500);
    }
}

void StreamDestinationDialog::LoadDestination(HWND hDlg, const StreamDestination& dest) {
    Win32Helpers::SetWindowText(hDlg, IDC_NAME_EDIT, dest.name);
    Win32Helpers::SetWindowText(hDlg, IDC_URL_EDIT, dest.url);
    Win32Helpers::SetWindowText(hDlg, IDC_KEY_EDIT, dest.key);
    Win32Helpers::SetCheckBox(hDlg, IDC_ENABLED_CHECK, dest.enabled);
    Win32Helpers::SetCheckBox(hDlg, IDC_MAIN_ENCODER_CHECK, dest.useMainEncoder);
    Win32Helpers::SetSpinBox(hDlg, IDC_BITRATE_SPIN, dest.bitrate);
}

bool StreamDestinationDialog::ValidateAndSave(HWND hDlg) {
    dialogResult.name = Win32Helpers::GetWindowText(hDlg, IDC_NAME_EDIT);
    dialogResult.url = Win32Helpers::GetWindowText(hDlg, IDC_URL_EDIT);
    dialogResult.key = Win32Helpers::GetWindowText(hDlg, IDC_KEY_EDIT);
    dialogResult.enabled = Win32Helpers::GetCheckBox(hDlg, IDC_ENABLED_CHECK);
    dialogResult.useMainEncoder = Win32Helpers::GetCheckBox(hDlg, IDC_MAIN_ENCODER_CHECK);
    dialogResult.bitrate = Win32Helpers::GetSpinBox(hDlg, IDC_BITRATE_SPIN);
    
    // Basic validation
    if (dialogResult.name.empty()) {
        MessageBoxA(hDlg, "Please enter a name for the destination.", "Validation Error", MB_OK | MB_ICONWARNING);
        return false;
    }
    
    if (dialogResult.url.empty()) {
        MessageBoxA(hDlg, "Please enter an RTMP URL.", "Validation Error", MB_OK | MB_ICONWARNING);
        return false;
    }
    
    if (dialogResult.key.empty()) {
        MessageBoxA(hDlg, "Please enter a stream key.", "Validation Error", MB_OK | MB_ICONWARNING);
        return false;
    }
    
    return true;
}

void StreamDestinationDialog::PopulatePresets(HWND hDlg) {
    Win32Helpers::ClearComboBox(hDlg, IDC_PRESET_COMBO);
    Win32Helpers::AddComboBoxItem(hDlg, IDC_PRESET_COMBO, "Custom");
    Win32Helpers::AddComboBoxItem(hDlg, IDC_PRESET_COMBO, "Twitch");
    Win32Helpers::AddComboBoxItem(hDlg, IDC_PRESET_COMBO, "YouTube");
    Win32Helpers::AddComboBoxItem(hDlg, IDC_PRESET_COMBO, "Facebook");
    Win32Helpers::AddComboBoxItem(hDlg, IDC_PRESET_COMBO, "TikTok");
    Win32Helpers::SetComboBox(hDlg, IDC_PRESET_COMBO, 0);
}

void StreamDestinationDialog::OnPresetChanged(HWND hDlg) {
    int selection = Win32Helpers::GetComboBox(hDlg, IDC_PRESET_COMBO);
    
    switch (selection) {
    case 1: // Twitch
        Win32Helpers::SetWindowText(hDlg, IDC_URL_EDIT, "rtmp://live.twitch.tv/live/");
        break;
    case 2: // YouTube
        Win32Helpers::SetWindowText(hDlg, IDC_URL_EDIT, "rtmp://a.rtmp.youtube.com/live2/");
        break;
    case 3: // Facebook
        Win32Helpers::SetWindowText(hDlg, IDC_URL_EDIT, "rtmps://live-api-s.facebook.com:443/rtmp/");
        break;
    case 4: // TikTok
        Win32Helpers::SetWindowText(hDlg, IDC_URL_EDIT, "rtmp://push.tiktokcdn.com/live/");
        break;
    default: // Custom
        break;
    }
}

// ============================================================================
// Multistream Dock
// ============================================================================

MultistreamDock::MultistreamDock() 
    : dock(nullptr), destinationList(nullptr), statusLabel(nullptr), startStopBtn(nullptr) {
}

MultistreamDock::~MultistreamDock() {
    Shutdown();
}

bool MultistreamDock::Initialize() {
    blog(LOG_INFO, "[obs-multistream] Initializing dock");
    
    // Get OBS main window as parent
    QMainWindow* mainWindow = static_cast<QMainWindow*>(obs_frontend_get_main_window());
    if (!mainWindow) {
        blog(LOG_ERROR, "[obs-multistream] Failed to get main window");
        return false;
    }
    
    // Create the dock widget
    QDockWidget* dockWidget = new QDockWidget("Multistream", mainWindow);
    
    // Create the main widget content
    QWidget* contentWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(contentWidget);
    
    // Add destination list
    destinationList = new QTextEdit();
    destinationList->setReadOnly(true);
    destinationList->setMaximumHeight(150);
    layout->addWidget(new QLabel("Stream Destinations:"));
    layout->addWidget(destinationList);
    
    // Add control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* addBtn = new QPushButton("Add Destination");
    QPushButton* editBtn = new QPushButton("Edit Selected");
    QPushButton* removeBtn = new QPushButton("Remove Selected");
    
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(removeBtn);
    layout->addLayout(buttonLayout);
    
    // Add status and start/stop
    statusLabel = new QLabel("Status: Stopped");
    layout->addWidget(statusLabel);
    
    startStopBtn = new QPushButton("Start Multistream");
    layout->addWidget(startStopBtn);
    
    // Connect button signals
    QObject::connect(addBtn, &QPushButton::clicked, [this]() { OnAddDestination(); });
    QObject::connect(editBtn, &QPushButton::clicked, [this]() { OnEditDestination(); });
    QObject::connect(removeBtn, &QPushButton::clicked, [this]() { OnRemoveDestination(); });
    QObject::connect(startStopBtn, &QPushButton::clicked, [this]() { OnStartStop(); });
    
    // Set the content widget
    dockWidget->setWidget(contentWidget);
    
    // Add dock to OBS with proper ID and title
    bool success = obs_frontend_add_dock_by_id("multistream_dock", "Multistream", dockWidget);
    if (!success) {
        blog(LOG_ERROR, "[obs-multistream] Failed to add dock to OBS");
        delete dockWidget;
        return false;
    }
    
    // Store references
    dock = dockWidget;
    
    UpdateDestinationList();
    UpdateStatus();
    
    blog(LOG_INFO, "[obs-multistream] Dock initialized successfully");
    return true;
}

void MultistreamDock::Shutdown() {
    if (dock) {
        obs_frontend_remove_dock("multistream_dock");
        dock = nullptr;
    }
}



void MultistreamDock::OnAddDestination() {
    StreamDestinationDialog dialog(nullptr);
    if (dialog.ShowDialog()) {
        StreamDestination dest = dialog.GetDestination();
        MultistreamPlugin::GetInstance()->AddDestination(dest);
        UpdateDestinationList();
    }
}

void MultistreamDock::OnEditDestination() {
    // For simplicity, just show a message for now
    MessageBoxA(nullptr, "Edit functionality would open the edit dialog for the selected destination.", 
                "Edit Destination", MB_OK | MB_ICONINFORMATION);
}

void MultistreamDock::OnRemoveDestination() {
    // For simplicity, just show a message for now
    MessageBoxA(nullptr, "Remove functionality would delete the selected destination.", 
                "Remove Destination", MB_OK | MB_ICONINFORMATION);
}

void MultistreamDock::OnStartStop() {
    MultistreamPlugin* plugin = MultistreamPlugin::GetInstance();
    
    if (plugin->IsStreaming()) {
        plugin->StopStreaming();
    } else {
        plugin->StartStreaming();
    }
    
    UpdateStatus();
}

void MultistreamDock::UpdateDestinationList() {
    if (!destinationList) return;
    
    MultistreamPlugin* plugin = MultistreamPlugin::GetInstance();
    const auto& destinations = plugin->GetDestinations();
    
    std::stringstream ss;
    for (size_t i = 0; i < destinations.size(); i++) {
        const auto& dest = destinations[i];
        ss << (i + 1) << ". " << dest.name;
        if (dest.enabled) {
            ss << " (Enabled)";
        } else {
            ss << " (Disabled)";
        }
        ss << "\n   URL: " << dest.url << "\n";
    }
    
    if (destinations.empty()) {
        ss << "No destinations configured.\nUse 'Add Destination' to get started.";
    }
    
    destinationList->setPlainText(QString::fromStdString(ss.str()));
}

void MultistreamDock::UpdateStatus() {
    if (!statusLabel || !startStopBtn) return;
    
    MultistreamPlugin* plugin = MultistreamPlugin::GetInstance();
    
    if (plugin->IsStreaming()) {
        statusLabel->setText("Status: Multistreaming ACTIVE");
        startStopBtn->setText("Stop Multistream");
    } else {
        statusLabel->setText("Status: Multistreaming STOPPED");
        startStopBtn->setText("Start Multistream");
    }
}

// Include MOC file for Qt's meta-object system
#include "multistream-dock.moc"