// combined_game_v3.cpp
// Büyükleştirilmiş sürüm: Kedi simülasyonu + Gelişmiş Arena (PvP/PvE) + AI durum makinesi + Envanter + Save/Load
// Derleme: g++ combined_game_v3.cpp -std=c++17 -O2 -o game
// Yaklaşık hedef uzunluk: ~500 satır (açıklamalar dahil)

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <chrono>
#include <thread>
#include <fstream>
#include <algorithm>
#include <map>
#include <iomanip>

#ifdef _WIN32
  #include <windows.h>
#else
  #include <unistd.h>
#endif

using namespace std;

// ----------------- AYARLAR -----------------
static const bool USE_EMOJI = true;          // eğer terminal emoji göstermiyorsa false yap
static const int DEFAULT_BILET_GAIN = 3;     // günlük bilet geliri
static const string SAVE_FILE = "game_save.txt";

// ----------------- YARDIMCI FONK. -----------------
void msleep(int ms) {
#ifdef _WIN32
    Sleep(ms);
#else
    this_thread::sleep_for(chrono::milliseconds(ms));
#endif
}

void clearScreen(){
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

int rndInt(int a, int b){
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<> dist(a,b);
    return dist(gen);
}

void ensureUtf8Console() {
#ifdef _WIN32
    // attempt to enable UTF-8 in windows console - may not work on all terminals
    SetConsoleOutputCP(CP_UTF8);
#endif
}

string padRight(const string &s, int width) {
    if ((int)s.size() >= width) return s;
    return s + string(width - s.size(), ' ');
}

// ----------------- ITEM / ENVENTAR -----------------
enum class ItemType { FOOD, POTION, TICKET, EQUIP, NONE };

struct Item {
    string name;
    ItemType type;
    int value; // food = hunger restore, potion = hp restore, ticket = ticket count, equip = attack bonus etc.
    string desc;
    Item() : name(""), type(ItemType::NONE), value(0), desc("") {}
    Item(string n, ItemType t, int v, string d) : name(n), type(t), value(v), desc(d) {}
};

struct Inventory {
    vector<Item> items;
    void add(const Item &it) { items.push_back(it); }
    bool removeOneByName(const string &name) {
        for (size_t i=0;i<items.size();++i){
            if (items[i].name == name) { items.erase(items.begin()+i); return true; }
        }
        return false;
    }
    int countByType(ItemType t) const {
        int c=0;
        for (auto &it: items) if (it.type==t) c++;
        return c;
    }
    void list() const {
        if (items.empty()) { cout << "Envanter boş.\n"; return; }
        cout << "Envanter:\n";
        for (size_t i=0;i<items.size();++i) {
            cout << i+1 << ") " << padRight(items[i].name, 18) << " - " << items[i].desc << " (Val:" << items[i].value << ")\n";
        }
    }
};

// ----------------- KEDİ SİMÜLASYONU -----------------
struct CatState {
    int hp = 100;
    int hunger = 100;
    int happiness = 100;
    int mama = 0;
    int somon = 0, sut = 0, havuc = 0;
    int ticket = 10;
    int day = 0;
    int xp = 0;
    int level = 1;
    Inventory inv;
};

// leveling
int xpToNext(int lvl) {
    return 50 + lvl * 40;
}

void giveXP(CatState &s, int xp) {
    s.xp += xp;
    while (s.xp >= xpToNext(s.level)) {
        s.xp -= xpToNext(s.level);
        s.level++;
        cout << "Tebrikler! Seviye atladınız: Level " << s.level << " 🎉\n";
        s.hp = min(100, s.hp+20);
        msleep(600);
    }
}

// shop
void shopRandom(CatState &s) {
    if (s.ticket < 5) { cout << "Mağaza için en az 5 bilet gerekir.\n"; return; }
    cout << "Mağaza: rastgele stoklar geliyor...\n"; msleep(400);
    int som = rndInt(0,2), st = rndInt(0,2), hav = rndInt(0,2);
    cout << "Stok: Somon " << som << " Sut " << st << " Havuc " << hav << "\n";
    s.somon += som; s.sut += st; s.havuc += hav;
    s.ticket -= 5;
    // chance to get a potion or item
    if (rndInt(1,100) <= 30) {
        Item pot("İksir", ItemType::POTION, 20, "HP +20 restore eden iksir.");
        s.inv.add(pot);
        cout << "Mağazada küçük bir iksir bulundu! Envantere eklendi.\n";
    }
}

// make food
void makeFood(CatState &s) {
    if (s.ticket < 3) { cout << "Mama yapmak için en az 3 bilet gerekiyor.\n"; return; }
    if (s.somon>0 && s.sut>0 && s.havuc>0) {
        s.mama++;
        s.somon--; s.sut--; s.havuc--;
        s.ticket -= 3;
        cout << "Mama üretildi! Toplam mama: " << s.mama << "\n";
    } else cout << "Malzemeler yetersiz.\n";
}

// feed
void feedCat(CatState &s) {
    if (s.ticket < 2) { cout << "Beslemek için 2 bilet gerekir.\n"; return; }
    if (s.mama <= 0) { cout << "Mama yok! Mağazaya git veya yap.\n"; return; }
    int inc = rndInt(12, 30);
    int old = s.hunger;
    s.hunger = min(100, s.hunger + inc);
    s.mama--;
    s.ticket -= 2;
    cout << "Kedi beslendi (" << old << " -> " << s.hunger << ")\n";
    giveXP(s, 10);
}

// pet
void petCat(CatState &s) {
    if (s.ticket < 1) { cout << "Sevmek 1 bilet ister.\n"; return; }
    s.happiness = min(100, s.happiness + 12);
    s.ticket -= 1;
    cout << "Kedi sevildi. Mutluluk: " << s.happiness << "\n";
    giveXP(s, 5);
}

// day end - now no cost
void dayEnd(CatState &s) {
    s.day++;
    int dec = rndInt(4, 12);
    s.hunger = max(0, s.hunger - dec);
    s.happiness = max(0, s.happiness - 5);
    if (s.hunger == 0 || s.happiness == 0) {
        s.hp = max(0, s.hp - 10);
        cout << "Kedi kötü durumda, can -10 -> " << s.hp << "\n";
    } else {
        if (s.hunger > 80) s.hp = min(100, s.hp + 8);
        if (s.happiness > 80) s.hp = min(100, s.hp + 5);
    }
    s.ticket += DEFAULT_BILET_GAIN;
    cout << "Gün bitti. Gün: " << s.day << " Bilet + " << DEFAULT_BILET_GAIN << "\n";
    giveXP(s, 8);
}

// save/load
void saveState(const CatState &s) {
    ofstream fout(SAVE_FILE);
    if (!fout) { cout << "Kaydetme hatası!\n"; return; }
    fout << s.hp << ' ' << s.hunger << ' ' << s.happiness << ' ' << s.mama << ' '
         << s.somon << ' ' << s.sut << ' ' << s.havuc << ' ' << s.ticket << ' '
         << s.day << ' ' << s.xp << ' ' << s.level << '\n';
    // items count and names
    fout << s.inv.items.size() << '\n';
    for (auto &it: s.inv.items) {
        fout << it.name << '|' << (int)it.type << '|' << it.value << '|' << it.desc << '\n';
    }
    fout.close();
    cout << "Oyun kaydedildi -> " << SAVE_FILE << "\n";
}

bool loadState(CatState &s) {
    ifstream fin(SAVE_FILE);
    if (!fin) { return false; }
    fin >> s.hp >> s.hunger >> s.happiness >> s.mama
        >> s.somon >> s.sut >> s.havuc >> s.ticket
        >> s.day >> s.xp >> s.level;
    size_t n; fin >> n; string line; getline(fin,line); // rest of line
    s.inv.items.clear();
    for (size_t i=0;i<n;i++){
        if (!getline(fin,line)) break;
        // parse name|type|value|desc
        size_t p1 = line.find('|');
        size_t p2 = line.find('|', p1+1);
        size_t p3 = line.find('|', p2+1);
        if (p1==string::npos||p2==string::npos||p3==string::npos) continue;
        string name = line.substr(0,p1);
        ItemType t = (ItemType)stoi(line.substr(p1+1,p2-p1-1));
        int val = stoi(line.substr(p2+1,p3-p2-1));
        string desc = line.substr(p3+1);
        s.inv.add(Item(name,t,val,desc));
    }
    cout << "Kayıt yüklendi.\n";
    return true;
}

// show state
void showCat(const CatState &s) {
    clearScreen();
    cout << "=== Kedi Simülasyonu ===\n";
    cout << "Seviye: " << s.level << "  XP: " << s.xp << "/" << xpToNext(s.level) << "\n";
    cout << "Can: " << s.hp << "  Aclik: " << s.hunger << "  Mutluluk: " << s.happiness << "\n";
    cout << "Mama: " << s.mama << "  Somon: " << s.somon << "  Sut: " << s.sut << "  Havuc: " << s.havuc << "\n";
    cout << "Bilet: " << s.ticket << "  Gun: " << s.day << "\n";
    cout << "-------------------------\n";
}

// ----------------- ARENA & SAVAŞ -----------------
enum class AIState { IDLE, AGGRESSIVE, DEFENSIVE, FLEE };

struct Player {
    string name;
    int x, y;
    int hp;
    int maxHp;
    int atk;
    int def;
    int stamina;
    bool stunned;
    int stunTurns;
    Player(string n="Player", int X=0,int Y=0) : name(n), x(X), y(Y), hp(100), maxHp(100), atk(15), def(3), stamina(100), stunned(false), stunTurns(0) {}
};

struct ArenaMap {
    int H, W;
    vector<vector<char>> grid; // '.' empty, '#' wall, 'T' trap, 'P' power-up
    ArenaMap(int h=6,int w=7): H(h), W(w), grid(h, vector<char>(w,'.')) {}
    void placeWallsRandomly(int count){
        for (int i=0;i<count;i++){
            int rx=rndInt(0,W-1), ry=rndInt(0,H-1);
            if (grid[ry][rx]=='.') grid[ry][rx]='#';
        }
    }
    void placeTrapsRandomly(int count){
        for (int i=0;i<count;i++){
            int rx=rndInt(0,W-1), ry=rndInt(0,H-1);
            if (grid[ry][rx]=='.') grid[ry][rx]='T';
        }
    }
    void placePowersRandomly(int count){
        for (int i=0;i<count;i++){
            int rx=rndInt(0,W-1), ry=rndInt(0,H-1);
            if (grid[ry][rx]=='.') grid[ry][rx]='P';
        }
    }
};

struct ArenaState {
    ArenaMap map;
    Player p1, p2;
    bool useEmoji;
    int turnCount;
    ArenaState(int H=6,int W=7) : map(H,W), p1("P1",0,0), p2("P2", W-1, H-1), useEmoji(USE_EMOJI), turnCount(0) {}
};

// display
void drawArena(const ArenaState &a) {
    clearScreen();
    string emp = a.useEmoji ? "⬜" : ".";
    string p1s = a.useEmoji ? "🧍" : "1";
    string p2s = a.useEmoji ? "🤖" : "2";
    string wall = a.useEmoji ? "🧱" : "#";
    string trap = a.useEmoji ? "💥" : "T";
    string powr = a.useEmoji ? "✨" : "P";
    cout << "Arena Turn: " << a.turnCount << "  " << a.p1.name << " HP:" << a.p1.hp << "  " << a.p2.name << " HP:" << a.p2.hp << "\n";
    for (int i=0;i<a.map.H;i++){
        for (int j=0;j<a.map.W;j++){
            if (a.p1.x==j && a.p1.y==i) { cout << p1s << ' '; }
            else if (a.p2.x==j && a.p2.y==i) { cout << p2s << ' '; }
            else {
                char c = a.map.grid[i][j];
                if (c=='.') cout << emp << ' ';
                else if (c=='#') cout << wall << ' ';
                else if (c=='T') cout << trap << ' ';
                else if (c=='P') cout << powr << ' ';
                else cout << emp << ' ';
            }
        }
        cout << '\n';
    }
}

// helpers
bool adjacent(const Player &a, const Player &b) {
    return abs(a.x - b.x) + abs(a.y - b.y) == 1;
}

void movePlayer(Player &p, char dir, const ArenaMap &m, const Player &other) {
    int nx = p.x, ny = p.y;
    if (dir=='U') ny--;
    if (dir=='D') ny++;
    if (dir=='L') nx--;
    if (dir=='R') nx++;
    if (nx<0 || nx>=m.W || ny<0 || ny>=m.H) return;
    if (m.grid[ny][nx]=='#') return;
    if (other.x==nx && other.y==ny) return;
    p.x = nx; p.y = ny;
}

void applyTileEffect(Player &pl, const ArenaMap &m) {
    char t = m.grid[pl.y][pl.x];
    if (t=='T') {
        int dmg = rndInt(5,15);
        pl.hp = max(0, pl.hp - dmg);
        cout << pl.name << " tuzağa bastı! -" << dmg << " HP\n";
    } else if (t=='P') {
        int buff = rndInt(8,18);
        pl.hp = min(pl.maxHp, pl.hp + buff);
        cout << pl.name << " güç kutusu buldu! HP +" << buff << "\n";
    }
}

// combat
int calcDamage(const Player &attacker, const Player &defender, bool critical) {
    int base = attacker.atk - defender.def;
    if (base < 1) base = 1;
    int dmg = base + rndInt(0,6);
    if (critical) dmg = int(dmg * 1.8);
    return dmg;
}

bool tryCritical() {
    return rndInt(1,100) <= 12; // 12% crit
}

// AI decision: small state machine
char aiDecide(ArenaState &as, AIState &state) {
    Player &ai = as.p2;
    Player &pl = as.p1;
    int dist = abs(ai.x-pl.x) + abs(ai.y-pl.y);
    // state transitions
    if (ai.hp < 30 && state != AIState::FLEE) {
        if (rndInt(1,100) < 70) state = AIState::FLEE;
    } else if (ai.hp >= 60 && dist <= 4) {
        state = AIState::AGGRESSIVE;
    } else if (ai.hp >= 30 && dist <= 6) {
        state = AIState::DEFENSIVE;
    } else state = AIState::IDLE;

    if (adjacent(ai,pl)) {
        // if adjacent decide to attack or special
        if (state == AIState::AGGRESSIVE) return 'F';
        if (state == AIState::FLEE) return 'R'; // try to move away (simple)
        if (state == AIState::DEFENSIVE) {
            if (rndInt(1,100) <= 60) return 'F';
            else return 'B'; // defend (block)
        }
        return 'F';
    }

    // movement logic: approach or evade
    vector<char> moves = {'U','D','L','R'};
    char best='U';
    int bestScore = (state==AIState::FLEE) ? -1000 : 1000;
    for (char m : moves) {
        int nx = ai.x, ny = ai.y;
        if (m=='U') ny--;
        if (m=='D') ny++;
        if (m=='L') nx--;
        if (m=='R') nx++;
        if (nx<0 || nx>=as.map.W || ny<0 || ny>=as.map.H) continue;
        if (as.map.grid[ny][nx]=='#') continue;
        if (nx==pl.x && ny==pl.y) continue;
        int nd = abs(nx - pl.x) + abs(ny - pl.y);
        int score = nd;
        if (state==AIState::FLEE) {
            if (nd > bestScore) { bestScore = nd; best = m; }
        } else {
            if (nd < bestScore) { bestScore = nd; best = m; }
        }
    }
    // small randomness
    if (rndInt(1,100) <= 12) return moves[rndInt(0,3)];
    return best;
}

// run arena PvE
void runArenaPvE(int H=6,int W=7) {
    ArenaState as(H,W);
    // place obstacles and special tiles
    as.map.placeWallsRandomly((H*W)/10);
    as.map.placeTrapsRandomly((H*W)/15);
    as.map.placePowersRandomly((H*W)/20);
    as.p1 = Player("Oyuncu", 0, 0);
    as.p2 = Player("AI", W-1, H-1);
    as.p2.atk = 14; as.p2.def = 3;
    AIState aiState = AIState::IDLE;
    bool running=true;
    while (running) {
        as.turnCount++;
        drawArena(as);
        if (as.p1.hp <= 0 || as.p2.hp <= 0) break;
        cout << "Hareket: (U/D/L/R)  Saldırı: F  Blok: B  E: çıkış\n";
        char c; cin >> c; c = toupper(c);
        if (c=='E') break;
        if (as.p1.stunned) {
            cout << as.p1.name << " sersemledi, bir tur bekliyor!\n";
            as.p1.stunned = false;
            as.p1.stunTurns = 0;
        } else {
            if (c=='F') {
                if (adjacent(as.p1, as.p2)) {
                    bool crit = tryCritical();
                    int dmg = calcDamage(as.p1, as.p2, crit);
                    as.p2.hp = max(0, as.p2.hp - dmg);
                    cout << as.p1.name << " saldırdı! Hasar: " << dmg << (crit? " (Crit!)":"") << "\n";
                } else cout << "Yan yana değilsiniz.\n";
            } else if (c=='B') {
                cout << as.p1.name << " blok pozisyonu aldı (bir sonraki gelen hasarı düşürebilir).\n";
                as.p1.def += 4;
            } else {
                movePlayer(as.p1, c, as.map, as.p2);
                applyTileEffect(as.p1, as.map);
            }
        }
        msleep(250);

        // AI turn
        if (as.p2.hp <= 0 || as.p1.hp <= 0) break;
        char aiAct = aiDecide(as, aiState);
        if (aiAct == 'F') {
            bool crit = tryCritical();
            int dmg = calcDamage(as.p2, as.p1, crit);
            as.p1.hp = max(0, as.p1.hp - dmg);
            cout << as.p2.name << " saldırdı! Hasar: " << dmg << (crit? " (Crit!)":"") << "\n";
        } else if (aiAct == 'B') {
            cout << as.p2.name << " blok alıyor.\n";
            as.p2.def += 3;
        } else {
            movePlayer(as.p2, aiAct, as.map, as.p1);
            applyTileEffect(as.p2, as.map);
            cout << "AI hareket etti.\n";
        }
        // reset temporary defense buffs
        as.p1.def = max(1, as.p1.def - 4);
        as.p2.def = max(1, as.p2.def - 3);
        msleep(400);
    }

    drawArena(as);
    if (as.p1.hp <= 0) cout << "Kaybettin... AI kazandı.\n";
    else if (as.p2.hp <= 0) cout << "Tebrikler! AI yenildi.\n";
    else cout << "Arena sonlandı.\n";
    cout << "Devam etmek için Enter'a bas...";
    cin.ignore(); cin.get();
}

// run PvP
void runArenaPvP(int H=6,int W=7) {
    ArenaState as(H,W);
    as.map.placeWallsRandomly((H*W)/12);
    as.map.placeTrapsRandomly((H*W)/18);
    as.p1 = Player("P1", 0, 0);
    as.p2 = Player("P2", W-1, H-1);
    bool running = true;
    while (running) {
        as.turnCount++;
        drawArena(as);
        // player1
        cout << "P1 hareket (U/D/L/R/F/E): ";
        char c; cin >> c; c = toupper(c);
        if (c=='E') break;
        if (c=='F') {
            if (adjacent(as.p1,as.p2)) {
                bool crit = tryCritical();
                int dmg = calcDamage(as.p1, as.p2, crit);
                as.p2.hp = max(0, as.p2.hp - dmg);
                cout << "P1 saldırdı! " << dmg << (crit? " (Crit)":"") << "\n";
            } else cout << "Yan yana değilsiniz.\n";
        } else movePlayer(as.p1, c, as.map, as.p2);
        msleep(150);
        drawArena(as);
        // player2
        cout << "P2 hareket (U/D/L/R/F/E): ";
        char d; cin >> d; d = toupper(d);
        if (d=='E') break;
        if (d=='F') {
            if (adjacent(as.p1,as.p2)) {
                bool crit = tryCritical();
                int dmg = calcDamage(as.p2, as.p1, crit);
                as.p1.hp = max(0, as.p1.hp - dmg);
                cout << "P2 saldırdı! " << dmg << (crit? " (Crit)":"") << "\n";
            } else cout << "Yan yana değilsiniz.\n";
        } else movePlayer(as.p2, d, as.map, as.p1);
        msleep(150);
    }
    drawArena(as);
    cout << "PvP sonlandı. Enter ile dön...";
    cin.ignore(); cin.get();
}

// ---------- ANA MENÜLER VE OYUN DÖNGÜSÜ ----------
void playArenaMode() {
    while (true) {
        clearScreen();
        cout << "=== Arena Modu ===\n1) Oyuncu vs AI\n2) Oyuncu vs Oyuncu\n3) Geri\nSeçim: ";
        int s; cin >> s;
        if (s==1) { runArenaPvE(6,7); break; }
        else if (s==2) { runArenaPvP(6,7); break; }
        else if (s==3) break;
        else cout << "Geçersiz.\n";
        msleep(300);
    }
}

// Cat sim main loop
void catSimMain() {
    ensureUtf8Console();
    CatState s;
    // attempt to load
    if (loadState(s)) {
        cout << "Kayıt yüklendi. Devam ediliyor...\n";
        msleep(800);
    }
    bool running = true;
    while (running) {
        showCat(s);
        cout << "1) Kedi sev (1 bilet)\n2) Mağaza (5 bilet)\n3) Mama yap (3 bilet)\n4) Kedi besle (2 bilet)\n5) Gün bitir (0 bilet)\n";
        cout << "6) Arena (2 bilet)\n7) Envanteri göster\n8) Kaydet\n9) Çıkış\nSeçim: ";
        int ch; cin >> ch;
        clearScreen();
        switch (ch) {
            case 1: petCat(s); break;
            case 2: shopRandom(s); break;
            case 3: makeFood(s); break;
            case 4: feedCat(s); break;
            case 5: dayEnd(s); break;
            case 6:
                if (s.ticket < 2) cout << "Yeterli bilet yok.\n";
                else {
                    s.ticket -= 2;
                    cout << "Arenaya gidiliyor...\n";
                    msleep(400);
                    playArenaMode();
                }
                break;
            case 7: s.inv.list(); break;
            case 8: saveState(s); break;
            case 9:
                cout << "Simülasyondan çıkılıyor. Kaydetmek ister misin? (y/n): ";
                char y; cin >> y;
                if (y=='y' || y=='Y') saveState(s);
                running = false;
                break;
            default: cout << "Geçersiz seçim.\n";
        }
        if (s.hp <= 0) {
            cout << "Kediniz öldü. Yeni oyun başlatmak ister misiniz? (y/n): ";
            char a; cin >> a;
            if (a=='y' || a=='Y') {
                s = CatState();
                cout << "Yeni oyun başlatıldı.\n";
            } else {
                running = false;
            }
        }
        msleep(600);
    }
}

// ---------- ANA PROGRAM ----------
int main(){
    ensureUtf8Console();
    clearScreen();
    cout << "=== Büyükleştirilmiş Kedi ve Arena Oyunu v3 ===\n";
    // small intro
    cout << "Not: Terminal emoji destekliyorsa emojiler gösterilir.\n";
    while (true) {
        cout << "\nAna Menü:\n1) Kedi Simülasyonu\n2) Arena (direkt)\n3) Kaydetilmiş yükle\n4) Çıkış\nSeçim: ";
        int c; cin >> c;
        clearScreen();
        if (c==1) catSimMain();
        else if (c==2) playArenaMode();
        else if (c==3) {
            CatState s;
            if (loadState(s)) {
                cout << "Kayıt yüklendi, kedi durumu:\n";
                showCat(s);
            } else cout << "Kayıt bulunamadı.\n";
            cout << "Devam için Enter...\n";
            cin.ignore(); cin.get();
        }
        else if (c==4) { cout << "Çıkış yapıldı.\n"; break; }
        else cout << "Geçersiz seçim.\n";
    }
    return 0;
}
