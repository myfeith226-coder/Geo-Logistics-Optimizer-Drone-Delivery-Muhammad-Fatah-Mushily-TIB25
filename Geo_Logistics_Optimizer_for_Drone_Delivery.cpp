
#include <iostream>
using namespace std;

class MyString {
private:
    char* data;
    int   panjang;

    // Hitung panjang c-string
    static int cstrLen(const char* s) {
        if (!s) return 0;
        int n = 0;
        while (s[n] != '\0') n++;
        return n;
    }

    // Salin c-string ke buffer
    static void cstrCopy(char* dst, const char* src, int n) {
        for (int i = 0; i < n; i++) dst[i] = src[i];
        dst[n] = '\0';
    }

    void alokasi(const char* s, int n) {
        panjang = n;
        data    = new char[panjang + 1];
        cstrCopy(data, s, panjang);
    }

public:
    // Konstruktor default
    MyString() : data(nullptr), panjang(0) {
        data    = new char[1];
        data[0] = '\0';
    }

    // Konstruktor dari c-string literal
    MyString(const char* s) {
        int n = cstrLen(s);
        alokasi(s, n);
    }

    // Copy constructor
    MyString(const MyString& lain) {
        alokasi(lain.data, lain.panjang);
    }

    // Assignment operator dari MyString
    MyString& operator=(const MyString& lain) {
        if (this == &lain) return *this;
        delete[] data;
        alokasi(lain.data, lain.panjang);
        return *this;
    }

    // Assignment operator dari c-string
    MyString& operator=(const char* s) {
        delete[] data;
        int n = cstrLen(s);
        alokasi(s, n);
        return *this;
    }

    ~MyString() { delete[] data; }

    int size() const { return panjang; }

    const char* cstr() const { return data; }

    char operator[](int i) const { return data[i]; }

    // Perbandingan kesetaraan
    bool operator==(const MyString& lain) const {
        if (panjang != lain.panjang) return false;
        for (int i = 0; i < panjang; i++)
            if (data[i] != lain.data[i]) return false;
        return true;
    }

    bool operator==(const char* s) const {
        int n = cstrLen(s);
        if (panjang != n) return false;
        for (int i = 0; i < panjang; i++)
            if (data[i] != s[i]) return false;
        return true;
    }

    bool operator!=(const MyString& lain) const { return !(*this == lain); }

    // Operator output ke cout
    friend ostream& operator<<(ostream& os, const MyString& ms) {
        os << ms.data;
        return os;
    }

    // Operator input dari cin
    friend istream& operator>>(istream& is, MyString& ms) {
        char buf[512];
        is >> buf;
        ms = buf;
        return is;
    }
};

//  KONSTANTA & ENUM  (manual, tanpa <climits>)
const int MAX_LOKASI   = 10;
const int HT_KAPASITAS = 64;
// INF manual: nilai double sangat besar sebagai pengganti INT_MAX
const double INF_JARAK = 1e18;

enum StatusPaket {
    MENUNGGU = 0,
    DIKIRIM  = 1,
    TERKIRIM = 2,
    GAGAL    = 3
};

enum StatusDrone {
    IDLE    = 0,
    TERBANG = 1,
    KEMBALI = 2,
    RUSAK   = 3
};

//  UTILITAS — pengganti <cstring> / <cmath>
// Nilai absolut double manual
double myAbs(double x) { return (x < 0) ? -x : x; }

// Minimal dua double
double myMin(double a, double b) { return (a < b) ? a : b; }

// Salin c-string manual
void myCStrCopy(char* dst, const char* src) {
    int i = 0;
    while (src[i]) { dst[i] = src[i]; i++; }
    dst[i] = '\0';
}

//  STRUCT DASAR
struct Koordinat {
    double   x, y;
    MyString namaLokasi;
};

struct Paket {
    int         idPaket;
    MyString    resiPaket;
    MyString    namaPenerima;
    int         lokasiTujuan;
    double      bobotKg;
    int         prioritas;     // 1 = sangat urgen, 5 = normal
    StatusPaket status;
    Paket*      next;

    Paket(int id, const char* resi, const char* nama,
          int tujuan, double bobot, int prio)
        : idPaket(id), resiPaket(resi), namaPenerima(nama),
          lokasiTujuan(tujuan), bobotKg(bobot), prioritas(prio),
          status(MENUNGGU), next(nullptr) {}
};

