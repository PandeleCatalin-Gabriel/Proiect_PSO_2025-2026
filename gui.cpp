#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/treectrl.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

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
    void UpdateTree();
    
    bool ConnectToServer();
    void DisconnectFromServer();
    wxString SendCommand(const wxString& command);
    void ParseSelectResult(const wxString& result);
    void LogMessage(const wxString& message);
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
    
    connectionBox->AddStretchSpacer();
    
    m_statusLabel = new wxStaticText(mainPanel, wxID_ANY, "Status: Disconnected");
    m_statusLabel->SetForegroundColour(*wxRED);
    connectionBox->Add(m_statusLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    
    mainSizer->Add(connectionBox, 0, wxEXPAND | wxALL, 5);
    
    m_splitter = new wxSplitterWindow(mainPanel, wxID_ANY);
    
    wxPanel* leftPanel = new wxPanel(m_splitter);
    wxBoxSizer* leftSizer = new wxBoxSizer(wxVERTICAL);
    leftSizer->Add(new wxStaticText(leftPanel, wxID_ANY, "Database Tables:"), 0, wxALL, 5);
    m_tree = new wxTreeCtrl(leftPanel, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                            wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT);
    leftSizer->Add(m_tree, 1, wxEXPAND | wxALL, 5);
    leftPanel->SetSizer(leftSizer);
    
    m_rightPanel = new wxPanel(m_splitter);
    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    
    rightSizer->Add(new wxStaticText(m_rightPanel, wxID_ANY, "SQL Command:"), 0, wxALL, 5);
    m_sqlInput = new wxTextCtrl(m_rightPanel, wxID_ANY, "",
                                wxDefaultPosition, wxSize(-1, 80),
                                wxTE_MULTILINE | wxTE_PROCESS_ENTER);
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

void MyFrame::OnConnect(wxCommandEvent& event)
{
    if (!connected) {
        serverIP = m_serverIPInput->GetValue().Trim();
        long port;
        if (!m_serverPortInput->GetValue().ToLong(&port) || port < 1 || port > 65535) {
            wxMessageBox("Invalid port number!", "Error");
            return;
        }
        serverPort = port;
        
        LogMessage("Attempting to connect to " + serverIP + ":" + wxString::Format("%d", serverPort));
        
        if (ConnectToServer()) {
            m_connectButton->SetLabel("Disconnect");
            m_statusLabel->SetLabel("Status: Connected");
            m_statusLabel->SetForegroundColour(*wxGREEN);
            m_executeButton->Enable(true);
            m_serverIPInput->Enable(false);
            m_serverPortInput->Enable(false);
            LogMessage("✓ Connected successfully!");
        } else {
            LogMessage("✗ Failed to connect!");
            wxMessageBox("Failed to connect to server!\nCheck if server is running on " + 
                        serverIP + ":" + wxString::Format("%d", serverPort), "Connection Error");
        }
    } else {
        DisconnectFromServer();
        m_connectButton->SetLabel("Connect");
        m_statusLabel->SetLabel("Status: Disconnected");
        m_statusLabel->SetForegroundColour(*wxRED);
        m_executeButton->Enable(false);
        m_serverIPInput->Enable(true);
        m_serverPortInput->Enable(true);
        LogMessage("Disconnected from server");
    }
}

bool MyFrame::ConnectToServer()
{
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

    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    
    if (inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr) <= 0) {
        LogMessage("ERROR: Invalid IP address format");
        close(clientSocket);
        clientSocket = -1;
        return false;
    }

    LogMessage("Connecting to server...");
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        LogMessage("ERROR: Connection failed - " + wxString(strerror(errno)));
        close(clientSocket);
        clientSocket = -1;
        return false;
    }

    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        LogMessage("Server: " + wxString(buffer).Trim());
    }

    connected = true;
    return true;
}

void MyFrame::DisconnectFromServer()
{
    if (clientSocket != -1) {
        LogMessage("Sending QUIT command...");
        wxString quitCmd = "QUIT\n";
        send(clientSocket, quitCmd.c_str(), quitCmd.length(), 0);
        
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
    
    LogMessage("→ Sending: " + wxString(command).Trim());

    
    ssize_t sent = send(clientSocket, cmdToSend.c_str(), cmdToSend.length(), 0);
    if (sent < 0) {
        LogMessage("ERROR: Failed to send command - " + wxString(strerror(errno)));
        return "ERROR: Failed to send command";
    }

    char buffer[8192]; 
    memset(buffer, 0, sizeof(buffer));
    
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead <= 0) {
        if (bytesRead == 0) {
            LogMessage("ERROR: Connection closed by server");
        } else {
            LogMessage("ERROR: Failed to receive response - " + wxString(strerror(errno)));
        }
        DisconnectFromServer();
        m_connectButton->SetLabel("Connect");
        m_statusLabel->SetLabel("Status: Disconnected");
        m_statusLabel->SetForegroundColour(*wxRED);
        m_executeButton->Enable(false);
        return "ERROR: Connection lost";
    }

    buffer[bytesRead] = '\0';
    wxString response(buffer);
    response.Trim(true).Trim(false);
    
    LogMessage("← Received: " + response.Left(100) + (response.Length() > 100 ? "..." : ""));
    
    return response;
}

