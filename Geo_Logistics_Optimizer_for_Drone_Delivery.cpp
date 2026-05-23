#include <iostream>


using namespace std;

void screen() {
    cout << "====== GEO-LOGISTIC OPTIMIZER DRONE DELIVERY ======" << endl;
}

//Node dasar linkedlist untuk queue
struct NodePaket {
int idPaket;
double bobot;
NodePaket* next; // Pointer untuk menghubungkan ke node selanjutnya
};

struct NodeRute { // Node dasar linkedlist untuk stack
    char namaTujuan;
    NodeRute* next;
};

//Node dasar untuk daftar drone yang aktif Menggunakan doubly linkedlist
struct DroneAktif {
string droneAktif;
DroneAktif* next;
DroneAktif* prev;
};

// Node untuk Hash Table
struct NodeHashTable {
    int idPaket; // key
    int value;
    NodeHashTable* hashTable[100];
    NodeHashTable* next;

    NodeHashTable(int k, int v) : idPaket(k), value(v), next(nullptr) {}
};

class CustomQueue {
    private:
    NodePaket *Head;
    NodePaket *Tail;

    public:
    // Konstruktor untuk menjadikan antrean awal menjadi kosong
    CustomQueue() {
        Head = nullptr;
        Tail = nullptr;
    }

    // Destruktor untuk membersihkan memori saat program selesai agar tidak bocor
    ~CustomQueue() {
        while(!isEmpty()) {
            dequeue();
        }
    }

    // Fungsi untuk cek apakah antrian sudah kosong
    bool isEmpty() {
        return (Head == nullptr);
    }

    // Menambah elemen ke belakang antrean (enqueue)
    void enqueue(int idPaket, int bobot = 0) {
        NodePaket* nodebaru = new NodePaket();
        nodebaru->idPaket = idPaket;
        nodebaru->bobot = bobot;
        nodebaru->next = nullptr;

        // JIka antrean masih kosong nodebaru akan menjadi head sekaligus tail
        if(isEmpty()) {
            Head = nodebaru;
            Tail = nodebaru;
        } else {
            Tail->next = nodebaru; // JIka antrian sudah ada isinya maka Tail disambungkan dengan nodebaru
            Tail = nodebaru; // Menggeser status Tail menjadi nodebaru
        }
        cout << "ID paket " << idPaket << "Berhasil di tambahkan!" << endl;
    }

    // Fungsi Dequeue untuk menghapus elemen dari posisi depan antrean (Head)
    void dequeue() {
        if(isEmpty()) {
        cout << "Antrean kosong! tidak ada data antrean paket yang bisa di hapus" << endl;
        return;
        }

        // menyimpan sementara node depan yang akan di hapus
        NodePaket* temp = Head;
        int datadihapus = temp->idPaket;

        // Menggeser penunjuk Head ke node di belakangnya
        Head = Head->next;

        // set Tail menjadi nullptr jika sesudah penunjuk Head di pindah ke node di belakangnya antrian menjadi kosong
        if(Head == nullptr) {
            Tail = nullptr;
        }

        // Menghapus node dari memori asli
        delete temp;
        cout << datadihapus << " telah keluar dari antrean " << endl;
    }

    // Fungsi untuk menampilkan elemen queue paling depan
    void peek() {
        if(isEmpty()) {
            cout << "Antrean paket Kosong!" << endl;
        } else {
            cout << "Elemen paling depan" << Head->idPaket << endl;
        }
    }

    // Melihat semua antrian
    void tampilkanqueue() {
        if(isEmpty()) {
            cout << "Antrean paket Kosong!" << endl;
            return;
        }

        cout << "Isi antrian paket saat ini: " << endl;
        NodePaket* saatini = Head;
        while(saatini != nullptr) {
            cout << saatini->idPaket << "->";
            saatini = saatini->next;
        }
        cout << "NULL" << endl;
    }
};

class CustomStack {
    private:
    NodeRute* top; //Pointer top untuk menunjuk ke elemen paling atas

    public:
    CustomStack() {
        top = nullptr; //inisialisasi stack kosong
    }
    // Fungsi untuk cek apakah stack kosong
    bool isEmpty() {
        return top == nullptr;
    }

    // Fungsi push untuk menambah nilai atau elemen ke posisi paling atas (head/top)
    void push(char namaTujuan) {
        NodeRute* newnode = new NodeRute();
        newnode->namaTujuan = namaTujuan;
        newnode->next = top;
        top = newnode;
        cout << "Rute " << namaTujuan << " berhasil di tambahkan ke Stack" << endl;
    }

    void pop() {
        if(isEmpty()) {
            cout << "Tidak ada rute yang dapat di lacak!" << endl;
            return; 
        }
        NodeRute* temp = top; // Simpan alamat top
        top = top->next; // Geser top ke node di bawahnya
        cout << "Rute " << temp->namaTujuan << " berhasil di hapus!" << endl;
        delete temp; // Membersihkan memori dari node yang di hapus 
    }

