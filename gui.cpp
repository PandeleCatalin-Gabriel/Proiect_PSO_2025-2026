#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title);

private:
    wxTextCtrl* m_sqlInput;
    wxGrid* m_grid;
    wxTreeCtrl* m_tree;
    wxSplitterWindow* m_splitter;
    wxPanel* m_rightPanel;

    struct TableData {
        std::vector<std::pair<wxString, wxString>> columns; 
        std::vector<std::vector<wxString>> rows;
    };
    std::map<wxString, std::map<wxString, TableData>> databases;
    wxString currentDB;

    void OnExecute(wxCommandEvent& event);
    void UpdateTree();
    void OnTreeItemActivated(wxTreeEvent& event);
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame("Simulare SSMS - GUI Bază de date");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1100, 700))
{
    m_splitter = new wxSplitterWindow(this, wxID_ANY);
    wxPanel* leftPanel = new wxPanel(m_splitter);
    m_rightPanel = new wxPanel(m_splitter);

    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    m_tree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
    leftSizer->Add(m_tree, 1, wxEXPAND | wxALL, 5);
    leftPanel->SetSizer(leftSizer);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);

    m_sqlInput = new wxTextCtrl(m_rightPanel, wxID_ANY, "",
                                wxDefaultPosition, wxSize(-1, 100),
                                wxTE_MULTILINE | wxTE_PROCESS_ENTER);
    rightSizer->Add(new wxStaticText(m_rightPanel, wxID_ANY, "Linie de comandă SQL:"), 0, wxALL, 5);
    rightSizer->Add(m_sqlInput, 0, wxALL | wxEXPAND, 5);

    wxButton* executeButton = new wxButton(m_rightPanel, wxID_ANY, "Run");
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    buttonSizer->Add(executeButton, 0, wxALL, 5);
    rightSizer->Add(buttonSizer, 0, wxEXPAND);

    m_grid = new wxGrid(m_rightPanel, wxID_ANY);
    m_grid->CreateGrid(0, 0);
    rightSizer->Add(m_grid, 1, wxALL | wxEXPAND, 10);
    m_grid->Hide();

    m_rightPanel->SetSizer(rightSizer);

    m_splitter->SplitVertically(leftPanel, m_rightPanel, 250);

    executeButton->Bind(wxEVT_BUTTON, &MyFrame::OnExecute, this);
    m_tree->Bind(wxEVT_TREE_ITEM_ACTIVATED, &MyFrame::OnTreeItemActivated, this);

    UpdateTree();
}

