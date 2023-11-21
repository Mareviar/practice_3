#include <winsock2.h>
#include <iostream>
#include <string>
#include <mutex>
#pragma warning(disable: 4996)


#pragma comment(lib, "ws2_32.lib")

using namespace std;

struct Pair {
    string Key;
    string Value;

    void ppush(const string& key, const string& value) {
        Key = key;
        Value = value;
    }
};

struct Node_Table {
    Pair Data;
    Node_Table* Next;
    Node_Table* Previous;
};

struct Doubly_Connected_Table {
    int Length;
    Node_Table* Head;
    Node_Table* Tail;

    Doubly_Connected_Table() : Length(0), Head(nullptr), Tail(nullptr) {}

    bool dpush(const Pair& val) {
        Node_Table* node_hash = new Node_Table{ val, nullptr, nullptr };

        if (Head == nullptr) {
            Head = node_hash;
            Tail = node_hash;
        }
        else {
            Node_Table* currentNode = Head;
            while (currentNode != nullptr) {
                if (currentNode->Data.Key == val.Key) {
                    delete node_hash;
                    return false; 
                }
                currentNode = currentNode->Next;
            }

            Tail->Next = node_hash;
            node_hash->Previous = Tail;
            Tail = node_hash;
        }

        Length++;
        return true;
    }

    Pair ddel(const string& val) {
        Node_Table* currentNode = Head;

        if (currentNode == nullptr) {
            Pair removedPair;
            removedPair.Key = "Table is clear";
            return removedPair;
        }

        while (currentNode != nullptr) {
            if (currentNode->Data.Key == val) {
                Pair removedPair = currentNode->Data;

                if (currentNode == Head) {
                    Head = currentNode->Next;
                    if (Head != nullptr) {
                        Head->Previous = nullptr;
                    }
                }
                else if (currentNode == Tail) {
                    Tail = currentNode->Previous;
                    if (Tail != nullptr) {
                        Tail->Next = nullptr;
                    }
                }
                else {
                    currentNode->Previous->Next = currentNode->Next;
                    currentNode->Next->Previous = currentNode->Previous;
                }

                delete currentNode;
                Length--;
                return removedPair;
            }

            currentNode = currentNode->Next;
        }
        Pair removedPair;
        removedPair.Key = "Key not found";
        return removedPair;
    }
};

struct Hash_Table {
    Doubly_Connected_Table** Table;
    int Size;
	int Cout = 0; 

    Hash_Table(int size) : Size(size) {
        Table = new Doubly_Connected_Table*[size];
        for (int i = 0; i < size; ++i) {
            Table[i] = new Doubly_Connected_Table();
        }
    }

    ~Hash_Table() {
        for (int i = 0; i < Size; ++i) {
            delete Table[i];
        }
        delete[] Table;
    }

    int Hash(const string& key) {
        int key_int = 0;
        for (char symbol : key) {
            key_int += static_cast<int>(symbol);
        }
        return key_int % Size;
    }

    bool Hset(const string& key, const string& value) {
        Pair val;
        val.ppush(key, value);
        int hash = Hash(val.Key);

        if (Table[hash]->Length < 20) {
            return Table[hash]->dpush(val);
        }
        else {
            int oldSize = Size;
            Hash_Table newHT(oldSize * 2);

            for (int i = 0; i < oldSize; i++) {
                Node_Table* currentNode = Table[i]->Head;
                while (currentNode != nullptr) {
                    int new_hash = newHT.Hash(currentNode->Data.Key);
                    newHT.Table[new_hash]->dpush(currentNode->Data);
                    currentNode = currentNode->Next;
                }
            }

            *this = newHT;
            int new_hash = Hash(val.Key);
            return Table[new_hash]->dpush(val);
        }
    }

    Pair Hdel(const string& key) {
        int hash = Hash(key);
        return Table[hash]->ddel(key);
    }

    string Hget(const string& key) {
        int hash = Hash(key);
        Node_Table* currentNode = Table[hash]->Head;
        while (currentNode != nullptr) {
            if (currentNode->Data.Key == key) {
                return currentNode->Data.Value;
            }
            currentNode = currentNode->Next;
        }
        return "Element not found"; 
    }
};

Hash_Table hashTable(10);
SOCKET Connections[100];
string command[2];
std::mutex mute;

void Parsing(char* cmd) { 
	int j = 0, size = 0;
	for (int i = 0; i < 2; i++) {
		string part = "";
		for (j; cmd[j] != '\0'; j++) {
			if (cmd[j] == '\n') {
				j++;
				break;
			}
			else {
				part += cmd[j];
			}
		}
		command[i] = part;
	}
}

void ClientHandler(LPVOID lpParam) {
    SOCKET clientSocket = *((SOCKET*)lpParam);
    delete (SOCKET*)lpParam; 

    int msg_size = 1024;
    char* msg = new char[msg_size + 1];
    msg[msg_size] = '\0';
    recv(clientSocket, msg, msg_size, 0);
    Parsing(msg);
    cout<<command[0]<<command[1]<<endl;

    mute.lock();
	
    if (command[0] == "post") {
        if (command[1].empty()) {
            string error = "Ошибка при добавлении";
            int err_size = error.size();

            send(clientSocket, error.c_str(), err_size, 0);
        } else {
            string base = "http://10.241.125.222/" + to_string(hashTable.Cout + 1);
            hashTable.Hset(base, command[1]);
            hashTable.Cout++;

            int base_size = base.size();
            send(clientSocket, base.c_str(), base_size, 0);
        }
    } else if (command[0] == "get"){
        if (command[1].empty()) {
            string error = "Ошибка при добавлении";
            int err_size = error.size();
            send(clientSocket, error.c_str(), err_size, 0);
        } else {
            string response = hashTable.Hget(command[1]);
            int base_size = response.size();
            send(clientSocket, response.c_str(), base_size, 0);
        }
    } else {
        string error = "неверная команда";
        int err_size = error.size();
        send(clientSocket, (char*)&err_size, sizeof(int), 0);
        send(clientSocket, error.c_str(), err_size, 0);
    }
    mute.unlock();

    closesocket(clientSocket);
    delete[] msg;
}

int main() {
    setlocale(LC_ALL, "Rus");

    WSADATA wsaData;
    WORD DLLVersion = MAKEWORD(2, 1);
    if (WSAStartup(DLLVersion, &wsaData) != 0) {
        cerr << "Error in WSAStartup\n";
        return 1;
    }

    SOCKADDR_IN addr;
    int sizeofaddr = sizeof(addr);
    addr.sin_addr.s_addr = inet_addr("10.241.125.222");
    addr.sin_port = htons(6379);
    addr.sin_family = AF_INET;

    SOCKET sListen = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(sListen, (SOCKADDR*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        cerr << "Error binding socket\n";
        closesocket(sListen);
        WSACleanup();
        return 1;
    }

    listen(sListen, SOMAXCONN);

    SOCKET newConnection;
    for (int i = 0; i < 100; i++) {
        cout << endl << "Waiting for activity... ";
        newConnection = accept(sListen, (SOCKADDR*)&addr, &sizeofaddr);

        if (newConnection == INVALID_SOCKET) {
            cerr << "Error accepting connection\n";
            closesocket(sListen);
            WSACleanup();
            return 1;
        } else {
            cout << "Client Connected!\n";
            SOCKET* newConnectionCopy = new SOCKET(newConnection);
            thread(ClientHandler, reinterpret_cast<LPVOID>(newConnectionCopy)).detach();
        }
    }

    closesocket(sListen);
    WSACleanup();
    system("pause");
    return 0;
}
