/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║        GEO-LOGISTIC OPTIMIZER FOR DRONE DELIVERY                ║
 * ║        Simulasi Rute & Distribusi Drone Paket                   ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 * Struktur Data yang Digunakan:
 *   - Struct & Pointer   : Paket, Drone, Koordinat
 *   - Graph (Adjacency)  : Peta kota & jalur udara berbobot
 *   - Queue (Manual)     : Antrean paket FIFO
 *   - Stack (Manual)     : Log riwayat rute drone (backtracking)
 *   - Doubly Linked List : Pool drone aktif/tersedia
 *   - Hash Table         : Tracking paket O(1) via Resi
 *   - BST                : Manajemen data pelanggan
 *   - Sorting (QuickSort): Mengurutkan paket berdasarkan prioritas
 *   - Dijkstra           : Pencarian rute terpendek di graph
 *   - DFS Rekursif       : Eksplorasi jalur alternatif
 *   - Fungsi Rekursif    : Traversal BST & backtrack stack drone
 */

#include <iostream>
#include <string>
#include <climits>   // INT_MAX
#include <cstring>   // memset

using namespace std;

// ═══════════════════════════════════════════════════════
//  KONSTANTA & ENUM
// ═══════════════════════════════════════════════════════

const int MAX_LOKASI   = 10;   // Jumlah maksimum node/lokasi di peta
const int INF          = INT_MAX;
const int HT_KAPASITAS = 64;   // Ukuran awal Hash Table (power of 2)

enum StatusPaket {
    MENUNGGU    = 0,  // Paket di antrean
    DIKIRIM     = 1,  // Paket sedang dalam perjalanan
    TERKIRIM    = 2,  // Paket sudah sampai tujuan
    GAGAL       = 3   // Pengiriman gagal (baterai habis, dll.)
};

enum StatusDrone {
    IDLE        = 0,  // Drone menganggur, siap pakai
    TERBANG     = 1,  // Drone sedang mengantarkan paket
    KEMBALI     = 2,  // Drone sedang kembali ke base
    RUSAK       = 3   // Drone tidak dapat digunakan
};

// ═══════════════════════════════════════════════════════
//  STRUCT DASAR
// ═══════════════════════════════════════════════════════

struct Koordinat {
    double x, y;
    string namaLokasi;
};

struct Paket {
    int    idPaket;
    string resiPaket;      // Kode unik untuk Hash Table (contoh: "PKT-001")
    string namaPenerima;
    int    lokasiTujuan;   // Indeks node tujuan di graph
    double bobotKg;
    int    prioritas;      // 1 = sangat urgen, 5 = normal
    StatusPaket status;

    // Pointer ke paket berikutnya (dipakai di Queue & sorted array temp)
    Paket* next;

    Paket(int id, const string& resi, const string& nama,
          int tujuan, double bobot, int prio)
        : idPaket(id), resiPaket(resi), namaPenerima(nama),
          lokasiTujuan(tujuan), bobotKg(bobot), prioritas(prio),
          status(MENUNGGU), next(nullptr) {}
};

struct Drone {
    int       idDrone;
    string    namaDrone;
    double    kapasitasKg;
    double    bateraiPersen;   // 0–100
    int       lokasiSekarang;  // Indeks node saat ini di graph
    StatusDrone status;
    Paket*    muatanSaatIni;   // Pointer ke paket yang dibawa

    // Pointer untuk Doubly Linked List pool drone
    Drone* next;
    Drone* prev;

    Drone(int id, const string& nama, double kapasitas)
        : idDrone(id), namaDrone(nama), kapasitasKg(kapasitas),
          bateraiPersen(100.0), lokasiSekarang(0),
          status(IDLE), muatanSaatIni(nullptr),
          next(nullptr), prev(nullptr) {}
};

// ═══════════════════════════════════════════════════════
//  NODE GRAPH (Adjacency List Berbobot)
// ═══════════════════════════════════════════════════════

struct NodeEdge {
    int    dest;    // Indeks lokasi tujuan
    double bobot;   // Jarak / biaya jalur
    NodeEdge* next;

