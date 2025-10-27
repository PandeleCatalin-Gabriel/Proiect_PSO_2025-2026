## Clase Implementate

### DataType_Interface
- Clasa abstracta pentru tipuri de date
- Operatori de comparatie (<, >, ==, <=, >=)
- Metode: toString(), getType(), clone()

### DataType_Int
- Tip de date intreg
- Stocare si comparare valori int
- Conversie la/de la string

### DataType_Varchar
- Tip de date string cu lungime variabila
- Validare lungime maxima
- Stocare VARCHAR(N)

### DataType_Date
- Tip de date pentru date calendaristice
- Format: YYYY-MM-DD
- Validare format

### Column
- Definitie coloana tabel
- Proprietati: nume, tip, lungime maxima, PRIMARY KEY, NOT NULL, index
- Getters/setters pentru toate proprietatile

### Row
- Stocare date un rand din tabel
- rowID intern unic
- Map pentru date coloane: map<nume_coloana, valoare>
- Metode: getValue(), setValue(), hasColumn()

### BTree
- Implementare B-tree template pentru indexare
- Operatii: insert(), search() - O(log n)
- Auto-echilibrare prin splitare noduri
- Folosit pentru PRIMARY KEY

### Tabel
- Gestionare structura si date tabel
- Schema: addColumn(), getColumn(), hasColumn()
- CRUD: insertRow(), selectByPrimaryKey(), selectAll(), updateRow(), deleteRow()
- Validare: validateRow() - verificare tipuri si constrainte
- Indexare automata PRIMARY KEY cu B-tree

### Database
- Gestionare tabele multiple
- Operatii: createTable(), dropTable(), getTable(), hasTable(), listTables()
- Thread-safe cu mutex
- Persistenta: saveToFile(), loadFromFile()

### DatabaseManager
- Gestionare conexiuni client
- O baza de date per manager
- Tracking conexiuni: registerConnection(), closeConnection(), listActiveConnections()
- Persistenta JSON: saveDatabase(), loadDatabase()
- Initializare director date: initializeDataDirectory()

### ClientConnection
- Structura pentru informatii conexiune client
- Campuri: connectionId, clientIP, clientPort, isActive, threadId

### Command_Interface
- Interfata abstracta pentru comenzi SQL
- Metoda: execute(Database&) -> string

### Insert_Command
- Parsare comanda INSERT INTO table VALUES (...)
- Extragere: tableName, values[]
- Convertire valori la tipuri DataType
- Validare si inserare in tabel

### Delete_Command
- Parsare comanda DELETE FROM table WHERE column = value
- Extragere: tableName, whereColumn, whereValue
- Validare PRIMARY KEY
- Stergere rand din tabel

### Select_Command
- Parsare comanda SELECT * FROM table [WHERE column = value]
- Suport pentru scan complet sau WHERE pe PRIMARY KEY
- Returnare rezultate formatate
- Afisare randuri gasite

---

## Functionalitati Principale

**Operatii Tabel:**
- Creare schema tabel cu coloane tipizate
- Inserare randuri cu validare automata
- Selectie dupa PRIMARY KEY (cautare rapida O(log n))
- Selectie toate randurile
- Update randuri existente
- Stergere randuri dupa PRIMARY KEY

**Validari:**
- Verificare tipuri de date (INT, VARCHAR, DATE)
- Aplicare constrainte NOT NULL
- Validare lungime maxima VARCHAR
- Verificare existenta PRIMARY KEY

**Persistenta:**
- Salvare baza de date in format JSON
- Incarcare baza de date din JSON
- Creare automata directoare pentru stocare

**Parsare Comenzi:**
- INSERT INTO table VALUES (val1, val2, ...)
- DELETE FROM table WHERE column = value
- SELECT * FROM table [WHERE column = value]

**Thread Safety:**
- Operatii database thread-safe cu mutex
- Tracking conexiuni multiple clienti