void MyFrame::OnExecute(wxCommandEvent& event)
{
    wxString command;
    if (m_sqlInput->GetStringSelection().IsEmpty()) {
        long pos = m_sqlInput->GetInsertionPoint();
        int line = m_sqlInput->PositionToXY(pos, nullptr, nullptr);
        command = m_sqlInput->GetLineText(line).Trim().Trim(false).Upper();
    } else {
        command = m_sqlInput->GetStringSelection().Trim().Trim(false).Upper();
    }

    if (command.IsEmpty()) {
        wxMessageBox("Nicio comandă selectată sau linia este goală!", "Eroare");
        return;
    }

    wxLogMessage("Comandă executată: " + command);

    wxString tmp;
    if (command.StartsWith("CREATE DATABASE", &tmp))
    {
        wxString dbName = tmp.Trim();
        if (dbName.IsEmpty()) {
            wxMessageBox("Numele bazei de date lipsește!", "Eroare");
            return;
        }
        if (databases.find(dbName) != databases.end()) {
            wxMessageBox("Baza de date '" + dbName + "' există deja!", "Eroare");
            return;
        }
        databases[dbName] = {};
        currentDB = dbName;
        wxMessageBox("Bază de date creată: " + dbName, "Succes");
    }
    else if (command.StartsWith("USE", &tmp))
    {
        wxString dbName = tmp.Trim();
        if (dbName.IsEmpty()) {
            wxMessageBox("Numele bazei de date lipsește după USE!", "Eroare");
            return;
        }
        if (databases.find(dbName) == databases.end()) {
            wxMessageBox("Baza de date '" + dbName + "' nu există!", "Eroare");
            return;
        }
        currentDB = dbName;
        wxMessageBox("Bază de date selectată: " + currentDB, "Succes");
    }
    else if (command.StartsWith("CREATE TABLE", &tmp))
    {
        if (currentDB.IsEmpty()) {
            wxMessageBox("Selectați o bază de date cu USE!", "Eroare");
            return;
        }

        int start = command.Find('(');
        int end = command.Find(')');
        if (start == wxNOT_FOUND || end == wxNOT_FOUND || start >= end) {
            wxMessageBox("Sintaxă invalidă pentru CREATE TABLE! Verificați parantezele.", "Eroare");
            return;
        }

        wxString tableName = command.SubString(12, start - 1).Trim();
        if (tableName.IsEmpty()) {
            wxMessageBox("Numele tabelului lipsește!", "Eroare");
            return;
        }
        if (databases[currentDB].find(tableName) != databases[currentDB].end()) {
            wxMessageBox("Tabelul '" + tableName + "' există deja în baza de date '" + currentDB + "'!", "Eroare");
            return;
        }

        wxString columnsPart = command.SubString(start + 1, end - 1).Trim();
        wxArrayString colDefs;
        wxString currentCol;
        bool insideQuotes = false;

        for (size_t i = 0; i < columnsPart.Length(); ++i) {
            wxChar c = columnsPart[i];
            if (c == '"') {
                insideQuotes = !insideQuotes;
                continue;
            }
            if (c == ',' && !insideQuotes) {
                if (!currentCol.IsEmpty()) {
                    colDefs.Add(currentCol.Trim().Trim(false));
                    currentCol.Clear();
                }
                continue;
            }
            currentCol += c;
        }
        if (!currentCol.IsEmpty()) {
            colDefs.Add(currentCol.Trim().Trim(false));
        }

        TableData table;
        for (auto& col : colDefs) {
            col = col.Trim().Trim(false);
            if (col.IsEmpty()) continue;
            wxArrayString parts = wxSplit(col, ' ');
            if (parts.IsEmpty()) continue;
            wxString colName = parts[0].Trim();
            wxString colType = parts.size() > 1 ? parts[1].Trim() : "TEXT";
            if (colName.IsEmpty()) continue;
            table.columns.push_back({colName, colType});
            wxLogMessage("Coloană adăugată: " + colName + " (" + colType + ")");
        }

        if (table.columns.empty()) {
            wxMessageBox("Tabelul trebuie să aibă cel puțin o coloană!", "Eroare");
            return;
        }

        databases[currentDB][tableName] = table;
        wxMessageBox("Tabel creat: " + tableName + " în baza de date " + currentDB, "Succes");
    }
    else if (command.StartsWith("INSERT INTO", &tmp))
    {
        if (currentDB.IsEmpty()) {
            wxMessageBox("Selectați o bază de date cu USE!", "Eroare");
            return;
        }

        int valuesStart = command.Find('(');
        if (valuesStart == wxNOT_FOUND) {
            wxMessageBox("Sintaxă invalidă pentru INSERT INTO! Lipsesc parantezele cu valorile.", "Eroare");
            return;
        }
        wxString tableName = command.SubString(11, valuesStart - 1).Trim();
        if (tableName.IsEmpty()) {
            wxMessageBox("Numele tabelului lipsește în INSERT INTO!", "Eroare");
            return;
        }
        if (databases[currentDB].find(tableName) == databases[currentDB].end()) {
            wxMessageBox("Tabelul '" + tableName + "' nu există în baza de date '" + currentDB + "'!", "Eroare");
            return;
        }

        int end = command.Find(')');
        if (end == wxNOT_FOUND || end < valuesStart) {
            wxMessageBox("Sintaxă invalidă pentru INSERT INTO! Verificați parantezele.", "Eroare");
            return;
        }

        wxString valuesPart = command.SubString(valuesStart + 1, end - 1).Trim();
        wxArrayString values;
        wxString currentValue;
        bool insideQuotes = false;

        for (size_t i = 0; i < valuesPart.Length(); ++i) {
            wxChar c = valuesPart[i];
            if (c == '"' || c == '\'') {
                insideQuotes = !insideQuotes;
                currentValue += c;
                continue;
            }
            if (c == ',' && !insideQuotes) {
                if (!currentValue.IsEmpty()) {
                    values.Add(currentValue.Trim().Trim(false));
                    currentValue.Clear();
                }
                continue;
            }
            currentValue += c;
        }
        if (!currentValue.IsEmpty()) {
            values.Add(currentValue.Trim().Trim(false));
        }

        TableData& tbl = databases[currentDB][tableName];

        std::vector<wxString> row;
        for (auto& v : values) {
            wxString cleaned = v.Trim().Trim(false);
            if (cleaned.StartsWith("\"") && cleaned.EndsWith("\"")) {
                cleaned = cleaned.SubString(1, cleaned.Length() - 2);
            } else if (cleaned.StartsWith("'") && cleaned.EndsWith("'")) {
                cleaned = cleaned.SubString(1, cleaned.Length() - 2);
            }
            row.push_back(cleaned);
        }

        if (row.size() != tbl.columns.size()) {
            wxMessageBox("Numărul de valori (" + wxString::Format("%ld", row.size()) +
                         ") nu corespunde cu numărul de coloane (" +
                         wxString::Format("%ld", tbl.columns.size()) + ")!", "Eroare");
            return;
        }

        tbl.rows.push_back(row);
        wxMessageBox("Rând inserat în " + tableName, "Succes");

        wxString rowStr;
        for (size_t i = 0; i < row.size(); ++i) {
            if (i > 0) rowStr += ",";
            rowStr += row[i];
        }
        wxLogMessage("Rând inserat: " + rowStr);
    }
    else
    {
        wxMessageBox("Comandă SQL necunoscută: " + command, "Eroare");
    }

    UpdateTree();
}