    NodeEdge(int d, double b) : dest(d), bobot(b), next(nullptr) {}
};

// ═══════════════════════════════════════════════════════
//  GRAPH – PETA KOTA
// ═══════════════════════════════════════════════════════

struct Graph {
    int        jumlahSimpul;
    NodeEdge** adjList;      // Array pointer ke head adjacency list
    Koordinat  lokasi[MAX_LOKASI];

    Graph(int n) : jumlahSimpul(n) {
        adjList = new NodeEdge*[n];
        for (int i = 0; i < n; i++) adjList[i] = nullptr;
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

    // Tambah edge undirected berbobot
    void tambahEdge(int src, int dst, double bobot) {
        // src -> dst
        NodeEdge* e1 = new NodeEdge(dst, bobot);
        e1->next = adjList[src];
        adjList[src] = e1;
        // dst -> src
        NodeEdge* e2 = new NodeEdge(src, bobot);
        e2->next = adjList[dst];
        adjList[dst] = e2;
    }

    void setLokasi(int idx, double x, double y, const string& nama) {
        lokasi[idx] = {x, y, nama};
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

// ═══════════════════════════════════════════════════════
//  QUEUE MANUAL – ANTREAN PAKET (FIFO)
// ═══════════════════════════════════════════════════════

class CustomQueue {
private:
    Paket* head;
    Paket* tail;
    int    ukuran;

public:
    CustomQueue() : head(nullptr), tail(nullptr), ukuran(0) {}

    ~CustomQueue() {
        while (!isEmpty()) dequeue();
    }

    bool isEmpty() const { return head == nullptr; }
    int  getUkuran() const { return ukuran; }

    // Enqueue: tambahkan paket ke belakang antrean
    void enqueue(Paket* paket) {
        if (!paket) return;
        paket->next = nullptr;

        if (isEmpty()) {
            head = tail = paket;
        } else {
            tail->next = paket;
            tail = paket;
        }
        ukuran++;
        cout << "  [QUEUE] Paket " << paket->resiPaket
             << " (" << paket->namaPenerima << ") masuk antrean."
             << " [Prioritas:" << paket->prioritas
             << " | Bobot:" << paket->bobotKg << "kg]" << endl;
    }

    // Dequeue: ambil paket dari depan antrean
    Paket* dequeue() {
        if (isEmpty()) {
            cout << "  [QUEUE] Antrean kosong!" << endl;
            return nullptr;
        }
        Paket* diambil = head;
        head = head->next;
        if (head == nullptr) tail = nullptr;
        diambil->next = nullptr;
        ukuran--;
        return diambil;
    }

    Paket* peek() const { return head; }

    void tampilkan() const {
        if (isEmpty()) {
            cout << "  [QUEUE] Antrean kosong." << endl;
            return;
        }
        cout << "  [QUEUE] Isi antrean (" << ukuran << " paket): " << endl;
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

// ═══════════════════════════════════════════════════════
//  STACK MANUAL – LOG RUTE DRONE (LIFO)
// ═══════════════════════════════════════════════════════

struct NodeStack {
    int       lokasiIdx;
    string    namaLokasi;
    double    bobotTerbang;  // Persentase baterai saat meninggalkan titik ini
    NodeStack* next;
};

class CustomStack {
private:
    NodeStack* top;
    int        ukuran;

public:
    CustomStack() : top(nullptr), ukuran(0) {}

    ~CustomStack() {
        while (!isEmpty()) pop();
    }

    bool isEmpty() const { return top == nullptr; }
    int  getUkuran() const { return ukuran; }

    // Push: catat titik yang baru dilewati drone
    void push(int idx, const string& nama, double bateraiSaatItu) {
        NodeStack* baru = new NodeStack();
        baru->lokasiIdx    = idx;
        baru->namaLokasi   = nama;
        baru->bobotTerbang = bateraiSaatItu;
        baru->next         = top;
        top   = baru;
        ukuran++;
    }

    // Pop: hapus log titik teratas
    NodeStack* pop() {
        if (isEmpty()) return nullptr;
        NodeStack* diambil = top;
        top = top->next;
        ukuran--;
        // Caller bertanggung jawab delete node ini
        return diambil;
    }

    NodeStack* peek() const { return top; }

    void tampilkan() const {
        if (isEmpty()) {
            cout << "    (log kosong)" << endl;
            return;
        }
        NodeStack* cur = top;
        int step = ukuran;
        while (cur) {
            cout << "    Step " << step-- << ": ["
                 << cur->namaLokasi << "] baterai="
                 << cur->bobotTerbang << "%" << endl;
            cur = cur->next;
        }
    }
};

// ═══════════════════════════════════════════════════════
//  DOUBLY LINKED LIST – POOL DRONE
// ═══════════════════════════════════════════════════════

class PoolDrone {
private:
    Drone* head;
    Drone* tail;
    int    jumlah;

public:
    PoolDrone() : head(nullptr), tail(nullptr), jumlah(0) {}

    ~PoolDrone() {
        Drone* cur = head;
        while (cur) {
            Drone* tmp = cur;
            cur = cur->next;
            delete tmp;
        }
    }

    int getJumlah() const { return jumlah; }

    // Tambah drone baru ke belakang pool
    void tambahDrone(Drone* drone) {
        drone->next = nullptr;
        drone->prev = nullptr;
        if (!head) {
            head = tail = drone;
        } else {
            tail->next = drone;
            drone->prev = tail;
            tail = drone;
        }
        jumlah++;
    }

    // Ambil drone IDLE pertama dari pool (tidak dihapus dari list, ubah status)
    Drone* ambilDroneIdle() {
        Drone* cur = head;
        while (cur) {
            if (cur->status == IDLE) {
                return cur;
            }
            cur = cur->next;
        }
        return nullptr;
    }

    // Hapus drone tertentu dari pool berdasarkan pointer
    void hapusDrone(Drone* target) {
        if (!target) return;
        if (target->prev) target->prev->next = target->next;
        else head = target->next;
        if (target->next) target->next->prev = target->prev;
        else tail = target->prev;
        target->next = target->prev = nullptr;
        jumlah--;
    }

    void tampilkan() const {
        cout << "\n=== POOL DRONE (" << jumlah << " unit) ===" << endl;
        Drone* cur = head;
        while (cur) {
            string st = (cur->status == IDLE)    ? "IDLE"    :
                        (cur->status == TERBANG)  ? "TERBANG" :
                        (cur->status == KEMBALI)  ? "KEMBALI" : "RUSAK";
            cout << "  [D" << cur->idDrone << "] " << cur->namaDrone
                 << " | Baterai:" << cur->bateraiPersen << "%"
                 << " | Status:" << st
                 << " | Kapasitas:" << cur->kapasitasKg << "kg" << endl;
            cur = cur->next;
        }
    }
};

// ═══════════════════════════════════════════════════════
//  HASH TABLE – TRACKING PAKET O(1) via RESI
// ═══════════════════════════════════════════════════════

struct NodeHT {
    string  resi;       // Key
    Paket*  paket;      // Value: pointer ke objek paket
    NodeHT* next;

    NodeHT(const string& r, Paket* p) : resi(r), paket(p), next(nullptr) {}
};

class HashTable {
private:
    NodeHT** bucket;
    int      kapasitas;
    int      ukuran;

    // Hash function: polynomial rolling hash
    int hashFungsi(const string& key) const {
        long long hash = 0;
        for (char c : key) {
            hash = (hash * 31 + c) % kapasitas;
        }
        return (int)hash;
    }

    void resize() {
        int kapLama = kapasitas;
        kapasitas  *= 2;
        NodeHT** lama = bucket;

        bucket = new NodeHT*[kapasitas];
        for (int i = 0; i < kapasitas; i++) bucket[i] = nullptr;
        ukuran = 0;

        for (int i = 0; i < kapLama; i++) {
            NodeHT* cur = lama[i];
            while (cur) {
                sisipkan(cur->resi, cur->paket);
                NodeHT* tmp = cur;
                cur = cur->next;
                delete tmp;
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
            while (cur) {
                NodeHT* tmp = cur;
                cur = cur->next;
                delete tmp;
            }
        }
        delete[] bucket;
    }

    // Sisipkan / update paket berdasarkan resi
    void sisipkan(const string& resi, Paket* paket) {
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

    // Cari paket berdasarkan resi — O(1) rata-rata
    Paket* cari(const string& resi) const {
        int idx = hashFungsi(resi);
        NodeHT* cur = bucket[idx];
        while (cur) {
            if (cur->resi == resi) return cur->paket;
            cur = cur->next;
        }
        return nullptr;
    }

    // Hapus entry berdasarkan resi
    void hapus(const string& resi) {
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
            prev = cur;
            cur  = cur->next;
        }
        cout << "  [HT] Resi " << resi << " tidak ditemukan." << endl;
    }

    void tampilkanStatus(const string& resi) const {
        Paket* p = cari(resi);
        if (!p) {
            cout << "  [TRACKING] Resi " << resi << ": tidak ditemukan." << endl;
            return;
        }
        string st = (p->status == MENUNGGU) ? "MENUNGGU"  :
                    (p->status == DIKIRIM)  ? "DIKIRIM"   :
                    (p->status == TERKIRIM) ? "TERKIRIM"  : "GAGAL";
        cout << "  [TRACKING] Resi:" << resi
             << " | Penerima:" << p->namaPenerima
             << " | Status:" << st << endl;
    }
};

// ═══════════════════════════════════════════════════════
//  BST – DATA PELANGGAN
// ═══════════════════════════════════════════════════════

struct NodeBST {
    int     idPelanggan;
    string  namaPelanggan;
    string  alamat;
    int     lokasiDefault;   // Indeks node lokasi default pelanggan
    NodeBST* left;
    NodeBST* right;

    NodeBST(int id, const string& nama, const string& alamatP, int lok)
        : idPelanggan(id), namaPelanggan(nama), alamat(alamatP),
          lokasiDefault(lok), left(nullptr), right(nullptr) {}
};

class BSTPelanggan {
private:
    NodeBST* root;

    // ── Rekursif: insert ──────────────────────────────
    NodeBST* insertRekursif(NodeBST* node, int id,
                             const string& nama,
                             const string& alamat, int lok) {
        if (!node) return new NodeBST(id, nama, alamat, lok);
        if (id < node->idPelanggan)
            node->left  = insertRekursif(node->left,  id, nama, alamat, lok);
        else if (id > node->idPelanggan)
            node->right = insertRekursif(node->right, id, nama, alamat, lok);
        return node;
    }

    // ── Rekursif: search ─────────────────────────────
    NodeBST* searchRekursif(NodeBST* node, int id) const {
        if (!node || node->idPelanggan == id) return node;
        if (id < node->idPelanggan)
            return searchRekursif(node->left,  id);
        return searchRekursif(node->right, id);
    }

    // ── Rekursif: inorder traversal ───────────────────
    void inorderRekursif(NodeBST* node) const {
        if (!node) return;
        inorderRekursif(node->left);
        cout << "    ID:" << node->idPelanggan
             << " | " << node->namaPelanggan
             << " | " << node->alamat
             << " | LokasiDefault:" << node->lokasiDefault << endl;
        inorderRekursif(node->right);
    }

    // ── Rekursif: hapus semua node ────────────────────
    void hapusSemuaRekursif(NodeBST* node) {
        if (!node) return;
        hapusSemuaRekursif(node->left);
        hapusSemuaRekursif(node->right);
        delete node;
    }

    // ── Rekursif: cari nilai minimum ──────────────────
    NodeBST* minVal(NodeBST* node) const {
        while (node->left) node = node->left;
        return node;
    }

    // ── Rekursif: delete node ─────────────────────────
    NodeBST* deleteRekursif(NodeBST* node, int id) {
        if (!node) return nullptr;
        if (id < node->idPelanggan) {
            node->left  = deleteRekursif(node->left,  id);
        } else if (id > node->idPelanggan) {
            node->right = deleteRekursif(node->right, id);
        } else {
            if (!node->left) {
                NodeBST* tmp = node->right;
                delete node;
                return tmp;
            }
            if (!node->right) {
                NodeBST* tmp = node->left;
                delete node;
                return tmp;
            }
            NodeBST* succ = minVal(node->right);
            node->idPelanggan  = succ->idPelanggan;
            node->namaPelanggan = succ->namaPelanggan;
            node->alamat       = succ->alamat;
            node->lokasiDefault = succ->lokasiDefault;
            node->right = deleteRekursif(node->right, succ->idPelanggan);
        }
        return node;
    }

public:
    BSTPelanggan() : root(nullptr) {}

    ~BSTPelanggan() { hapusSemuaRekursif(root); }

    void insert(int id, const string& nama,
                const string& alamat, int lok) {
        root = insertRekursif(root, id, nama, alamat, lok);
    }

    NodeBST* search(int id) const {
        return searchRekursif(root, id);
    }

    void hapus(int id) {
        root = deleteRekursif(root, id);
    }

    void tampilkanInorder() const {
        cout << "\n=== DATA PELANGGAN (BST Inorder) ===" << endl;
        inorderRekursif(root);
    }
};

// ═══════════════════════════════════════════════════════
//  SORTING MANUAL – QUICKSORT PAKET (Berdasarkan Prioritas)
// ═══════════════════════════════════════════════════════

// Untuk kemudahan sorting, kita gunakan array pointer sementara
void tukarPtr(Paket**& a, Paket**& b) {
    Paket* tmp = *a;
    *a = *b;
    *b = tmp;
}

int partisiPaket(Paket** arr, int lo, int hi) {
    int pivot = arr[hi]->prioritas;
    int i = lo - 1;
    for (int j = lo; j < hi; j++) {
        // Urut ascending: prioritas 1 (urgen) tampil paling depan
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

// Urutkan paket dalam array lalu masukkan ke queue
void urutkanDanEnqueue(Paket** arr, int n, CustomQueue& queue) {
    quickSortPaket(arr, 0, n - 1);
    cout << "\n[SORTING] Paket diurutkan berdasarkan prioritas (1=paling urgen):" << endl;
    for (int i = 0; i < n; i++) {
        queue.enqueue(arr[i]);
    }
}

// ═══════════════════════════════════════════════════════
//  DIJKSTRA – RUTE TERPENDEK
// ═══════════════════════════════════════════════════════

struct HasilDijkstra {
    double jarak[MAX_LOKASI];
    int    prev[MAX_LOKASI];   // Untuk rekonstruksi jalur
};

HasilDijkstra dijkstra(const Graph& g, int sumber) {
    HasilDijkstra hasil;
    bool dikunjungi[MAX_LOKASI] = {};

    for (int i = 0; i < g.jumlahSimpul; i++) {
        hasil.jarak[i] = 1e18;
        hasil.prev[i]  = -1;
    }
    hasil.jarak[sumber] = 0;

    for (int iterasi = 0; iterasi < g.jumlahSimpul; iterasi++) {
        // Pilih node belum dikunjungi dengan jarak minimum
        int u = -1;
        for (int i = 0; i < g.jumlahSimpul; i++) {
            if (!dikunjungi[i] &&
                (u == -1 || hasil.jarak[i] < hasil.jarak[u]))
                u = i;
        }
        if (u == -1 || hasil.jarak[u] == 1e18) break;
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

// Rekonstruksi jalur dari sumber ke tujuan (rekursif)
void cetakJalur(const HasilDijkstra& h, const Graph& g, int tujuan) {
    if (h.prev[tujuan] == -1) {
        cout << g.lokasi[tujuan].namaLokasi;
        return;
    }
    cetakJalur(h, g, h.prev[tujuan]);
    cout << " -> " << g.lokasi[tujuan].namaLokasi;
}

// ═══════════════════════════════════════════════════════
//  DFS REKURSIF – EKSPLORASI JALUR ALTERNATIF
// ═══════════════════════════════════════════════════════

void dfsCariJalur(const Graph& g, int node, int tujuan,
                  bool dikunjungi[], int jalur[], int panjang) {
    dikunjungi[node] = true;
    jalur[panjang]   = node;
    panjang++;

    if (node == tujuan) {
        cout << "  [DFS] Jalur alternatif: ";
        for (int i = 0; i < panjang; i++) {
            cout << g.lokasi[jalur[i]].namaLokasi;
            if (i < panjang - 1) cout << " -> ";
        }
        cout << endl;
    } else {
        NodeEdge* edge = g.adjList[node];
        while (edge) {
            if (!dikunjungi[edge->dest])
                dfsCariJalur(g, edge->dest, tujuan,
                             dikunjungi, jalur, panjang);
            edge = edge->next;
        }
    }
    dikunjungi[node] = false;
}

// ═══════════════════════════════════════════════════════
//  BACKTRACK REKURSIF – DRONE KEMBALI LEWAT STACK LOG
// ═══════════════════════════════════════════════════════

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

    drone->lokasiSekarang = titik->lokasiIdx;
    drone->bateraiPersen -= 3.0;  // Konsumsi baterai saat backtrack
    delete titik;

    backtrackDrone(drone, logRute, g);   // Rekursif
}

// ═══════════════════════════════════════════════════════
//  UTILITAS TAMPILAN
// ═══════════════════════════════════════════════════════

void garisH(char c = '=', int n = 60) {
    for (int i = 0; i < n; i++) cout << c;
    cout << endl;
}

void banner() {
    garisH('=');
    cout << "  GEO-LOGISTIC OPTIMIZER FOR DRONE DELIVERY" << endl;
    cout << "  Simulasi Rute & Distribusi Paket via Drone" << endl;
    garisH('=');
}

// ═══════════════════════════════════════════════════════
//  SIMULASI PENGIRIMAN
// ═══════════════════════════════════════════════════════

void simulasiPengiriman(Drone* drone, Paket* paket,
                        CustomStack& logRute,
                        const Graph& g,
                        HashTable& tracker,
                        bool simulasiGagal = false) {
    garisH('-');
    cout << "[PENGIRIMAN] Drone " << drone->namaDrone
         << " mengambil paket " << paket->resiPaket
         << " untuk " << paket->namaPenerima << endl;

    drone->status         = TERBANG;
    drone->muatanSaatIni  = paket;
    paket->status         = DIKIRIM;
    tracker.sisipkan(paket->resiPaket, paket);

    // Cari rute terpendek Dijkstra
    HasilDijkstra h = dijkstra(g, drone->lokasiSekarang);
    int tujuan = paket->lokasiTujuan;

    cout << "[NAVIGASI] Rute terpendek (" 
         << g.lokasi[drone->lokasiSekarang].namaLokasi
         << " -> " << g.lokasi[tujuan].namaLokasi
         << "): ";
    cetakJalur(h, g, tujuan);
    cout << "\n[NAVIGASI] Total jarak: " << h.jarak[tujuan] << " km" << endl;

    // Rekonstruksi jalur sebagai array untuk push ke stack
    int jalur[MAX_LOKASI], pjg = 0;
    {
        int tmp[MAX_LOKASI], t = tujuan, cnt = 0;
        while (t != -1) { tmp[cnt++] = t; t = h.prev[t]; }
        for (int i = cnt - 1; i >= 0; i--) jalur[pjg++] = tmp[i];
    }

    // Push setiap titik ke stack log rute drone
    cout << "[LOG RUTE] Drone mencatat titik perjalanan:" << endl;
    for (int i = 0; i < pjg; i++) {
        drone->bateraiPersen -= (h.jarak[tujuan] / pjg) * 2.5;
        logRute.push(jalur[i],
                     g.lokasi[jalur[i]].namaLokasi,
                     drone->bateraiPersen);
        cout << "  -> " << g.lokasi[jalur[i]].namaLokasi
             << " (baterai: " << drone->bateraiPersen << "%)" << endl;

        // Cek baterai kritis (< 20%) di tengah perjalanan
        if (simulasiGagal && drone->bateraiPersen < 20.0 && i < pjg - 1) {
            cout << "\n[DARURAT!] Baterai drone "
                 << drone->namaDrone << " kritis ("
                 << drone->bateraiPersen << "%)! "
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

    // Tampilkan jalur alternatif via DFS
    bool kunjungi[MAX_LOKASI] = {};
    int  jalurDFS[MAX_LOKASI];
    cout << "[DFS] Eksplorasi jalur alternatif:" << endl;
    dfsCariJalur(g, 0, tujuan, kunjungi, jalurDFS, 0);
}

// ═══════════════════════════════════════════════════════
//  MAIN – ALUR PROGRAM UTAMA
// ═══════════════════════════════════════════════════════

int main() {
    banner();

    // ────────────────────────────────────────────────
    // 1. INISIALISASI PETA (GRAPH)
    // ────────────────────────────────────────────────
    cout << "\n[FASE 1] Inisialisasi Peta Kota...\n";
    Graph peta(8);
    peta.setLokasi(0, 0,  0,  "MARKAS PUSAT");
    peta.setLokasi(1, 2,  3,  "Perumahan Maju");
    peta.setLokasi(2, 5,  1,  "Pasar Sentral");
    peta.setLokasi(3, 7,  5,  "Kawasan Industri");
    peta.setLokasi(4, 3,  7,  "Komplek Griya");
    peta.setLokasi(5, 6,  9,  "Taman Kota");
    peta.setLokasi(6, 1,  6,  "Ruko Damai");
    peta.setLokasi(7, 9,  2,  "Bandara Kargo");

    // Edge berbobot (jarak dalam km)
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

    // ────────────────────────────────────────────────
    // 2. INISIALISASI DATA PELANGGAN (BST)
    // ────────────────────────────────────────────────
    cout << "\n[FASE 2] Memuat Data Pelanggan (BST)...\n";
    BSTPelanggan bstPelanggan;
    bstPelanggan.insert(1042, "Budi Santoso",   "Jl. Maju No.5",  1);
    bstPelanggan.insert(2087, "Siti Rahayu",    "Jl. Pasar Km.2", 2);
    bstPelanggan.insert(1563, "Ahmad Fauzi",    "Komplek G B3",   4);
    bstPelanggan.insert(3001, "Dewi Lestari",   "Taman Kota 12",  5);
    bstPelanggan.insert(0512, "Rizky Pratama",  "Ruko Damai No.8",6);
    bstPelanggan.insert(2500, "Anita Wijaya",   "Kawasan Ind. D7",3);
    bstPelanggan.tampilkanInorder();

    // Contoh pencarian pelanggan (rekursif BST)
    cout << "\n[BST] Cari pelanggan ID 1563: ";
    NodeBST* hasil = bstPelanggan.search(1563);
    if (hasil) cout << "Ditemukan! Nama: " << hasil->namaPelanggan << endl;
    else       cout << "Tidak ditemukan." << endl;

    // ────────────────────────────────────────────────
    // 3. INISIALISASI FLEET DRONE (Doubly LL)
    // ────────────────────────────────────────────────
    cout << "\n[FASE 3] Inisialisasi Fleet Drone...\n";
    PoolDrone pool;
    pool.tambahDrone(new Drone(1, "Garuda-01", 5.0));
    pool.tambahDrone(new Drone(2, "Garuda-02", 3.0));
    pool.tambahDrone(new Drone(3, "Falcon-01", 8.0));
    pool.tambahDrone(new Drone(4, "Falcon-02", 8.0));
    pool.tampilkan();

    // ────────────────────────────────────────────────
    // 4. DAFTAR PAKET MASUK, HASH & SORTING + QUEUE
    // ────────────────────────────────────────────────
    cout << "\n[FASE 4] Mendaftarkan Paket Baru...\n";

    HashTable  tracker;
    CustomQueue antrean;

    // Buat paket (heap-allocated agar pointer valid sepanjang program)
    const int JUMLAH_PAKET = 5;
    Paket* daftarPaket[JUMLAH_PAKET] = {
        new Paket(101, "PKT-101", "Budi Santoso",  1, 2.5, 3),
        new Paket(102, "PKT-102", "Siti Rahayu",   2, 0.8, 1),   // URGEN
        new Paket(103, "PKT-103", "Ahmad Fauzi",   4, 4.0, 4),
        new Paket(104, "PKT-104", "Dewi Lestari",  5, 1.2, 2),
        new Paket(105, "PKT-105", "Rizky Pratama", 6, 3.0, 5)
    };

    // Daftarkan ke Hash Table
    cout << "\n[HASH TABLE] Mendaftarkan paket ke sistem tracking..." << endl;
    for (int i = 0; i < JUMLAH_PAKET; i++) {
        tracker.sisipkan(daftarPaket[i]->resiPaket, daftarPaket[i]);
        cout << "  Resi " << daftarPaket[i]->resiPaket
             << " terdaftar. [ID:" << daftarPaket[i]->idPaket
             << " | Prio:" << daftarPaket[i]->prioritas << "]" << endl;
    }

    // Sorting berdasarkan prioritas + masukkan ke Queue
    cout << "\n[FASE 5] Mengurutkan & Memasukkan ke Antrean...\n";
    urutkanDanEnqueue(daftarPaket, JUMLAH_PAKET, antrean);
    antrean.tampilkan();

    // ────────────────────────────────────────────────
    // 5. PROSES PENGIRIMAN (Drone ambil dari Queue)
    // ────────────────────────────────────────────────
    cout << "\n[FASE 6] Memulai Proses Pengiriman...\n";
    garisH();

    int noSesi = 1;
    while (!antrean.isEmpty()) {
        Drone* droneAktif = pool.ambilDroneIdle();
        if (!droneAktif) {
            cout << "[TUNGGU] Semua drone sedang bertugas. Menunggu..." << endl;
            break;
        }

        Paket* paket = antrean.dequeue();
        if (!paket) break;

        CustomStack logRute;   // Stack log khusus per sesi pengiriman

        // Simulasi baterai habis pada pengiriman ke-2
        bool gagalkan = (noSesi == 2);
        if (gagalkan) {
            droneAktif->bateraiPersen = 18.0;  // Set baterai kritis
            cout << "\n[SIMULASI] Baterai drone " << droneAktif->namaDrone
                 << " sengaja dikurangi ke " << droneAktif->bateraiPersen
                 << "% untuk demo darurat." << endl;
        }

        simulasiPengiriman(droneAktif, paket, logRute, peta, tracker, gagalkan);

        // Tampilkan status tracking setelah pengiriman
        tracker.tampilkanStatus(paket->resiPaket);

        // Reset baterai drone setelah kembali (recharge cepat)
        if (droneAktif->bateraiPersen < 50.0) {
            droneAktif->bateraiPersen = 100.0;
            cout << "[BASE] Drone " << droneAktif->namaDrone
                 << " mengisi baterai kembali ke 100%." << endl;
        }
        droneAktif->status       = IDLE;
        droneAktif->lokasiSekarang = 0;  // Kembali ke markas

        noSesi++;
        garisH();
    }

    // ────────────────────────────────────────────────
    // 6. LAPORAN AKHIR
    // ────────────────────────────────────────────────
    cout << "\n[LAPORAN AKHIR] Status Semua Paket:" << endl;
    garisH('-');
    for (int i = 0; i < JUMLAH_PAKET; i++) {
        tracker.tampilkanStatus(daftarPaket[i]->resiPaket);
    }

    pool.tampilkan();
    bstPelanggan.tampilkanInorder();

    garisH();
    cout << "  Simulasi selesai. Terima kasih!" << endl;
    garisH();

    // Bersihkan memori paket
    for (int i = 0; i < JUMLAH_PAKET; i++) delete daftarPaket[i];

    return 0;
}