struct Drone {
    int         idDrone;
    MyString    namaDrone;
    double      kapasitasKg;
    double      bateraiPersen;
    int         lokasiSekarang;
    StatusDrone status;
    Paket*      muatanSaatIni;
    Drone*      next;
    Drone*      prev;

    Drone(int id, const char* nama, double kapasitas)
        : idDrone(id), namaDrone(nama), kapasitasKg(kapasitas),
          bateraiPersen(100.0), lokasiSekarang(0),
          status(IDLE), muatanSaatIni(nullptr),
          next(nullptr), prev(nullptr) {}
};

//  NODE GRAPH (Adjacency List Berbobot)
struct NodeEdge {
    int       dest;
    double    bobot;
    NodeEdge* next;

    NodeEdge(int d, double b) : dest(d), bobot(b), next(nullptr) {}
};

//  GRAPH – PETA KOTA
struct Graph {
    int        jumlahSimpul;
    NodeEdge** adjList;
    Koordinat  lokasi[MAX_LOKASI];

    Graph(int n) : jumlahSimpul(n) {
        adjList = new NodeEdge*[n];
        for (int i = 0; i < n; i++) adjList[i] = nullptr;
        // Inisialisasi nama lokasi default
        for (int i = 0; i < MAX_LOKASI; i++) {
            lokasi[i].x = 0; lokasi[i].y = 0;
            lokasi[i].namaLokasi = "";
        }
    }