    // Fungsi untuk melihat riwayat terakhir rute
    void peek() {
        if(isEmpty()) {
            cout << "Riwayat masih kosong!" << endl;
            return;
        }
        cout << "Riwayat rute terakhir: " << top->namaTujuan << endl;
    }

    // Destructor untuk membersihkan sisa memori dari objek yang hancur (sudah di hapus)
    ~CustomStack() {
        while(!isEmpty()) {
            NodeRute* temp = top;
            top = top->next;
            delete temp;
        }
    }
};

class DoublyLinkedList { // Class Doubly LinkedList untuk data drone yang aktif
private:
DroneAktif* Head;
DroneAktif* Tail;

public:
DoublyLinkedList() { //Inisialisasi node Prev(Head) dan node next(Prev)
    Head = nullptr;
    Tail = nullptr;
}

void insertEnd(string droneAktif) { //Menambah data drone yang aktif di belakang(Tail)
DroneAktif* newnode = new DroneAktif();
newnode->droneAktif = droneAktif;
newnode->next = nullptr;
newnode->prev = nullptr;

if(Head == nullptr) {
    Head = newnode;
    Tail = newnode;
} else {
    Tail->next = newnode;
    newnode->prev = Tail;
    Tail = newnode;
}
}

// Menambah data drone yang aktif di depan (Head)
void insertFront(string droneAktif) {
DroneAktif* newnode = new DroneAktif();
newnode->droneAktif = droneAktif;
newnode->next = Head;
newnode->prev = nullptr;

if(Head == nullptr) {
    Head = newnode;
    Tail = newnode;
} else {
    Head->prev = newnode;
    Head = newnode;
}
}

// Menghapus data
void deleteData(string value) {
if(Head == nullptr) {
    cout << "Data drone aktif: kosong | tidak ada yang bisa di hapus" << endl;
    return;
}

// Mencari data berdasarkan nilai yang di cari
DroneAktif* current = Head;
while(current != nullptr && current->droneAktif != value) {
    current = current->next;
}

if(current == nullptr) { //Kondisi jika data tidak di temukan
    cout << "Data drone tidak di temukan!" << endl;
    return;
}

// Kondisi jika yang di hapus adalah head
if(current == Head) {
    Head = current->next;
    if(Head != nullptr) {
        Head->prev = nullptr;
    } else {
        Tail = nullptr; // Jika list menjadi kosong
    }
} else if(current == Tail) {  //Kondisi jika yang dihapus adalah Tail
    Tail = current->prev;
    Tail->next = nullptr;
} else {
    current->prev->next = current->next;
    current->next->prev = current->prev;
}

delete current; // Hapus data dari memori
}

// Menampilkan data drone aktif dari depan ke belakang
void displayForward() {
    DroneAktif* current = Head;
    cout << "Drone Aktif(Depan): ";
    while(current != nullptr) {
        cout << current->droneAktif << "-> ";
        current = current->next;
    }
    cout << "NULL" << endl;
}

// Menampilkan data drone aktif dari belakang ke depan
void displayBackward() {
    DroneAktif* current = Tail;
    cout << "Drone Aktif (Belakang): ";
    while(current != nullptr) {
        cout << current->droneAktif << "-> ";
        current = current->prev;
    }
    cout << "NULL" << endl;
}

// Membersihkan untuk membersihkan memori saat program selesai
~DoublyLinkedList() {
    DroneAktif* current = Head;
    while(current != nullptr) {
        DroneAktif* nextNode = current->next;
        delete current;
        current = nextNode;
    }
}
};

