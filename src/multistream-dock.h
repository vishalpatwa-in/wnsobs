#pragma once

#include <obs-frontend-api.h>
#include <obs-properties.h>
#include <Windows.h>
#include <string>

// Check if Qt is available
#ifdef QT_CORE_LIB
// Qt includes for dock widget
#include <QDockWidget>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QObject>
#include <QString>
#define HAVE_QT
#endif

struct StreamDestination;

// Simple Win32-based configuration dialog
class StreamDestinationDialog {
public:
    StreamDestinationDialog(HWND parent = nullptr);
    ~StreamDestinationDialog();
    
    // Show dialog and return true if OK was pressed
    bool ShowDialog(StreamDestination* dest = nullptr);
    
    // Get the configured destination after dialog closes
    StreamDestination GetDestination() const;

private:
    static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    
    void InitializeDialog(HWND hDlg);
    void LoadDestination(HWND hDlg, const StreamDestination& dest);
    bool ValidateAndSave(HWND hDlg);
    void PopulatePresets(HWND hDlg);
    void OnPresetChanged(HWND hDlg);
    
    StreamDestination* currentDest;
    StreamDestination dialogResult;
    bool dialogOK;
    HWND parentWindow;
};

// OBS Frontend dock implementation
#ifdef HAVE_QT
class MultistreamDock : public QObject {
    Q_OBJECT
    
public:
    MultistreamDock();
    ~MultistreamDock();
    
    bool Initialize();
    void Shutdown();
    
private slots:
    void OnAddDestination();
    void OnEditDestination();
    void OnRemoveDestination();
    void OnStartStop();
    
private:
    void UpdateDestinationList();
    void UpdateStatus();
    
    QDockWidget* dock;
    QTextEdit* destinationList;
    QLabel* statusLabel;
    QPushButton* startStopBtn;
};
#else
// Fallback implementation for builds without Qt
class MultistreamDock {
public:
    MultistreamDock();
    ~MultistreamDock();
    
    bool Initialize();
    void Shutdown();
    
private:
    // Create OBS properties for the dock
    static obs_properties_t* GetProperties(void* data);
    static void OnAddDestination(obs_properties_t* props, obs_property_t* property, void* data);
    static void OnEditDestination(obs_properties_t* props, obs_property_t* property, void* data);
    static void OnRemoveDestination(obs_properties_t* props, obs_property_t* property, void* data);
    static void OnStartStop(obs_properties_t* props, obs_property_t* property, void* data);
    static void OnRefresh(obs_properties_t* props, obs_property_t* property, void* data);
    
    void UpdateDestinationList();
    void UpdateStatus();
    
    obs_properties_t* properties;
    obs_data_t* settings;
};
#endif

// Helper functions for Win32 dialogs
namespace Win32Helpers {
    void SetWindowText(HWND hWnd, int controlId, const std::string& text);
    std::string GetWindowText(HWND hWnd, int controlId);
    void SetCheckBox(HWND hWnd, int controlId, bool checked);
    bool GetCheckBox(HWND hWnd, int controlId);
    void SetSpinBox(HWND hWnd, int controlId, int value);
    int GetSpinBox(HWND hWnd, int controlId);
    void SetComboBox(HWND hWnd, int controlId, int index);
    int GetComboBox(HWND hWnd, int controlId);
    void AddComboBoxItem(HWND hWnd, int controlId, const std::string& text);
    void ClearComboBox(HWND hWnd, int controlId);
} 