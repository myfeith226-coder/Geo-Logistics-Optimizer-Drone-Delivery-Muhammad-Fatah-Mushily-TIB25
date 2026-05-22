#include <iostream>

using namespace std;

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

class DoublyLinkedList { // Class Doubly LinkedList
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
    void enqueue(int idPaket, double bobot = 0.0) {
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

int main() {

CustomQueue antrianPaket;

cout << "=== UJI COBA STACK ==" << endl;
CustomStack rutePaket;

rutePaket.push('A');
rutePaket.push('B');
rutePaket.push('C');
cout << "Elemen teratas: " << endl;
rutePaket.peek();

rutePaket.pop();
cout << "Elemen teratas setelah pop: " << endl;
rutePaket.peek();


    return 0;
}