class HashTable { // class hash table untuk menyimpan paket sesuai ID
private:
NodeHashTable** table; //Bucket
int capacity; // ukuran total kapasitas array
int size; // jumlah elemen yang tersimpan

int hashFunction(int& idPaket) {
    int hash = idPaket % 100;

    // Jika key bernilai negatif di jaga tetap positif
    if(hash < 0) {
        hash += capacity;
    }
    return hash;
}

// Re hashing manual saat array penuh
void resize() {
    int oldCapacity = capacity;
    capacity *= 2; // Menggandakan ukuran array statis di belakang layar

    NodeHashTable** oldTable = table;

    // Alokasi memori array dinamis baru
    table = new NodeHashTable*[capacity];
    for(int i = 0; i < capacity; i++) {
        table[i] = nullptr; // Inisialisasi table kosong
    }

    size = 0; // size di reset karena fungsi insert akan menghitung ulang
    
    // Memindahkan data dari tabel lama ke tabel baru
    for(int i = 0; i < oldCapacity; i++) {
        NodeHashTable* current = oldTable[i];
        while(current != nullptr) {
            insert(current->idPaket, current->value); // re hash ke indeks baru

            NodeHashTable* temp = current;
            current = current->next;
            delete temp; // Menghapus node lama untuk mencegah kebocoran memori
        }
    }
    delete[] oldTable; // Menghapus array pointer lama
}

public:
// Konstruktor
HashTable(int initCapacity = 100) {
    capacity = initCapacity;
    size = 0;

    table = new NodeHashTable*[capacity];
    for(int i = 0; i < capacity; i++) {
        table[i] = nullptr;
    }
}

// Destructor untuk membersihkan semua memori dinamis
~HashTable() {
    for(int i = 0; i < capacity; i++) {
        NodeHashTable* current = table[i];
        while(current != nullptr) {
            NodeHashTable* temp = current;
            current = current ->next;
            delete temp;
        }
    }
    delete[] table;
}

// Operasi penyisipan data (insert / Update)
void insert(int idPaket, int value) {
    // Melakukan resize jika load factor mencapai atau melebihi 75%
    // MEnggunakan perkalia silang untuk menghindari casting float
    if(size * 4 >= capacity * 3) {
        resize();
    }

    int index = hashFunction(idPaket);
    NodeHashTable* current = table[index];

    // Melakukan update jika key sudah terdaftar
    while(current != nullptr) {
        if(current ->idPaket == idPaket) {
            current->value = value;
            return;
        }
        current = current->next;
    }

    // Melakukan Chaining (sisipkan data di awal linkedlist) jika key baru
    NodeHashTable* newnode = new NodeHashTable(idPaket, value);
    newnode->next = table[index];
    table[index] = newnode;
    size++;
}

// Operasi Pencarian data (search)
// Mengembalikan nilai -1, jika key tidak di temukan
int search(int idPaket) {
    int index = hashFunction(idPaket);
    NodeHashTable* current = table[index];

    while(current != nullptr) {
        if(current->idPaket == idPaket) {
            return current ->value;
        }
        current = current->next;
    }
    return -1;
} 

// Operasi penghapusan data (delete)
void remove(int idPaket) {
    int index = hashFunction(idPaket);
    NodeHashTable* current = table[index];
    NodeHashTable* prev = nullptr;

    while(current != nullptr) {
        if(current ->idPaket == idPaket) {
            if(prev == nullptr) {
                table[index] = current->next; // Node berada di kepala list
            } else {
                prev->next = current->next; // Node berada di tengah list
            }
            delete current;
            size--;
            cout << "ID paket " << idPaket << " berhasil di hapus" << endl;
            return;
        }
        prev = current;
        current = current ->next;
    }
    cout << "ID paket " << idPaket << " tidak di temukan" << endl;
}
};

// Fungsi tukar paket
void tukar(int& a, int& b) {
    int temp = a;
    a  = b;
    b = temp;
}

//  partisi quicksort
int partisi(int arr[], int ringan, int berat) {
    int pivot = arr[berat]; // pivot di ambil dari elemen terakhir
    int i = ringan - 1;
    for(int j = ringan; j < berat; j++) {
        if(arr[j] < pivot) {
            i++;
            tukar(arr[i], arr[j]);
        }
    }
    tukar(arr[i + 1], arr[berat]);
    return i + 1;
}

void quicksort(int arr[], int ringan, int berat) {
    if(ringan < berat) {
        int p = partisi(arr, ringan, berat);
        quicksort(arr, ringan, p - 1);
        quicksort(arr, p + 1, berat);
    }
}

int binarySearch(int* arr, int IdPaket, int target) {
    int kiri = 0;
    int kanan = IdPaket - 1;
    
    while(kiri <= kanan) {
        // rumus menghitumg titik tengah untuk menghindari overflow
        int tengah = kiri + (kanan - kiri) / 2;

        if(arr[tengah] == target) {
            return tengah;
        }

        if(arr[tengah] < target) {
            kiri = tengah + 1;
        } else {
            kanan = tengah - 1;
        }
    }

    // Mengembalikan nilai -1 jika data tidak di temukan
    return -1;
}

int main() {

int n, idPaket, target;
string nama_paket;    

DoublyLinkedList list;
CustomStack stack;
CustomQueue antrian;

cout << "=== TESTING QUICK SORT" << endl;
cout << "Masukkan jumlah paket yang masuk: ";
cin >> n;

//Array dinamis di memory heap
int* dataPaket = new int[n];

cout << "Masukkan " << n << " ukuran paket:" << endl;
for(int i = 0; i < n; i++) {
    cout << "Ukuran paket ke- " << i + 1 << ": ";
    cin >> dataPaket[i];
}

quicksort(dataPaket, 0, n - 1);

cout << "Hasil pengurutan: " << endl;
for(int i = 0; i < n; i++) {
    cout << dataPaket[i] << " ";
}
cout << endl;

delete[] dataPaket;
cout << "===========================" << endl;
cout << "===  TESTING SEARCHING  ===" << endl;
cout << "Masukkan jumlah paket yang masuk: ";
cin >> idPaket;

int* data = new int[idPaket];

cout << "Masukkan ID paket: ";
for(int i = 0; i < idPaket; i++) {
    cout << "Elemen ke- " << i + 1 << ": ";
    cin >> data[i];
}

cout << "Masukkan Id Paket yang ingin di cari: ";
cin >> target;

int hasil = binarySearch(data, idPaket, target);

if(hasil != -1) {
    cout << "ID Paket berhasil di temukan!" << endl;
} else {
    cout << "ID paket tidak di temukan!" << endl;
}

delete[] data;

    return 0;
}