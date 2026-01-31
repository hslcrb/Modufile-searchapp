#include <wx/wx.h>
#include <wx/listctrl.h>
#include "FileEngine.h"

class ModufileFrame : public wxFrame {
public:
    ModufileFrame() : wxFrame(NULL, wxID_ANY, "Modufile", wxDefaultPosition, wxSize(800, 600)) {
        wxPanel* panel = new wxPanel(this);
        wxBoxSizer* vbox = new wxBoxSizer(wxVERTICAL);

        // Header
        wxBoxSizer* hbox1 = new wxBoxSizer(wxHORIZONTAL);
        m_searchCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_searchCtrl->SetHint("Search files...");
        
        m_smartMatchCheck = new wxCheckBox(panel, wxID_ANY, "Smart Match (Al-Jal-Ttak)");
        m_refreshBtn = new wxButton(panel, wxID_ANY, "Refresh Index");
        
        hbox1->Add(m_searchCtrl, 1, wxEXPAND | wxRIGHT, 8);
        hbox1->Add(m_smartMatchCheck, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 8);
        hbox1->Add(m_refreshBtn, 0, wxALIGN_CENTER_VERTICAL);
        
        vbox->Add(hbox1, 0, wxEXPAND | wxALL, 10);

        // Status
        m_statusText = new wxStaticText(panel, wxID_ANY, "Ready");
        vbox->Add(m_statusText, 0, wxLEFT | wxBOTTOM, 10);

        // List
        m_listCtrl = new wxListView(panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL);
        m_listCtrl->AppendColumn("Name", wxLIST_FORMAT_LEFT, 200);
        m_listCtrl->AppendColumn("Path", wxLIST_FORMAT_LEFT, 550);
        
        vbox->Add(m_listCtrl, 1, wxEXPAND | wxALL, 10);

        panel->SetSizer(vbox);

        // Events
        m_searchCtrl->Bind(wxEVT_TEXT, &ModufileFrame::OnSearch, this);
        m_smartMatchCheck->Bind(wxEVT_CHECKBOX, &ModufileFrame::OnSearch, this);
        m_refreshBtn->Bind(wxEVT_BUTTON, &ModufileFrame::OnRefresh, this);
        m_listCtrl->Bind(wxEVT_LIST_ITEM_ACTIVATED, &ModufileFrame::OnItemActivated, this);

        // Initial Refresh
        OnRefresh(wxCommandEvent());
    }

private:
    wxTextCtrl* m_searchCtrl;
    wxCheckBox* m_smartMatchCheck;
    wxButton* m_refreshBtn;
    wxStaticText* m_statusText;
    wxListView* m_listCtrl;
    
    std::vector<FileInfo> m_currentResults;

    void OnSearch(wxCommandEvent& event) {
        std::string query = m_searchCtrl->GetValue().ToStdString();
        bool smart = m_smartMatchCheck->IsChecked();
        
        m_currentResults = FileEngine::Get().Search(query, smart);
        UpdateList();
        
        m_statusText->SetLabel(wxString::Format("Found: %zu files", m_currentResults.size()));
    }

    void OnRefresh(wxCommandEvent& event) {
        m_refreshBtn->Disable();
        m_statusText->SetLabel("Indexing...");
        
        FileEngine::Get().RefreshIndex([this](size_t count) {
            // Call on UI thread
            this->GetEventHandler()->CallAfter([this, count]() {
                m_statusText->SetLabel(wxString::Format("Indexing complete. Total files: %zu", count));
                m_refreshBtn->Enable();
                // Re-trigger search to update view
                wxCommandEvent dummy;
                OnSearch(dummy);
            });
        });
    }

    void OnItemActivated(wxListEvent& event) {
        long index = event.GetIndex();
        if (index >= 0 && index < (long)m_currentResults.size()) {
            FileEngine::Get().OpenFile(m_currentResults[index].path);
        }
    }

    void UpdateList() {
        m_listCtrl->DeleteAllItems();
        for (long i = 0; i < (long)m_currentResults.size(); ++i) {
            long idx = m_listCtrl->InsertItem(i, m_currentResults[i].name);
            m_listCtrl->SetItem(idx, 1, m_currentResults[i].path);
        }
    }
};

class ModufileApp : public wxApp {
public:
    virtual bool OnInit() {
        ModufileFrame* frame = new ModufileFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(ModufileApp);
