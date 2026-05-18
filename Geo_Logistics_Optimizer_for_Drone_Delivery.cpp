#include <iostream>

using namespace std;

//Node dasar untuk linked list, queue, dan stack paket
struct NodePaket {
int idPaket;
double bobot;
NodePaket* next; // Pointer untuk menghubungkan ke node selanjutnya
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

int main() {

CustomQueue antrianPaket;

cout << "=== UJI COBA QUEUE ==" << endl;
antrianPaket.enqueue(10);
antrianPaket.enqueue(20);
antrianPaket.enqueue(30);
antrianPaket.enqueue(40);
antrianPaket.enqueue(50);

antrianPaket.tampilkanqueue();
antrianPaket.peek();

cout << "=== UJI COBA DEQUEUE ==" << endl;
antrianPaket.dequeue();
antrianPaket.dequeue();
antrianPaket.tampilkanqueue();
antrianPaket.peek();

    return 0;
}