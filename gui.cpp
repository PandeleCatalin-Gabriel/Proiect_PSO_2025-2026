#include <wx/wx.h>
#include <wx/grid.h>  // Pentru tabel de vizualizare a datelor

// Clasa aplicației
class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

// Fereastra principală
class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title);

private:
    void OnInsert(wxCommandEvent& event);
    void OnUpdate(wxCommandEvent& event);
    void OnDelete(wxCommandEvent& event);

    wxTextCtrl* m_idCtrl;
    wxTextCtrl* m_nameCtrl;
    wxTextCtrl* m_dateCtrl;
    wxGrid* m_grid;

    wxDECLARE_EVENT_TABLE();
};

wxIMPLEMENT_APP(MyApp);

// Implementarea funcțiilor aplicației
bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame("Server Baze de Date - Interfață");
    frame->Show(true);
    return true;
}

// Inițializarea ferestrei principale
MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title)
{
    // Setăm dimensiunea ferestrei
    SetSize(800, 600);

    // Creăm un meniu
    wxMenu* fileMenu = new wxMenu;
    fileMenu->Append(1001, "&Insert");
    fileMenu->Append(1002, "&Update");
    fileMenu->Append(1003, "&Delete");

    wxMenuBar* menuBar = new wxMenuBar;
    menuBar->Append(fileMenu, "&Operatiuni");

    SetMenuBar(menuBar);

    // Câmpuri pentru input de date
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* panel = new wxPanel(this);
    wxBoxSizer* formSizer = new wxBoxSizer(wxHORIZONTAL);

    formSizer->Add(new wxStaticText(panel, wxID_ANY, "ID:"), 0, wxALL, 5);
    m_idCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(150, 25));
    formSizer->Add(m_idCtrl, 0, wxALL, 5);

    formSizer->Add(new wxStaticText(panel, wxID_ANY, "Name:"), 0, wxALL, 5);
    m_nameCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(150, 25));
    formSizer->Add(m_nameCtrl, 0, wxALL, 5);

    formSizer->Add(new wxStaticText(panel, wxID_ANY, "Date:"), 0, wxALL, 5);
    m_dateCtrl = new wxTextCtrl(panel, wxID_ANY, "", wxDefaultPosition, wxSize(150, 25));
    formSizer->Add(m_dateCtrl, 0, wxALL, 5);

    panel->SetSizerAndFit(formSizer);

    // Tabel pentru vizualizarea datelor
    m_grid = new wxGrid(this, wxID_ANY);
    m_grid->CreateGrid(0, 3);  // Inițial fără rânduri
    m_grid->SetColLabelValue(0, "ID");
    m_grid->SetColLabelValue(1, "Name");
    m_grid->SetColLabelValue(2, "Date");

    // Adăugăm câmpurile și tabela în fereastră
    sizer->Add(panel, 0, wxALL, 10);
    sizer->Add(m_grid, 1, wxALL | wxEXPAND, 10);

    SetSizerAndFit(sizer);
}

// Funcțiile pentru operarea cu date

void MyFrame::OnInsert(wxCommandEvent& event)
{
    wxString id = m_idCtrl->GetValue();
    wxString name = m_nameCtrl->GetValue();
    wxString date = m_dateCtrl->GetValue();

    // Verificăm dacă câmpurile nu sunt goale
    if (id.IsEmpty() || name.IsEmpty() || date.IsEmpty())
    {
        wxMessageBox("Te rugăm să completezi toate câmpurile!", "Eroare", wxOK | wxICON_ERROR);
        return;
    }

    // Căutăm prima linie goală
    int rowToInsert = -1;
    for (int row = 0; row < m_grid->GetNumberRows(); row++) {
        if (m_grid->GetCellValue(row, 0).IsEmpty()) {
            rowToInsert = row;
            break;
        }
    }

    if (rowToInsert == -1) {
        // Dacă nu sunt rânduri goale, adăugăm un nou rând
        rowToInsert = m_grid->GetNumberRows();
        m_grid->AppendRows(1);  // Adăugăm un rând nou
    }

    // Setăm valorile în tabel
    m_grid->SetCellValue(rowToInsert, 0, id);
    m_grid->SetCellValue(rowToInsert, 1, name);
    m_grid->SetCellValue(rowToInsert, 2, date);

    wxMessageBox("Datele au fost inserate!", "Succes", wxOK | wxICON_INFORMATION);
}

void MyFrame::OnUpdate(wxCommandEvent& event)
{
    // Căutăm ID-ul în tabel pentru a actualiza datele
    wxString id = m_idCtrl->GetValue();
    wxString name = m_nameCtrl->GetValue();
    wxString date = m_dateCtrl->GetValue();

    if (id.IsEmpty() || name.IsEmpty() || date.IsEmpty())
    {
        wxMessageBox("Te rugăm să completezi toate câmpurile!", "Eroare", wxOK | wxICON_ERROR);
        return;
    }

    bool updated = false;
    for (int row = 0; row < m_grid->GetNumberRows(); row++)
    {
        if (m_grid->GetCellValue(row, 0) == id)
        {
            m_grid->SetCellValue(row, 1, name);
            m_grid->SetCellValue(row, 2, date);
            updated = true;
            break;
        }
    }

    if (updated)
    {
        wxMessageBox("Datele au fost actualizate!", "Succes", wxOK | wxICON_INFORMATION);
    }
    else
    {
        wxMessageBox("ID-ul nu a fost găsit!", "Eroare", wxOK | wxICON_ERROR);
    }
}

void MyFrame::OnDelete(wxCommandEvent& event)
{
    // Căutăm ID-ul în tabel pentru a șterge datele
    wxString id = m_idCtrl->GetValue();

    if (id.IsEmpty())
    {
        wxMessageBox("Te rugăm să completezi ID-ul pentru ștergere!", "Eroare", wxOK | wxICON_ERROR);
        return;
    }

    bool deleted = false;
    for (int row = 0; row < m_grid->GetNumberRows(); row++)
    {
        if (m_grid->GetCellValue(row, 0) == id)
        {
            m_grid->DeleteRows(row);
            deleted = true;
            break;
        }
    }

    if (deleted)
    {
        wxMessageBox("Datele au fost șterse!", "Succes", wxOK | wxICON_INFORMATION);
    }
    {
        wxMessageBox("ID-ul nu a fost găsit!", "Eroare", wxOK | wxICON_ERROR);
    }
}

// Definirea evenimentelor
wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(1001, MyFrame::OnInsert)
    EVT_MENU(1002, MyFrame::OnUpdate)
    EVT_MENU(1003, MyFrame::OnDelete)
wxEND_EVENT_TABLE()