    ~Graph() {
        for (int i = 0; i < jumlahSimpul; i++) {
            NodeEdge* cur = adjList[i];
            while (cur) {
                NodeEdge* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
        delete[] adjList;
    }

    void tambahEdge(int src, int dst, double bobot) {
        NodeEdge* e1 = new NodeEdge(dst, bobot);
        e1->next      = adjList[src];
        adjList[src]  = e1;

        NodeEdge* e2 = new NodeEdge(src, bobot);
        e2->next      = adjList[dst];
        adjList[dst]  = e2;
    }

    void setLokasi(int idx, double x, double y, const char* nama) {
        lokasi[idx].x         = x;
        lokasi[idx].y         = y;
        lokasi[idx].namaLokasi = nama;
    }

    void tampilkan() const {
        cout << "\n=== PETA KOTA (Graph Jalur Udara) ===" << endl;
        for (int v = 0; v < jumlahSimpul; v++) {
            cout << "  [" << v << "] " << lokasi[v].namaLokasi << " -> ";
            NodeEdge* tmp = adjList[v];
            while (tmp) {
                cout << lokasi[tmp->dest].namaLokasi
                     << "(jarak:" << tmp->bobot << "km)";
                if (tmp->next) cout << " -> ";
                tmp = tmp->next;
            }
            cout << " NULL" << endl;
        }
    }
};

//  QUEUE MANUAL – ANTREAN PAKET (FIFO)
class CustomQueue {
private:
    Paket* head;
    Paket* tail;
    int    ukuran;

public:
    CustomQueue() : head(nullptr), tail(nullptr), ukuran(0) {}

    ~CustomQueue() { while (!isEmpty()) dequeue(); }

    bool isEmpty()    const { return head == nullptr; }
    int  getUkuran()  const { return ukuran; }

    void enqueue(Paket* paket) {
        if (!paket) return;
        paket->next = nullptr;
        if (isEmpty()) { head = tail = paket; }
        else           { tail->next = paket; tail = paket; }
        ukuran++;
        cout << "  [QUEUE] Paket " << paket->resiPaket
             << " (" << paket->namaPenerima << ") masuk antrean."
             << " [Prioritas:" << paket->prioritas
             << " | Bobot:" << paket->bobotKg << "kg]" << endl;
    }

    Paket* dequeue() {
        if (isEmpty()) { cout << "  [QUEUE] Antrean kosong!" << endl; return nullptr; }
        Paket* diambil = head;
        head = head->next;
        if (!head) tail = nullptr;
        diambil->next = nullptr;
        ukuran--;
        return diambil;
    }

    Paket* peek() const { return head; }

    void tampilkan() const {
        if (isEmpty()) { cout << "  [QUEUE] Antrean kosong." << endl; return; }
        cout << "  [QUEUE] Isi antrean (" << ukuran << " paket):" << endl;
        Paket* cur = head;
        int no = 1;
        while (cur) {
            cout << "    " << no++ << ". Resi:" << cur->resiPaket
                 << " | Penerima:" << cur->namaPenerima
                 << " | Prioritas:" << cur->prioritas
                 << " | Bobot:" << cur->bobotKg << "kg" << endl;
            cur = cur->next;
        }
    }
};

//  STACK MANUAL – LOG RUTE DRONE (LIFO)
struct NodeStack {
    int       lokasiIdx;
    MyString  namaLokasi;
    double    bateraiSaatItu;
    NodeStack* next;

    NodeStack() : lokasiIdx(0), namaLokasi(""), bateraiSaatItu(0), next(nullptr) {}
};

class CustomStack {
private:
    NodeStack* top;
    int        ukuran;

public:
    CustomStack() : top(nullptr), ukuran(0) {}

    ~CustomStack() { while (!isEmpty()) { NodeStack* t = pop(); delete t; } }

    bool isEmpty()   const { return top == nullptr; }
    int  getUkuran() const { return ukuran; }

    void push(int idx, const MyString& nama, double baterai) {
        NodeStack* baru    = new NodeStack();
        baru->lokasiIdx    = idx;
        baru->namaLokasi   = nama;
        baru->bateraiSaatItu = baterai;
        baru->next         = top;
        top                = baru;
        ukuran++;
    }

    // Caller bertanggung jawab delete node yang dikembalikan
    NodeStack* pop() {
        if (isEmpty()) return nullptr;
        NodeStack* diambil = top;
        top    = top->next;
        ukuran--;
        diambil->next = nullptr;
        return diambil;
    }

    NodeStack* peek() const { return top; }

    void tampilkan() const {
        if (isEmpty()) { cout << "    (log kosong)" << endl; return; }
        NodeStack* cur = top;
        int step = ukuran;
        while (cur) {
            cout << "    Step " << step-- << ": ["
                 << cur->namaLokasi << "] baterai="
                 << cur->bateraiSaatItu << "%" << endl;
            cur = cur->next;
        }
    }
};

//  DOUBLY LINKED LIST – POOL DRONE
class PoolDrone {
private:
    Drone* head;
    Drone* tail;
    int    jumlah;

public:
    PoolDrone() : head(nullptr), tail(nullptr), jumlah(0) {}

    ~PoolDrone() {
        Drone* cur = head;
        while (cur) { Drone* tmp = cur; cur = cur->next; delete tmp; }
    }

    int getJumlah() const { return jumlah; }

    void tambahDrone(Drone* drone) {
        drone->next = nullptr;
        drone->prev = nullptr;
        if (!head) { head = tail = drone; }
        else       { tail->next = drone; drone->prev = tail; tail = drone; }
        jumlah++;
    }

    Drone* ambilDroneIdle() {
        Drone* cur = head;
        while (cur) { if (cur->status == IDLE) return cur; cur = cur->next; }
        return nullptr;
    }

    void hapusDrone(Drone* target) {
        if (!target) return;
        if (target->prev) target->prev->next = target->next;
        else              head = target->next;
        if (target->next) target->next->prev = target->prev;
        else              tail = target->prev;
        target->next = target->prev = nullptr;
        jumlah--;
    }

    void tampilkan() const {
        cout << "\n=== POOL DRONE (" << jumlah << " unit) ===" << endl;
        Drone* cur = head;
        while (cur) {
            const char* st =
                (cur->status == IDLE)    ? "IDLE"    :
                (cur->status == TERBANG) ? "TERBANG" :
                (cur->status == KEMBALI) ? "KEMBALI" : "RUSAK";
            cout << "  [D" << cur->idDrone << "] " << cur->namaDrone
                 << " | Baterai:" << cur->bateraiPersen << "%"
                 << " | Status:"  << st
                 << " | Kapasitas:" << cur->kapasitasKg << "kg" << endl;
            cur = cur->next;
        }
    }
};

//  HASH TABLE – TRACKING PAKET O(1) via RESI (Manual)
struct NodeHT {
    MyString resi;
    Paket*   paket;
    NodeHT*  next;

    NodeHT(const MyString& r, Paket* p) : resi(r), paket(p), next(nullptr) {}
};

class HashTable {
private:
    NodeHT** bucket;
    int      kapasitas;
    int      ukuran;

    // Polynomial rolling hash manual — tidak pakai fungsi library apapun
    int hashFungsi(const MyString& key) const {
        long long h = 0;
        for (int i = 0; i < key.size(); i++) {
            h = (h * 31 + (unsigned char)key[i]) % kapasitas;
        }
        if (h < 0) h += kapasitas;
        return (int)h;
    }

    void resize() {
        int      kapLama = kapasitas;
        kapasitas *= 2;
        NodeHT** lama    = bucket;

        bucket = new NodeHT*[kapasitas];
        for (int i = 0; i < kapasitas; i++) bucket[i] = nullptr;
        ukuran = 0;

        for (int i = 0; i < kapLama; i++) {
            NodeHT* cur = lama[i];
            while (cur) {
                sisipkan(cur->resi, cur->paket);
                NodeHT* tmp = cur; cur = cur->next; delete tmp;
            }
        }
        delete[] lama;
    }

public:
    HashTable(int kap = HT_KAPASITAS) : kapasitas(kap), ukuran(0) {
        bucket = new NodeHT*[kapasitas];
        for (int i = 0; i < kapasitas; i++) bucket[i] = nullptr;
    }

    ~HashTable() {
        for (int i = 0; i < kapasitas; i++) {
            NodeHT* cur = bucket[i];
            while (cur) { NodeHT* tmp = cur; cur = cur->next; delete tmp; }
        }
        delete[] bucket;
    }

    void sisipkan(const MyString& resi, Paket* paket) {
        if (ukuran * 4 >= kapasitas * 3) resize();

        int idx = hashFungsi(resi);
        NodeHT* cur = bucket[idx];
        while (cur) {
            if (cur->resi == resi) { cur->paket = paket; return; }
            cur = cur->next;
        }
        NodeHT* baru = new NodeHT(resi, paket);
        baru->next   = bucket[idx];
        bucket[idx]  = baru;
        ukuran++;
    }

    Paket* cari(const MyString& resi) const {
        int idx = hashFungsi(resi);
        NodeHT* cur = bucket[idx];
        while (cur) {
            if (cur->resi == resi) return cur->paket;
            cur = cur->next;
        }
        return nullptr;
    }

    void hapus(const MyString& resi) {
        int idx = hashFungsi(resi);
        NodeHT* cur  = bucket[idx];
        NodeHT* prev = nullptr;
        while (cur) {
            if (cur->resi == resi) {
                if (prev) prev->next = cur->next;
                else       bucket[idx] = cur->next;
                delete cur;
                ukuran--;
                cout << "  [HT] Resi " << resi << " dihapus." << endl;
                return;
            }
            prev = cur; cur = cur->next;
        }
        cout << "  [HT] Resi " << resi << " tidak ditemukan." << endl;
    }

    void tampilkanStatus(const MyString& resi) const {
        Paket* p = cari(resi);
        if (!p) { cout << "  [TRACKING] Resi " << resi << ": tidak ditemukan." << endl; return; }
        const char* st =
            (p->status == MENUNGGU) ? "MENUNGGU" :
            (p->status == DIKIRIM)  ? "DIKIRIM"  :
            (p->status == TERKIRIM) ? "TERKIRIM" : "GAGAL";
        cout << "  [TRACKING] Resi:" << resi
             << " | Penerima:" << p->namaPenerima
             << " | Status:" << st << endl;
    }
};

//  BST – DATA PELANGGAN (Rekursif Manual)
struct NodeBST {
    int      idPelanggan;
    MyString namaPelanggan;
    MyString alamat;
    int      lokasiDefault;
    NodeBST* left;
    NodeBST* right;

    NodeBST(int id, const char* nama, const char* alm, int lok)
        : idPelanggan(id), namaPelanggan(nama), alamat(alm),
          lokasiDefault(lok), left(nullptr), right(nullptr) {}
};

class BSTPelanggan {
private:
    NodeBST* root;

    NodeBST* insertRekursif(NodeBST* node, int id, const char* nama,
                             const char* alm, int lok) {
        if (!node) return new NodeBST(id, nama, alm, lok);
        if      (id < node->idPelanggan) node->left  = insertRekursif(node->left,  id, nama, alm, lok);
        else if (id > node->idPelanggan) node->right = insertRekursif(node->right, id, nama, alm, lok);
        return node;
    }

    NodeBST* searchRekursif(NodeBST* node, int id) const {
        if (!node || node->idPelanggan == id) return node;
        if (id < node->idPelanggan) return searchRekursif(node->left,  id);
        return searchRekursif(node->right, id);
    }

    void inorderRekursif(NodeBST* node) const {
        if (!node) return;
        inorderRekursif(node->left);
        cout << "    ID:" << node->idPelanggan
             << " | " << node->namaPelanggan
             << " | " << node->alamat
             << " | LokasiDefault:" << node->lokasiDefault << endl;
        inorderRekursif(node->right);
    }

    void hapusSemuaRekursif(NodeBST* node) {
        if (!node) return;
        hapusSemuaRekursif(node->left);
        hapusSemuaRekursif(node->right);
        delete node;
    }

    NodeBST* minVal(NodeBST* node) const {
        while (node->left) node = node->left;
        return node;
    }

    NodeBST* deleteRekursif(NodeBST* node, int id) {
        if (!node) return nullptr;
        if      (id < node->idPelanggan) node->left  = deleteRekursif(node->left,  id);
        else if (id > node->idPelanggan) node->right = deleteRekursif(node->right, id);
        else {
            if (!node->left)  { NodeBST* t = node->right; delete node; return t; }
            if (!node->right) { NodeBST* t = node->left;  delete node; return t; }
            NodeBST* succ          = minVal(node->right);
            node->idPelanggan      = succ->idPelanggan;
            node->namaPelanggan    = succ->namaPelanggan;
            node->alamat           = succ->alamat;
            node->lokasiDefault    = succ->lokasiDefault;
            node->right = deleteRekursif(node->right, succ->idPelanggan);
        }
        return node;
    }

public:
    BSTPelanggan() : root(nullptr) {}
    ~BSTPelanggan() { hapusSemuaRekursif(root); }

    void insert(int id, const char* nama, const char* alm, int lok) {
        root = insertRekursif(root, id, nama, alm, lok);
    }

    NodeBST* search(int id) const { return searchRekursif(root, id); }

    void hapus(int id) { root = deleteRekursif(root, id); }

    void tampilkanInorder() const {
        cout << "\n=== DATA PELANGGAN (BST Inorder) ===" << endl;
        inorderRekursif(root);
    }
};

//  QUICKSORT MANUAL – Paket berdasarkan prioritas
int partisiPaket(Paket** arr, int lo, int hi) {
    int pivot = arr[hi]->prioritas;
    int i     = lo - 1;
    for (int j = lo; j < hi; j++) {
        if (arr[j]->prioritas <= pivot) {
            i++;
            Paket* tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
        }
    }
    Paket* tmp = arr[i+1]; arr[i+1] = arr[hi]; arr[hi] = tmp;
    return i + 1;
}

void quickSortPaket(Paket** arr, int lo, int hi) {
    if (lo >= hi) return;
    int p = partisiPaket(arr, lo, hi);
    quickSortPaket(arr, lo, p - 1);
    quickSortPaket(arr, p + 1, hi);
}

void urutkanDanEnqueue(Paket** arr, int n, CustomQueue& queue) {
    quickSortPaket(arr, 0, n - 1);
    cout << "\n[SORTING] Paket diurutkan berdasarkan prioritas (1=paling urgen):" << endl;
    for (int i = 0; i < n; i++) queue.enqueue(arr[i]);
}

//  DIJKSTRA MANUAL – Rute Terpendek
struct HasilDijkstra {
    double jarak[MAX_LOKASI];
    int    prev[MAX_LOKASI];
};

HasilDijkstra dijkstra(const Graph& g, int sumber) {
    HasilDijkstra hasil;
    bool dikunjungi[MAX_LOKASI];
    for (int i = 0; i < g.jumlahSimpul; i++) {
        hasil.jarak[i]  = INF_JARAK;
        hasil.prev[i]   = -1;
        dikunjungi[i]   = false;
    }
    hasil.jarak[sumber] = 0;

    for (int iter = 0; iter < g.jumlahSimpul; iter++) {
        int u = -1;
        for (int i = 0; i < g.jumlahSimpul; i++) {
            if (!dikunjungi[i] && (u == -1 || hasil.jarak[i] < hasil.jarak[u]))
                u = i;
        }
        if (u == -1 || hasil.jarak[u] == INF_JARAK) break;
        dikunjungi[u] = true;

        NodeEdge* edge = g.adjList[u];
        while (edge) {
            int    v = edge->dest;
            double w = edge->bobot;
            if (!dikunjungi[v] && hasil.jarak[u] + w < hasil.jarak[v]) {
                hasil.jarak[v] = hasil.jarak[u] + w;
                hasil.prev[v]  = u;
            }
            edge = edge->next;
        }
    }
    return hasil;
}

// Rekonstruksi jalur rekursif
void cetakJalur(const HasilDijkstra& h, const Graph& g, int tujuan) {
    if (h.prev[tujuan] == -1) { cout << g.lokasi[tujuan].namaLokasi; return; }
    cetakJalur(h, g, h.prev[tujuan]);
    cout << " -> " << g.lokasi[tujuan].namaLokasi;
}

//  DFS REKURSIF – Eksplorasi Jalur Alternatif
void dfsCariJalur(const Graph& g, int node, int tujuan,
                  bool dikunjungi[], int jalur[], int pjg) {
    dikunjungi[node] = true;
    jalur[pjg]       = node;
    pjg++;

    if (node == tujuan) {
        cout << "  [DFS] Jalur alternatif: ";
        for (int i = 0; i < pjg; i++) {
            cout << g.lokasi[jalur[i]].namaLokasi;
            if (i < pjg - 1) cout << " -> ";
        }
        cout << endl;
    } else {
        NodeEdge* edge = g.adjList[node];
        while (edge) {
            if (!dikunjungi[edge->dest])
                dfsCariJalur(g, edge->dest, tujuan, dikunjungi, jalur, pjg);
            edge = edge->next;
        }
    }
    dikunjungi[node] = false;
}

//  BACKTRACK REKURSIF – Drone Balik Lewat Stack Log
void backtrackDrone(Drone* drone, CustomStack& logRute, const Graph& g) {
    if (logRute.isEmpty()) {
        cout << "  [BACKTRACK] Drone " << drone->namaDrone
             << " telah kembali ke titik awal." << endl;
        drone->status = KEMBALI;
        return;
    }
    NodeStack* titik = logRute.pop();
    cout << "  [BACKTRACK] Drone " << drone->namaDrone
         << " mundur ke: " << titik->namaLokasi
         << " (baterai: " << drone->bateraiPersen << "%)" << endl;
    drone->lokasiSekarang  = titik->lokasiIdx;
    drone->bateraiPersen  -= 3.0;
    delete titik;
    backtrackDrone(drone, logRute, g);  // Rekursif
}

//  UTILITAS TAMPILAN MANUAL
void garisH(char c = '=', int n = 60) {
    for (int i = 0; i < n; i++) cout << c;
    cout << endl;
}

void banner() {
    garisH('=');
    cout << "  GEO-LOGISTIC OPTIMIZER FOR DRONE DELIVERY" << endl;
    cout << "  Simulasi Rute & Distribusi Paket via Drone" << endl;
    cout << "  (Pure C++ - Tanpa Library Bantuan)" << endl;
    garisH('=');
}

//  SIMULASI PENGIRIMAN
void simulasiPengiriman(Drone* drone, Paket* paket,
                        CustomStack& logRute,
                        const Graph& g,
                        HashTable& tracker,
                        bool simulasiGagal = false) {
    garisH('-');
    cout << "[PENGIRIMAN] Drone " << drone->namaDrone
         << " mengambil paket " << paket->resiPaket
         << " untuk " << paket->namaPenerima << endl;

    drone->status        = TERBANG;
    drone->muatanSaatIni = paket;
    paket->status        = DIKIRIM;
    tracker.sisipkan(paket->resiPaket, paket);

    HasilDijkstra h    = dijkstra(g, drone->lokasiSekarang);
    int           tujuan = paket->lokasiTujuan;

    cout << "[NAVIGASI] Rute terpendek ("
         << g.lokasi[drone->lokasiSekarang].namaLokasi
         << " -> " << g.lokasi[tujuan].namaLokasi << "): ";
    cetakJalur(h, g, tujuan);
    cout << "\n[NAVIGASI] Total jarak: " << h.jarak[tujuan] << " km" << endl;

    // Rekonstruksi jalur ke array (manual, balik urutan dari prev)
    int jalur[MAX_LOKASI], pjg = 0;
    {
        int tmp[MAX_LOKASI], t = tujuan, cnt = 0;
        while (t != -1) { tmp[cnt++] = t; t = h.prev[t]; }
        for (int i = cnt - 1; i >= 0; i--) jalur[pjg++] = tmp[i];
    }

    // Push setiap titik ke stack log rute
    cout << "[LOG RUTE] Drone mencatat titik perjalanan:" << endl;
    for (int i = 0; i < pjg; i++) {
        drone->bateraiPersen -= (h.jarak[tujuan] / pjg) * 2.5;
        logRute.push(jalur[i],
                     g.lokasi[jalur[i]].namaLokasi,
                     drone->bateraiPersen);
        cout << "  -> " << g.lokasi[jalur[i]].namaLokasi
             << " (baterai: " << drone->bateraiPersen << "%)" << endl;

        // Cek baterai kritis di tengah jalan
        if (simulasiGagal && drone->bateraiPersen < 20.0 && i < pjg - 1) {
            cout << "\n[DARURAT!] Baterai drone " << drone->namaDrone
                 << " kritis (" << drone->bateraiPersen << "%)! "
                 << "Memulai backtrack..." << endl;
            paket->status = GAGAL;
            tracker.sisipkan(paket->resiPaket, paket);
            backtrackDrone(drone, logRute, g);
            drone->muatanSaatIni = nullptr;
            return;
        }
    }

    // Pengiriman berhasil
    drone->lokasiSekarang = tujuan;
    paket->status         = TERKIRIM;
    tracker.sisipkan(paket->resiPaket, paket);
    drone->status         = IDLE;
    drone->muatanSaatIni  = nullptr;

    cout << "[SUKSES] Paket " << paket->resiPaket
         << " berhasil dikirim ke " << paket->namaPenerima
         << " di " << g.lokasi[tujuan].namaLokasi << "!" << endl;

    // Eksplorasi jalur alternatif via DFS rekursif
    bool kunjungi[MAX_LOKASI];
    int  jalurDFS[MAX_LOKASI];
    for (int i = 0; i < MAX_LOKASI; i++) kunjungi[i] = false;
    cout << "[DFS] Eksplorasi jalur alternatif:" << endl;
    dfsCariJalur(g, 0, tujuan, kunjungi, jalurDFS, 0);
}

int main() {
    banner();

    // FASE 1: INISIALISASI PETA (GRAPH)
    cout << "\n[FASE 1] Inisialisasi Peta Kota...\n";
    Graph peta(8);
    peta.setLokasi(0, 0, 0, "MARKAS PUSAT");
    peta.setLokasi(1, 2, 3, "Perumahan Maju");
    peta.setLokasi(2, 5, 1, "Pasar Sentral");
    peta.setLokasi(3, 7, 5, "Kawasan Industri");
    peta.setLokasi(4, 3, 7, "Komplek Griya");
    peta.setLokasi(5, 6, 9, "Taman Kota");
    peta.setLokasi(6, 1, 6, "Ruko Damai");
    peta.setLokasi(7, 9, 2, "Bandara Kargo");

    peta.tambahEdge(0, 1, 3.5);
    peta.tambahEdge(0, 2, 5.0);
    peta.tambahEdge(0, 6, 4.2);
    peta.tambahEdge(1, 4, 4.8);
    peta.tambahEdge(1, 6, 2.1);
    peta.tambahEdge(2, 3, 3.0);
    peta.tambahEdge(2, 7, 6.5);
    peta.tambahEdge(3, 5, 4.0);
    peta.tambahEdge(3, 7, 2.5);
    peta.tambahEdge(4, 5, 3.3);
    peta.tambahEdge(5, 6, 5.1);
    peta.tambahEdge(6, 4, 3.7);
    peta.tampilkan();

    // FASE 2: INISIALISASI DATA PELANGGAN (BST)
    cout << "\n[FASE 2] Memuat Data Pelanggan (BST)...\n";
    BSTPelanggan bstPelanggan;
    bstPelanggan.insert(1042, "Budi Santoso",  "Jl. Maju No.5",   1);
    bstPelanggan.insert(2087, "Siti Rahayu",   "Jl. Pasar Km.2",  2);
    bstPelanggan.insert(1563, "Ahmad Fauzi",   "Komplek G B3",    4);
    bstPelanggan.insert(3001, "Dewi Lestari",  "Taman Kota 12",   5);
    bstPelanggan.insert(512,  "Rizky Pratama", "Ruko Damai No.8", 6);
    bstPelanggan.insert(2500, "Anita Wijaya",  "Kawasan Ind. D7", 3);
    bstPelanggan.tampilkanInorder();

    cout << "\n[BST] Cari pelanggan ID 1563: ";
    NodeBST* hasil = bstPelanggan.search(1563);
    if (hasil) cout << "Ditemukan! Nama: " << hasil->namaPelanggan << endl;
    else       cout << "Tidak ditemukan." << endl;

    // FASE 3: INISIALISASI FLEET DRONE (Doubly LL)
    cout << "\n[FASE 3] Inisialisasi Fleet Drone...\n";
    PoolDrone pool;
    pool.tambahDrone(new Drone(1, "Garuda-01", 5.0));
    pool.tambahDrone(new Drone(2, "Garuda-02", 3.0));
    pool.tambahDrone(new Drone(3, "Falcon-01", 8.0));
    pool.tambahDrone(new Drone(4, "Falcon-02", 8.0));
    pool.tampilkan();

    // FASE 4: DAFTAR PAKET MASUK + HASH TABLE
    cout << "\n[FASE 4] Mendaftarkan Paket Baru...\n";

    HashTable   tracker;
    CustomQueue antrean;

    const int JUMLAH_PAKET = 5;
    Paket* daftarPaket[JUMLAH_PAKET] = {
        new Paket(101, "PKT-101", "Budi Santoso",  1, 2.5, 3),
        new Paket(102, "PKT-102", "Siti Rahayu",   2, 0.8, 1),  // URGEN
        new Paket(103, "PKT-103", "Ahmad Fauzi",   4, 4.0, 4),
        new Paket(104, "PKT-104", "Dewi Lestari",  5, 1.2, 2),
        new Paket(105, "PKT-105", "Rizky Pratama", 6, 3.0, 5)
    };

    cout << "\n[HASH TABLE] Mendaftarkan paket ke sistem tracking..." << endl;
    for (int i = 0; i < JUMLAH_PAKET; i++) {
        tracker.sisipkan(daftarPaket[i]->resiPaket, daftarPaket[i]);
        cout << "  Resi " << daftarPaket[i]->resiPaket
             << " terdaftar. [ID:" << daftarPaket[i]->idPaket
             << " | Prio:" << daftarPaket[i]->prioritas << "]" << endl;
    }

    // FASE 5: SORTING + ENQUEUE
    cout << "\n[FASE 5] Mengurutkan & Memasukkan ke Antrean...\n";
    urutkanDanEnqueue(daftarPaket, JUMLAH_PAKET, antrean);
    antrean.tampilkan();

    // FASE 6: PROSES PENGIRIMAN
    cout << "\n[FASE 6] Memulai Proses Pengiriman...\n";
    garisH();

    int noSesi = 1;
    while (!antrean.isEmpty()) {
        Drone* droneAktif = pool.ambilDroneIdle();
        if (!droneAktif) {
            cout << "[TUNGGU] Semua drone sedang bertugas." << endl;
            break;
        }

        Paket* paket = antrean.dequeue();
        if (!paket) break;

        CustomStack logRute;

        // Demo darurat: paksa baterai kritis pada pengiriman ke-2
        bool gagalkan = (noSesi == 2);
        if (gagalkan) {
            droneAktif->bateraiPersen = 18.0;
            cout << "\n[SIMULASI] Baterai drone " << droneAktif->namaDrone
                 << " sengaja dikurangi ke " << droneAktif->bateraiPersen
                 << "% untuk demo darurat." << endl;
        }

        simulasiPengiriman(droneAktif, paket, logRute, peta, tracker, gagalkan);
        tracker.tampilkanStatus(paket->resiPaket);

        if (droneAktif->bateraiPersen < 50.0) {
            droneAktif->bateraiPersen = 100.0;
            cout << "[BASE] Drone " << droneAktif->namaDrone
                 << " mengisi baterai kembali ke 100%." << endl;
        }
        droneAktif->status        = IDLE;
        droneAktif->lokasiSekarang = 0;

        noSesi++;
        garisH();
    }

    // LAPORAN AKHIR
    cout << "\n[LAPORAN AKHIR] Status Semua Paket:" << endl;
    garisH('-');
    for (int i = 0; i < JUMLAH_PAKET; i++)
        tracker.tampilkanStatus(daftarPaket[i]->resiPaket);

    pool.tampilkan();
    bstPelanggan.tampilkanInorder();

    garisH();
    cout << "  Simulasi selesai. Terima kasih!" << endl;
    garisH();

    for (int i = 0; i < JUMLAH_PAKET; i++) delete daftarPaket[i];

    return 0;
}