void MyFrame::OnTestConnection(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error");
        return;
    }
    
    wxString response = SendCommand("PING");
    
    if (response == "PONG") {
        wxMessageBox("Connection test successful!\nServer responded with PONG", "Test OK");
    } else {
        wxMessageBox("Connection test failed!\nExpected PONG, got: " + response, "Test Failed");
    }
}

void MyFrame::OnExecute(wxCommandEvent& event)
{
    if (!connected) {
        wxMessageBox("Please connect to server first!", "Error");
        return;
    }

    wxString command = m_sqlInput->GetValue().Trim().Trim(false);
    
    if (command.IsEmpty()) {
        wxMessageBox("Command is empty!", "Error");
        return;
    }

    wxString response = SendCommand(command);
    
    wxString cmdUpper = command.Upper();
    if (cmdUpper.StartsWith("SELECT")) {
        ParseSelectResult(response);
    } else {
        wxMessageBox(response, "Server Response");
        LogMessage("Result: " + response);
    }
}

void MyFrame::ParseSelectResult(const wxString& result)
{
    if (m_grid->GetNumberCols() > 0) m_grid->DeleteCols(0, m_grid->GetNumberCols());
    if (m_grid->GetNumberRows() > 0) m_grid->DeleteRows(0, m_grid->GetNumberRows());

    if (result.StartsWith("ERROR")) {
        LogMessage("Query error: " + result);
        wxMessageBox(result, "Query Error");
        return;
    }

    if (result.Contains("0 row") || result.Contains("No records")) {
        LogMessage("Query returned 0 rows");
        wxMessageBox("No rows found", "Result");
        return;
    }

    wxArrayString lines = wxSplit(result, '\n');
    
    std::vector<std::vector<wxString>> rows;
    std::vector<wxString> columns;
    
    LogMessage("Parsing " + wxString::Format("%zu", lines.GetCount()) + " lines...");

    for (size_t i = 0; i < lines.GetCount(); i++) {
        wxString line = lines[i].Trim().Trim(false);
        if (line.IsEmpty()) continue;

        if (!line.StartsWith("Row ")) continue;

        int colonPos = line.Find(':');
        if (colonPos == wxNOT_FOUND) continue;

        wxString dataPart = line.Mid(colonPos + 1).Trim();
        
        wxArrayString pairs = wxSplit(dataPart, ' ');
        std::vector<wxString> rowData;

        for (const auto& pair : pairs) {
            if (pair.IsEmpty()) continue;
            
            int eqPos = pair.Find('=');
            if (eqPos == wxNOT_FOUND) continue;

            wxString colName = pair.Mid(0, eqPos);
            wxString value = pair.Mid(eqPos + 1);

            if (rows.empty()) {
                columns.push_back(colName);
            }

            rowData.push_back(value);
        }

        if (!rowData.empty()) {
            rows.push_back(rowData);
        }
    }

    if (columns.empty() || rows.empty()) {
        LogMessage("Could not parse result format");
        wxMessageBox("Result format not recognized:\n" + result, "Parse Error");
        return;
    }

    LogMessage(wxString::Format("Found %zu columns and %zu rows", columns.size(), rows.size()));

    m_grid->AppendCols(columns.size());
    for (size_t i = 0; i < columns.size(); i++) {
        m_grid->SetColLabelValue(i, columns[i]);
    }

    for (size_t r = 0; r < rows.size(); r++) {
        m_grid->AppendRows(1);
        for (size_t c = 0; c < rows[r].size() && c < columns.size(); c++) {
            m_grid->SetCellValue(r, c, rows[r][c]);
        }
    }

    m_grid->AutoSizeColumns();
    m_grid->ForceRefresh();
    
    LogMessage(wxString::Format("✓ Displayed %zu rows", rows.size()));
}

void MyFrame::UpdateTree()
{
    m_tree->DeleteAllItems();
    wxTreeItemId root = m_tree->AddRoot("Database");
    
    wxTreeItemId tablesNode = m_tree->AppendItem(root, "Tables");
    
    m_tree->AppendItem(tablesNode, "connections (system)");
    
    m_tree->ExpandAll();
}