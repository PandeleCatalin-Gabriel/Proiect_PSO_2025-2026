#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

class CreateTableDialog : public wxDialog
{
public:
    CreateTableDialog(wxWindow* parent);
    
    wxString GetTableName() const { return m_tableNameInput->GetValue(); }
    wxString GetCreateCommand() const;
    
private:
    wxTextCtrl* m_tableNameInput;
    wxListCtrl* m_columnList;
    wxTextCtrl* m_colNameInput;
    wxChoice* m_colTypeChoice;
    wxSpinCtrl* m_colLengthSpin;
    wxCheckBox* m_primaryKeyCheck;
    wxCheckBox* m_notNullCheck;
    
    void OnAddColumn(wxCommandEvent& event);
    void OnRemoveColumn(wxCommandEvent& event);
    void OnTypeChanged(wxCommandEvent& event);
    void UpdateLengthEnabled();
};

CreateTableDialog::CreateTableDialog(wxWindow* parent)
    : wxDialog(parent, wxID_ANY, "Create New Table", wxDefaultPosition, wxSize(650, 500))
{
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer* nameBox = new wxStaticBoxSizer(wxHORIZONTAL, this, "Table Name");
    m_tableNameInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200, -1));
    nameBox->Add(new wxStaticText(this, wxID_ANY, "Name:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    nameBox->Add(m_tableNameInput, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(nameBox, 0, wxEXPAND | wxALL, 10);
    
    wxStaticBoxSizer* addColBox = new wxStaticBoxSizer(wxVERTICAL, this, "Add Column");
    
    wxBoxSizer* colRow1 = new wxBoxSizer(wxHORIZONTAL);
    colRow1->Add(new wxStaticText(this, wxID_ANY, "Column Name:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_colNameInput = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(120, -1));
    colRow1->Add(m_colNameInput, 0, wxALL, 5);
    
    colRow1->Add(new wxStaticText(this, wxID_ANY, "Type:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    wxArrayString types;
    types.Add("INT");
    types.Add("VARCHAR");
    types.Add("DATE");
    m_colTypeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(100, -1), types);
    m_colTypeChoice->SetSelection(0);
    colRow1->Add(m_colTypeChoice, 0, wxALL, 5);
    
    colRow1->Add(new wxStaticText(this, wxID_ANY, "Length:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_colLengthSpin = new wxSpinCtrl(this, wxID_ANY, "50", wxDefaultPosition, wxSize(70, -1), wxSP_ARROW_KEYS, 1, 1000, 50);
    m_colLengthSpin->Enable(false);
    colRow1->Add(m_colLengthSpin, 0, wxALL, 5);
    
    addColBox->Add(colRow1, 0, wxEXPAND);
    
    wxBoxSizer* colRow2 = new wxBoxSizer(wxHORIZONTAL);
    m_primaryKeyCheck = new wxCheckBox(this, wxID_ANY, "PRIMARY KEY");
    m_notNullCheck = new wxCheckBox(this, wxID_ANY, "NOT NULL");
    colRow2->Add(m_primaryKeyCheck, 0, wxALL, 5);
    colRow2->Add(m_notNullCheck, 0, wxALL, 5);
    
    wxButton* addColBtn = new wxButton(this, wxID_ANY, "Add Column");
    colRow2->AddStretchSpacer();
    colRow2->Add(addColBtn, 0, wxALL, 5);
    
    addColBox->Add(colRow2, 0, wxEXPAND);
    mainSizer->Add(addColBox, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);
    
    wxStaticBoxSizer* colListBox = new wxStaticBoxSizer(wxVERTICAL, this, "Columns");
    
    m_columnList = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 150),
                                   wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
    
    wxListItem col0;
    col0.SetId(0);
    col0.SetText("Name");
    col0.SetWidth(120);
    m_columnList->InsertColumn(0, col0);
    
    wxListItem col1;
    col1.SetId(1);
    col1.SetText("Type");
    col1.SetWidth(100);
    m_columnList->InsertColumn(1, col1);
    
    wxListItem col2;
    col2.SetId(2);
    col2.SetText("Constraints");
    col2.SetWidth(130);
    m_columnList->InsertColumn(2, col2);
    
    colListBox->Add(m_columnList, 1, wxEXPAND | wxALL, 5);
    
    wxButton* removeColBtn = new wxButton(this, wxID_ANY, "Remove Selected");
    colListBox->Add(removeColBtn, 0, wxALL | wxALIGN_RIGHT, 5);
    
    mainSizer->Add(colListBox, 1, wxEXPAND | wxALL, 10);
    
    wxBoxSizer* btnSizer = new wxBoxSizer(wxHORIZONTAL);
    btnSizer->AddStretchSpacer();
    wxButton* okBtn = new wxButton(this, wxID_OK, "Create Table");
    wxButton* cancelBtn = new wxButton(this, wxID_CANCEL, "Cancel");
    btnSizer->Add(okBtn, 0, wxALL, 5);
    btnSizer->Add(cancelBtn, 0, wxALL, 5);
    mainSizer->Add(btnSizer, 0, wxEXPAND | wxALL, 10);
    
    SetSizer(mainSizer);
    
    addColBtn->Bind(wxEVT_BUTTON, &CreateTableDialog::OnAddColumn, this);
    removeColBtn->Bind(wxEVT_BUTTON, &CreateTableDialog::OnRemoveColumn, this);
    m_colTypeChoice->Bind(wxEVT_CHOICE, &CreateTableDialog::OnTypeChanged, this);
}

void CreateTableDialog::OnTypeChanged(wxCommandEvent& event)
{
    UpdateLengthEnabled();
}

void CreateTableDialog::UpdateLengthEnabled()
{
    wxString type = m_colTypeChoice->GetStringSelection();
    m_colLengthSpin->Enable(type == "VARCHAR");
}

void CreateTableDialog::OnAddColumn(wxCommandEvent& event)
{
    wxString colName = m_colNameInput->GetValue().Trim();
    if (colName.IsEmpty()) {
        wxMessageBox("Please enter a column name", "Error", wxOK | wxICON_ERROR, this);
        return;
    }
    
    wxString colType = m_colTypeChoice->GetStringSelection();
    if (colType == "VARCHAR") {
        colType += wxString::Format("(%d)", m_colLengthSpin->GetValue());
    }
    
    wxString constraints;
    if (m_primaryKeyCheck->GetValue()) {
        constraints += "PRIMARY KEY ";
    }
    if (m_notNullCheck->GetValue()) {
        constraints += "NOT NULL";
    }
    constraints.Trim();
    
    int itemCount = m_columnList->GetItemCount();
    
    wxListItem item;
    item.SetId(itemCount);
    item.SetColumn(0);
    item.SetText(colName);
    item.SetMask(wxLIST_MASK_TEXT);
    
    long index = m_columnList->InsertItem(item);
    
    if (index >= 0) {
        m_columnList->SetItem(index, 1, colType);
        m_columnList->SetItem(index, 2, constraints);
        
        m_columnList->EnsureVisible(index);
        
        m_columnList->Refresh();
        m_columnList->Update();
        
        m_colNameInput->Clear();
        m_primaryKeyCheck->SetValue(false);
        m_notNullCheck->SetValue(false);
        m_colTypeChoice->SetSelection(0);
        UpdateLengthEnabled();
        
        m_colNameInput->SetFocus();
    } else {
        wxMessageBox("Failed to add column to list (index=" + wxString::Format("%ld", index) + ")", 
                    "Error", wxOK | wxICON_ERROR, this);
    }
}

void CreateTableDialog::OnRemoveColumn(wxCommandEvent& event)
{
    long selected = m_columnList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
    if (selected != -1) {
        m_columnList->DeleteItem(selected);
    }
}

wxString CreateTableDialog::GetCreateCommand() const
{
    wxString tableName = m_tableNameInput->GetValue().Trim();
    if (tableName.IsEmpty()) {
        return "";
    }
    
    int colCount = m_columnList->GetItemCount();
    if (colCount == 0) {
        return "";
    }
    
    wxString cmd = "CREATE TABLE " + tableName + " (";
    
    for (int i = 0; i < colCount; i++) {
        if (i > 0) cmd += ", ";
        
        wxListItem item;
        item.SetId(i);
        item.SetMask(wxLIST_MASK_TEXT);
        
        item.SetColumn(0);
        m_columnList->GetItem(item);
        wxString colName = item.GetText();
        
        item.SetColumn(1);
        m_columnList->GetItem(item);
        wxString colType = item.GetText();
        
        item.SetColumn(2);
        m_columnList->GetItem(item);
        wxString constraints = item.GetText();
        
        cmd += colName + " " + colType;
        if (!constraints.IsEmpty()) {
            cmd += " " + constraints;
        }
    }
    
    cmd += ")";
    return cmd;
}

class MyApp : public wxApp
{
public:
    virtual bool OnInit();
};

class MyFrame : public wxFrame
{
public:
    MyFrame(const wxString& title);
    ~MyFrame();

private:
    wxTextCtrl* m_sqlInput;
    wxTextCtrl* m_logOutput;
    wxGrid* m_grid;
    wxTreeCtrl* m_tree;
    wxSplitterWindow* m_splitter;
    wxPanel* m_rightPanel;
    wxButton* m_connectButton;
    wxButton* m_executeButton;
    wxButton* m_refreshButton;
    wxButton* m_createTableButton;
    wxStaticText* m_statusLabel;
    wxTextCtrl* m_serverIPInput;
    wxTextCtrl* m_serverPortInput;
    
    int clientSocket;
    bool connected;
    wxString serverIP;
    int serverPort;

    void OnExecute(wxCommandEvent& event);
    void OnConnect(wxCommandEvent& event);
    void OnTestConnection(wxCommandEvent& event);
    void OnRefreshStructure(wxCommandEvent& event);
    void OnCreateTable(wxCommandEvent& event);
    void OnTreeItemActivated(wxTreeEvent& event);
    void OnTreeRightClick(wxTreeEvent& event);
    void UpdateTree();
    void LoadDatabaseStructure();
    
    bool ConnectToServer();
    void DisconnectFromServer();
    wxString SendCommand(const wxString& command);
    void ParseSelectResult(const wxString& result);
    void LogMessage(const wxString& message);
    
    bool EnsureConnected();
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit()
{
    MyFrame* frame = new MyFrame("Database Client - GUI");
    frame->Show(true);
    return true;
}

MyFrame::MyFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(1200, 800)),
      clientSocket(-1), connected(false), serverIP("127.0.0.1"), serverPort(5432)
{
    wxPanel* mainPanel = new wxPanel(this);
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBoxSizer* connectionBox = new wxStaticBoxSizer(wxHORIZONTAL, mainPanel, "Connection");
    
    connectionBox->Add(new wxStaticText(mainPanel, wxID_ANY, "Server IP:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_serverIPInput = new wxTextCtrl(mainPanel, wxID_ANY, serverIP, wxDefaultPosition, wxSize(120, -1));
    connectionBox->Add(m_serverIPInput, 0, wxALL, 5);
    
    connectionBox->Add(new wxStaticText(mainPanel, wxID_ANY, "Port:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    m_serverPortInput = new wxTextCtrl(mainPanel, wxID_ANY, wxString::Format("%d", serverPort), wxDefaultPosition, wxSize(80, -1));
    connectionBox->Add(m_serverPortInput, 0, wxALL, 5);
    
    m_connectButton = new wxButton(mainPanel, wxID_ANY, "Connect");
    connectionBox->Add(m_connectButton, 0, wxALL, 5);
    
    wxButton* testButton = new wxButton(mainPanel, wxID_ANY, "Test (PING)");
    connectionBox->Add(testButton, 0, wxALL, 5);
    
    m_refreshButton = new wxButton(mainPanel, wxID_ANY, "Refresh");
    m_refreshButton->Enable(false);
    connectionBox->Add(m_refreshButton, 0, wxALL, 5);
    
    m_createTableButton = new wxButton(mainPanel, wxID_ANY, "Create Table");
    m_createTableButton->Enable(false);
    connectionBox->Add(m_createTableButton, 0, wxALL, 5);
    
    connectionBox->AddStretchSpacer();
    
    m_statusLabel = new wxStaticText(mainPanel, wxID_ANY, "Status: Disconnected");
    m_statusLabel->SetForegroundColour(*wxRED);
    connectionBox->Add(m_statusLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    
    mainSizer->Add(connectionBox, 0, wxEXPAND | wxALL, 5);
    
    m_splitter = new wxSplitterWindow(mainPanel, wxID_ANY);
    
    wxPanel* leftPanel = new wxPanel(m_splitter);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "Database Structure:"), 0, wxALL, 5);
    m_tree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
    leftSizer->Add(m_tree, 1, wxEXPAND | wxALL, 5);
    leftPanel->SetSizer(leftSizer);
    
    m_rightPanel = new wxPanel(m_splitter);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    
    rightSizer->Add(new wxStaticText(m_rightPanel, wxID_ANY, "SQL Command (select text to execute only selection):"), 0, wxALL, 5);
    m_sqlInput = new wxTextCtrl(m_rightPanel, wxID_ANY, "",
                                wxDefaultPosition, wxSize(-1, 120),
                                wxTE_MULTILINE | wxTE_PROCESS_ENTER | wxTE_RICH2 | wxHSCROLL);
    m_sqlInput->SetFont(wxFont(10, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    rightSizer->Add(m_sqlInput, 0, wxALL | wxEXPAND, 5);
    
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonSizer->AddStretchSpacer();
    m_executeButton = new wxButton(m_rightPanel, wxID_ANY, "Execute SQL");
    m_executeButton->Enable(false);
    buttonSizer->Add(m_executeButton, 0, wxALL, 5);
    rightSizer->Add(buttonSizer, 0, wxEXPAND);
    
    m_grid = new wxGrid(m_rightPanel, wxID_ANY);
    m_grid->CreateGrid(0, 0);
    m_grid->EnableEditing(false);
    rightSizer->Add(m_grid, 1, wxALL | wxEXPAND, 5);
    
    rightSizer->Add(new wxStaticText(m_rightPanel, wxID_ANY, "Log:"), 0, wxALL, 5);
    m_logOutput = new wxTextCtrl(m_rightPanel, wxID_ANY, "",
                                 wxDefaultPosition, wxSize(-1, 100),
                                 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH2);
    m_logOutput->SetFont(wxFont(9, wxFONTFAMILY_TELETYPE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));
    rightSizer->Add(m_logOutput, 0, wxALL | wxEXPAND, 5);
    
    m_rightPanel->SetSizer(rightSizer);
    
    m_splitter->SplitVertically(leftPanel, m_rightPanel, 250);
    m_splitter->SetMinimumPaneSize(200);
    
    mainSizer->Add(m_splitter, 1, wxEXPAND | wxALL, 5);
    mainPanel->SetSizer(mainSizer);
    
    m_executeButton->Bind(wxEVT_BUTTON, &MyFrame::OnExecute, this);
    m_connectButton->Bind(wxEVT_BUTTON, &MyFrame::OnConnect, this);
    testButton->Bind(wxEVT_BUTTON, &MyFrame::OnTestConnection, this);
    m_refreshButton->Bind(wxEVT_BUTTON, &MyFrame::OnRefreshStructure, this);
    m_createTableButton->Bind(wxEVT_BUTTON, &MyFrame::OnCreateTable, this);
    m_tree->Bind(wxEVT_TREE_ITEM_ACTIVATED, &MyFrame::OnTreeItemActivated, this);
    m_tree->Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &MyFrame::OnTreeRightClick, this);
    
    UpdateTree();
    LogMessage("Application started. Please connect to server.");
}

MyFrame::~MyFrame()
{
    DisconnectFromServer();
}

void MyFrame::LogMessage(const wxString& message)
{
    wxString timestamp = wxDateTime::Now().Format("[%H:%M:%S] ");
    m_logOutput->AppendText(timestamp + message + "\n");
}

bool MyFrame::EnsureConnected()
{
    if (!connected || clientSocket == -1) {
        return false;
    }
    
    char test;
    int result = recv(clientSocket, &test, 1, MSG_PEEK | MSG_DONTWAIT);
    
    if (result == 0) {
        LogMessage("Connection lost, attempting to reconnect...");
        DisconnectFromServer();
        
        if (ConnectToServer()) {
            LogMessage("Reconnected successfully!");
            return true;
        } else {
            LogMessage("Reconnection failed");
            m_connectButton->SetLabel("Connect");
            m_statusLabel->SetLabel("Status: Disconnected");
            m_statusLabel->SetForegroundColour(*wxRED);
            m_executeButton->Enable(false);
            m_refreshButton->Enable(false);
            m_createTableButton->Enable(false);
            m_serverIPInput->Enable(true);
            m_serverPortInput->Enable(true);
            return false;
        }
    }
    
    return true;
}

void MyFrame::OnCreateTable(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    CreateTableDialog dlg(this);
    if (dlg.ShowModal() == wxID_OK) {
        wxString cmd = dlg.GetCreateCommand();
        
        if (cmd.IsEmpty()) {
            wxMessageBox("Invalid table definition. Please provide a table name and at least one column.", 
                        "Error", wxOK | wxICON_ERROR);
            return;
        }
        
        LogMessage("Creating table: " + cmd);
        
        wxString response = SendCommand(cmd);
        
        if (response.StartsWith("ERROR: Connection lost")) {
            wxMessageBox("Connection lost. Please reconnect.", "Connection Error", wxOK | wxICON_ERROR);
            return;
        }
        
        wxMessageBox(response, "Create Table Result", wxOK | wxICON_INFORMATION);
        
        if (response.StartsWith("SUCCESS")) {
            LoadDatabaseStructure();
        }
    }
}

void MyFrame::OnTreeRightClick(wxTreeEvent& event)
{
    if (!connected) return;
    
    wxTreeItemId item = event.GetItem();
    wxString itemText = m_tree->GetItemText(item);
    
    wxTreeItemId parent = m_tree->GetItemParent(item);
    if (!parent.IsOk()) return;
    
    wxString parentText = m_tree->GetItemText(parent);
    
    if (parentText.StartsWith("Tables")) {
        wxMenu menu;
        menu.Append(1, "SELECT * FROM " + itemText);
        menu.Append(2, "DESCRIBE " + itemText);
        menu.AppendSeparator();
        menu.Append(3, "DROP TABLE " + itemText);
        
        int choice = GetPopupMenuSelectionFromUser(menu);
        
        if (choice == 1) {
            LogMessage("Quick SELECT: " + itemText);
            wxString response = SendCommand("SELECT * FROM " + itemText);
            
            if (!response.StartsWith("ERROR")) {
                ParseSelectResult(response);
            } else {
                wxMessageBox(response, "Query Error", wxOK | wxICON_ERROR);
            }
        } else if (choice == 2) {
            wxString response = SendCommand("DESCRIBE " + itemText);
            wxMessageBox(response, "Table Structure: " + itemText, wxOK | wxICON_INFORMATION);
        } else if (choice == 3) {
            int confirm = wxMessageBox("Are you sure you want to drop table '" + itemText + "'?\nThis cannot be undone!",
                                       "Confirm Drop", wxYES_NO | wxICON_WARNING);
            if (confirm == wxYES) {
                wxString response = SendCommand("DROP TABLE " + itemText);
                wxMessageBox(response, "Drop Table Result", wxOK | wxICON_INFORMATION);
                if (response.StartsWith("SUCCESS")) {
                    LoadDatabaseStructure();
                }
            }
        }
    }
}

void MyFrame::OnConnect(wxCommandEvent& event)
{
    if (!connected) {
        serverIP = m_serverIPInput->GetValue().Trim();
        long port;
        if (!m_serverPortInput->GetValue().ToLong(&port) || port < 1 || port > 65535) {
            wxMessageBox("Invalid port number!", "Error", wxOK | wxICON_ERROR);
            return;
        }
        serverPort = port;
        
        LogMessage("Attempting to connect to " + serverIP + ":" + wxString::Format("%d", serverPort));
        
        if (ConnectToServer()) {
            m_connectButton->SetLabel("Disconnect");
            m_statusLabel->SetLabel("Status: Connected");
            m_statusLabel->SetForegroundColour(wxColour(0, 128, 0));
            m_executeButton->Enable(true);
            m_refreshButton->Enable(true);
            m_createTableButton->Enable(true);
            m_serverIPInput->Enable(false);
            m_serverPortInput->Enable(false);
            LogMessage("Connected successfully!");
            
            LogMessage("Loading database structure...");
            LoadDatabaseStructure();
            
        } else {
            LogMessage("Failed to connect!");
            wxMessageBox("Failed to connect to server!\nCheck if server is running on " + 
                        serverIP + ":" + wxString::Format("%d", serverPort), "Connection Error", wxOK | wxICON_ERROR);
        }
    } else {
        DisconnectFromServer();
        m_connectButton->SetLabel("Connect");
        m_statusLabel->SetLabel("Status: Disconnected");
        m_statusLabel->SetForegroundColour(*wxRED);
        m_executeButton->Enable(false);
        m_refreshButton->Enable(false);
        m_createTableButton->Enable(false);
        m_serverIPInput->Enable(true);
        m_serverPortInput->Enable(true);
        UpdateTree();
        LogMessage("Disconnected from server");
    }
}

bool MyFrame::ConnectToServer()
{
    try {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            LogMessage("ERROR: Cannot create socket - " + wxString(strerror(errno)));
            return false;
        }
        
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        
        int keepalive = 1;
        setsockopt(clientSocket, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive));

        sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(serverPort);
        
        if (serverIP.IsEmpty()) {
            LogMessage("ERROR: Server IP is empty");
            close(clientSocket);
            clientSocket = -1;
            return false;
        }
        
        int ptonResult = inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
        if (ptonResult == 0) {
            LogMessage("ERROR: Invalid IP address format: " + serverIP);
            close(clientSocket);
            clientSocket = -1;
            return false;
        } else if (ptonResult < 0) {
            LogMessage("ERROR: inet_pton failed - " + wxString(strerror(errno)));
            close(clientSocket);
            clientSocket = -1;
            return false;
        }

        LogMessage("Connecting to server...");
        
        int connectResult = connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
        if (connectResult < 0) {
            LogMessage("ERROR: Connection failed - " + wxString(strerror(errno)));
            close(clientSocket);
            clientSocket = -1;
            return false;
        }

        timeout.tv_sec = 30;
        timeout.tv_usec = 0;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(clientSocket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            LogMessage("Server: " + wxString(buffer).Trim());
        } else if (bytesRead == 0) {
            LogMessage("ERROR: Server closed connection immediately");
            close(clientSocket);
            clientSocket = -1;
            return false;
        } else {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                LogMessage("WARNING: No welcome message received");
            }
        }

        connected = true;
        return true;
        
    } catch (const std::exception& e) {
        LogMessage("EXCEPTION during connect: " + wxString(e.what()));
        if (clientSocket != -1) {
            close(clientSocket);
            clientSocket = -1;
        }
        return false;
    } catch (...) {
        LogMessage("UNKNOWN EXCEPTION during connect");
        if (clientSocket != -1) {
            close(clientSocket);
            clientSocket = -1;
        }
        return false;
    }
}

void MyFrame::DisconnectFromServer()
{
    if (clientSocket != -1) {
        LogMessage("Sending QUIT command...");
        wxString quitCmd = "QUIT\n";
        send(clientSocket, quitCmd.c_str(), quitCmd.length(), MSG_NOSIGNAL);
        
        usleep(100000); 
        
        shutdown(clientSocket, SHUT_RDWR);
        close(clientSocket);
        clientSocket = -1;
    }
    connected = false;
}

wxString MyFrame::SendCommand(const wxString& command)
{
    if (!connected || clientSocket == -1) {
        return "ERROR: Not connected to server";
    }

    wxString cmdToSend = wxString(command).Trim();
    if (!cmdToSend.EndsWith("\n")) {
        cmdToSend += "\n";
    }
    
    LogMessage("-> " + wxString(command).Trim());

    ssize_t sent = send(clientSocket, cmdToSend.c_str(), cmdToSend.length(), MSG_NOSIGNAL);
    if (sent < 0) {
        LogMessage("ERROR: Failed to send command - " + wxString(strerror(errno)));
        return "ERROR: Failed to send command";
    }

    std::string fullResponse;
    char buffer[8192];
    
    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            if (bytesRead == 0) {
                LogMessage("ERROR: Connection closed by server");
                DisconnectFromServer();
                m_connectButton->SetLabel("Connect");
                m_statusLabel->SetLabel("Status: Disconnected");
                m_statusLabel->SetForegroundColour(*wxRED);
                m_executeButton->Enable(false);
                m_refreshButton->Enable(false);
                m_createTableButton->Enable(false);
                m_serverIPInput->Enable(true);
                m_serverPortInput->Enable(true);
                return "ERROR: Connection lost";
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (!fullResponse.empty()) {
                    break;
                }
                usleep(50000);
                continue;
            } else {
                LogMessage("ERROR: Failed to receive response - " + wxString(strerror(errno)));
                if (fullResponse.empty()) {
                    return "ERROR: Failed to receive response";
                }
                break;
            }
        }

        buffer[bytesRead] = '\0';
        fullResponse += buffer;
        
        if (fullResponse.length() > 0 && fullResponse.back() == '\n') {
            break;
        }
        
        if (bytesRead < (int)(sizeof(buffer) - 1)) {
            break;
        }
    }
    
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    wxString response(fullResponse);
    response.Trim(true).Trim(false);
    
    wxString logMsg = response.Length() > 200 ? 
                      response.Left(200) + "... (" + wxString::Format("%zu", response.Length()) + " chars total)" : 
                      response;
    LogMessage("<- " + logMsg);
    
    return response;
}

void MyFrame::OnTestConnection(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    wxString response = SendCommand("PING");
    
    if (response == "PONG") {
        wxMessageBox("Connection test successful!\nServer responded with PONG", "Test OK", wxOK | wxICON_INFORMATION);
    } else {
        wxMessageBox("Connection test failed!\nExpected PONG, got: " + response, "Test Failed", wxOK | wxICON_WARNING);
    }
}

void MyFrame::OnRefreshStructure(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    LogMessage("Refreshing database structure...");
    LoadDatabaseStructure();
}

void MyFrame::LoadDatabaseStructure()
{
    if (!connected) return;
    
    wxString response = SendCommand("LIST TABLES");
    
    if (response.StartsWith("ERROR")) {
        LogMessage("Failed to load tables: " + response);
        return;
    }
    
    wxArrayString lines = wxSplit(response, '\n');
    std::vector<wxString> tables;
    
    for (size_t i = 1; i < lines.GetCount(); i++) {
        wxString tableName = lines[i].Trim();
        if (!tableName.IsEmpty()) {
            tables.push_back(tableName);
        }
    }
    
    LogMessage(wxString::Format("Found %zu table(s)", tables.size()));

    m_tree->DeleteAllItems();
    wxTreeItemId root = m_tree->AddRoot("Database: MainDB");
    
    wxTreeItemId tablesNode = m_tree->AppendItem(root, wxString::Format("Tables (%zu)", tables.size()));
    
    for (const auto& tableName : tables) {
        wxTreeItemId tableNode = m_tree->AppendItem(tablesNode, tableName);
        
        wxString descResponse = SendCommand("DESCRIBE " + tableName);
        
        if (!descResponse.StartsWith("ERROR")) {
            wxArrayString descLines = wxSplit(descResponse, '\n');
            bool inColumns = false;
            
            for (size_t j = 0; j < descLines.GetCount(); j++) {
                wxString line = descLines[j];
                line.Trim();
                
                if (line == "Columns:") {
                    inColumns = true;
                    continue;
                }
                if (inColumns && line.StartsWith("  ")) {
                    wxString colInfo = line.Mid(2);
                    colInfo.Trim();
                    if (!colInfo.IsEmpty()) {
                        m_tree->AppendItem(tableNode, colInfo);
                    }
                }
            }
        }
        
        m_tree->Expand(tableNode);
    }
    
    m_tree->Expand(tablesNode);
    m_tree->Expand(root);
    
    LogMessage("Database structure loaded");
}

void MyFrame::OnTreeItemActivated(wxTreeEvent& event)
{
    wxTreeItemId item = event.GetItem();
    wxString itemText = m_tree->GetItemText(item);
    
    wxTreeItemId parent = m_tree->GetItemParent(item);
    if (parent.IsOk()) {
        wxString parentText = m_tree->GetItemText(parent);
        if (parentText.StartsWith("Tables")) {
            m_sqlInput->SetValue("SELECT * FROM " + itemText);
            LogMessage("Double-clicked table: " + itemText);
        }
    }
}

void MyFrame::OnExecute(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString textToExecute;
    
    long selStart, selEnd;
    m_sqlInput->GetSelection(&selStart, &selEnd);
    
    if (selStart != selEnd) {
        textToExecute = m_sqlInput->GetStringSelection();
        LogMessage("Executing selected text only");
    } else {
        textToExecute = m_sqlInput->GetValue();
    }
    
    textToExecute.Trim();
    
    if (textToExecute.IsEmpty()) {
        wxMessageBox("No command to execute!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    wxArrayString commands;
    wxArrayString lines = wxSplit(textToExecute, '\n');
    
    for (size_t i = 0; i < lines.GetCount(); i++) {
        wxString line = lines[i].Trim(true).Trim(false);
        
        if (line.IsEmpty() || line.StartsWith("--") || line.StartsWith("#")) {
            continue;
        }
        
        if (line.EndsWith(";")) {
            line = line.Left(line.Length() - 1).Trim();
        }
        
        if (!line.IsEmpty()) {
            commands.Add(line);
        }
    }
    
    if (commands.IsEmpty()) {
        wxMessageBox("No valid commands to execute!", "Error", wxOK | wxICON_ERROR);
        return;
    }
    
    LogMessage("========================================");
    LogMessage(wxString::Format("Executing %zu command(s)...", commands.GetCount()));
    
    bool needsRefresh = false;
    int successCount = 0;
    int errorCount = 0;
    wxString lastSelectResponse;
    wxArrayString results;
    
    for (size_t i = 0; i < commands.GetCount(); i++) {
        wxString command = commands[i];
        
        if (!connected || clientSocket == -1) {
            LogMessage("Connection lost, stopping execution");
            errorCount += (commands.GetCount() - i);
            break;
        }
        
        if (commands.GetCount() > 1) {
            LogMessage(wxString::Format("--- Command %zu/%zu ---", i + 1, commands.GetCount()));
        }
        
        LogMessage("Executing: " + command);
        
        wxString response;
        try {
            response = SendCommand(command);
        } catch (...) {
            LogMessage("Exception during SendCommand");
            response = "ERROR: Exception occurred";
        }
        
        if (response.StartsWith("ERROR: Connection lost") || response.StartsWith("ERROR: Not connected")) {
            LogMessage("Connection lost during command execution");
            errorCount += (commands.GetCount() - i);
            
            m_connectButton->SetLabel("Connect");
            m_statusLabel->SetLabel("Status: Disconnected");
            m_statusLabel->SetForegroundColour(*wxRED);
            m_executeButton->Enable(false);
            m_refreshButton->Enable(false);
            m_createTableButton->Enable(false);
            m_serverIPInput->Enable(true);
            m_serverPortInput->Enable(true);
            
            wxMessageBox("Connection lost after " + wxString::Format("%zu", i) + " command(s). Please reconnect.", 
                        "Connection Error", wxOK | wxICON_ERROR);
            return;
        }
        
        results.Add(response);
        
        LogMessage("Response: " + (response.Length() > 100 ? response.Left(100) + "..." : response));
        
        wxString cmdUpper = command.Upper();
        
        if (response.StartsWith("SUCCESS")) {
            successCount++;
        } else if (response.StartsWith("ERROR")) {
            errorCount++;
        } else {
            successCount++;
        }
        
        if (cmdUpper.StartsWith("SELECT")) {
            lastSelectResponse = response;
        }
        
        if (cmdUpper.StartsWith("INSERT") || 
            cmdUpper.StartsWith("DELETE") || 
            cmdUpper.StartsWith("UPDATE") ||
            cmdUpper.StartsWith("CREATE") ||
            cmdUpper.StartsWith("DROP") ||
            cmdUpper.StartsWith("ALTER")) {
            needsRefresh = true;
        }
        
        if (commands.GetCount() == 1 && !cmdUpper.StartsWith("SELECT")) {
            if (cmdUpper.StartsWith("LIST TABLES") || cmdUpper.StartsWith("DESCRIBE")) {
                wxMessageBox(response, "Result", wxOK | wxICON_INFORMATION);
            } else {
                wxMessageBox(response, "Server Response", wxOK | wxICON_INFORMATION);
            }
        }
        
        if (i < commands.GetCount() - 1) {
            wxMilliSleep(50);
            wxYield();
        }
    }
    
    if (!lastSelectResponse.IsEmpty()) {
        LogMessage("Processing SELECT results...");
        ParseSelectResult(lastSelectResponse);
    }
    
    if (needsRefresh && connected) {
        LogMessage("Data modified, refreshing structure...");
        LoadDatabaseStructure();
    }
    
    if (commands.GetCount() > 1) {
        wxString summary = wxString::Format("Executed %zu commands:\nSuccess: %d\nErrors: %d", 
                                            commands.GetCount(), successCount, errorCount);
        LogMessage(summary);
        wxMessageBox(summary, "Execution Complete", wxOK | wxICON_INFORMATION);
    }
    
    LogMessage("Command execution completed");
}

void MyFrame::ParseSelectResult(const wxString& result)
{
    LogMessage("=== ParseSelectResult START ===");
    LogMessage("Input length: " + wxString::Format("%zu", result.Length()));
    
    try {
        int numCols = m_grid->GetNumberCols();
        int numRows = m_grid->GetNumberRows();
        
        LogMessage(wxString::Format("Grid before clear: %d cols, %d rows", numCols, numRows));
        
        if (numCols > 0) {
            m_grid->DeleteCols(0, numCols);
        }
        if (numRows > 0) {
            m_grid->DeleteRows(0, numRows);
        }
        
        LogMessage("Grid cleared OK");
    } catch (...) {
        LogMessage("ERROR: Exception while clearing grid");
        return;
    }

    if (result.IsEmpty()) {
        LogMessage("Empty result");
        wxMessageBox("Empty response from server", "Error", wxOK | wxICON_ERROR);
        return;
    }

    wxString resultTrimmed = result;
    resultTrimmed.Trim(true).Trim(false);
    
    LogMessage("Result preview: " + resultTrimmed.Left(100));

    if (resultTrimmed.StartsWith("ERROR")) {
        LogMessage("Server returned error");
        wxMessageBox(resultTrimmed, "Query Error", wxOK | wxICON_ERROR);
        return;
    }

    if (resultTrimmed == "0 rows" || resultTrimmed.StartsWith("0 row")) {
        LogMessage("No rows returned");
        wxMessageBox("No rows found", "Result", wxOK | wxICON_INFORMATION);
        return;
    }

    std::vector<wxString> columns;
    std::vector<std::vector<wxString>> rows;
    
    wxArrayString lines = wxSplit(resultTrimmed, '\n');
    LogMessage(wxString::Format("Split into %zu lines", lines.GetCount()));
    
    for (size_t i = 0; i < lines.GetCount(); i++) {
        wxString line = lines[i];
        line.Trim(true).Trim(false);
        
        if (line.IsEmpty()) {
            continue;
        }
        
        LogMessage(wxString::Format("Line %zu: [%s]", i, line.Left(60)));
        
        if (line.Contains("row") && line.EndsWith(":")) {
            LogMessage("  -> Skipping summary line");
            continue;
        }
        
        if (!line.StartsWith("Row ")) {
            LogMessage("  -> Skipping (not a Row line)");
            continue;
        }
        
        int colonPos = line.Find(':');
        if (colonPos == wxNOT_FOUND || colonPos < 4) {
            LogMessage("  -> Skipping (no colon found)");
            continue;
        }
        
        wxString dataPart = line.Mid(colonPos + 1);
        dataPart.Trim(true).Trim(false);
        
        LogMessage("  -> Data: [" + dataPart.Left(50) + "]");
        
        if (dataPart.IsEmpty()) {
            LogMessage("  -> Skipping (empty data)");
            continue;
        }
        
        std::vector<wxString> rowData;
        std::vector<wxString> rowColumns;
        
        wxArrayString pairs = wxSplit(dataPart, ' ');
        
        for (size_t j = 0; j < pairs.GetCount(); j++) {
            wxString pair = pairs[j];
            pair.Trim(true).Trim(false);
            
            if (pair.IsEmpty()) continue;
            
            int eqPos = pair.Find('=');
            if (eqPos == wxNOT_FOUND || eqPos == 0) {
                continue;
            }
            
            wxString colName = pair.Left(eqPos);
            wxString value = pair.Mid(eqPos + 1);
            
            LogMessage(wxString::Format("    -> [%s] = [%s]", colName, value));
            
            rowColumns.push_back(colName);
            rowData.push_back(value);
        }
        
        if (columns.empty() && !rowColumns.empty()) {
            columns = rowColumns;
            LogMessage(wxString::Format("  -> Established %zu columns", columns.size()));
        }
        
        if (!rowData.empty()) {
            rows.push_back(rowData);
            LogMessage(wxString::Format("  -> Added row with %zu values", rowData.size()));
        }
    }
    
    LogMessage(wxString::Format("Parsing complete: %zu columns, %zu rows", columns.size(), rows.size()));
    
    if (columns.empty()) {
        LogMessage("ERROR: No columns parsed");
        wxMessageBox("Could not parse columns from:\n" + resultTrimmed.Left(200), "Parse Error", wxOK | wxICON_ERROR);
        return;
    }
    
    if (rows.empty()) {
        LogMessage("ERROR: No rows parsed");
        wxMessageBox("Could not parse rows from:\n" + resultTrimmed.Left(200), "Parse Error", wxOK | wxICON_ERROR);
        return;
    }
    
    try {
        LogMessage("Creating grid columns...");
        
        if (!m_grid->AppendCols(static_cast<int>(columns.size()))) {
            LogMessage("ERROR: AppendCols failed");
            wxMessageBox("Failed to create grid columns", "Error", wxOK | wxICON_ERROR);
            return;
        }
        
        for (size_t i = 0; i < columns.size(); i++) {
            m_grid->SetColLabelValue(static_cast<int>(i), columns[i]);
        }
        
        LogMessage("Creating grid rows...");
        
        for (size_t r = 0; r < rows.size(); r++) {
            if (!m_grid->AppendRows(1)) {
                LogMessage(wxString::Format("ERROR: AppendRows failed at row %zu", r));
                break;
            }
            
            for (size_t c = 0; c < rows[r].size() && c < columns.size(); c++) {
                m_grid->SetCellValue(static_cast<int>(r), static_cast<int>(c), rows[r][c]);
            }
        }
        
        LogMessage("Auto-sizing columns...");
        m_grid->AutoSizeColumns();
        m_grid->ForceRefresh();
        
        LogMessage(wxString::Format("SUCCESS: Displayed %zu rows x %zu cols", rows.size(), columns.size()));
        
    } catch (const std::exception& e) {
        LogMessage(wxString("EXCEPTION: ") + e.what());
        wxMessageBox(wxString("Exception: ") + e.what(), "Error", wxOK | wxICON_ERROR);
    } catch (...) {
        LogMessage("UNKNOWN EXCEPTION in grid population");
        wxMessageBox("Unknown exception while creating grid", "Error", wxOK | wxICON_ERROR);
    }
    
    LogMessage("=== ParseSelectResult END ===");
}

void MyFrame::UpdateTree()
{
    m_tree->DeleteAllItems();
    wxTreeItemId root = m_tree->AddRoot("Database");
    
    wxTreeItemId tablesNode = m_tree->AppendItem(root, "Not connected");
    
    m_tree->ExpandAll();
}