void MyFrame::UpdateTree()
{
    m_tree->DeleteAllItems();
    wxTreeItemId root = m_tree->AddRoot("Databases");

    for (const auto& db : databases)
    {
        wxTreeItemId dbNode = m_tree->AppendItem(root, db.first);
        wxTreeItemId tablesNode = m_tree->AppendItem(dbNode, "Tables");
        for (const auto& tbl : db.second)
        {
            m_tree->AppendItem(tablesNode, tbl.first);
        }
    }

    m_tree->ExpandAll();
}

void MyFrame::OnTreeItemActivated(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    if (!item.IsOk()) {
        wxMessageBox("Nod invalid selectat!", "Eroare");
        return;
    }

    wxString selected = m_tree->GetItemText(item);
    wxTreeItemId parent = m_tree->GetItemParent(item);
    if (!parent.IsOk()) return;

    wxString parentText = m_tree->GetItemText(parent);
    wxTreeItemId grandParent = m_tree->GetItemParent(parent);
    wxString dbName;

    if (parentText == "Tables" && grandParent.IsOk()) {
        dbName = m_tree->GetItemText(grandParent);
    } else {
        return; 
    }

    if (databases.find(dbName) == databases.end()) {
        wxMessageBox("Baza de date '" + dbName + "' nu există!", "Eroare");
        return;
    }
    if (databases[dbName].find(selected) == databases[dbName].end()) {
        wxMessageBox("Tabelul '" + selected + "' nu există în baza de date '" + dbName + "'!", "Eroare");
        return;
    }

    TableData& table = databases[dbName][selected];

    m_grid->Hide();
    if (m_grid->GetNumberCols() > 0) m_grid->DeleteCols(0, m_grid->GetNumberCols());
    if (m_grid->GetNumberRows() > 0) m_grid->DeleteRows(0, m_grid->GetNumberRows());

    m_grid->AppendCols(table.columns.size());
    for (size_t i = 0; i < table.columns.size(); i++) {
        m_grid->SetColLabelValue(i, table.columns[i].first);
        wxLogMessage("Setez eticheta coloanei %zu: %s", i, table.columns[i].first.c_str());
    }

    for (size_t r = 0; r < table.rows.size(); r++) {
        const auto& row = table.rows[r];
        m_grid->AppendRows(1);
        for (size_t i = 0; i < row.size() && i < table.columns.size(); i++) {
            m_grid->SetCellValue(r, i, row[i]);
            wxLogMessage("Setez celula (%zu, %zu): %s", r, i, row[i].c_str());
        }
    }

    m_grid->AutoSizeColumns();
    m_grid->Show(true);
    m_rightPanel->Layout();
    m_grid->ForceRefresh();
}