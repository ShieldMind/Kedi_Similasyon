#include <iostream>
#include <random>
#include <unistd.h>
using namespace std;

// Matris Degiskenleri
int x = 0;
int yeni_x = 0;
int y = 0;
int yeni_y = 0;

int x2 = 4;
int yeni_x2 = 4;
int y2 = 4;
int yeni_y2 = 4;

int harita[6][7] = {
    {1,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0},
    {0,0,0,0,2,0,0},
    {0,0,0,0,0,0,0}
};

int can1 = 100;
int can2 = 100;
char yon;
char yon2;

int random (int a) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(10, 25);
    return dist(gen);
}

// Fight fonksiyonu
void fight2() {
    cout << "SAVAS BASLADI!" << endl;
    int hasar1 = random(hasar1);

    if (can1 > 0) {
        can1 = can1 - hasar1;
        cout << "  Karakter 1 Can: " << can2 << endl;
    }

    if (can1 <= 0) {
        cout << "Karakter 1 yenildi!" << endl;
    }
}

void fight() {
    cout << "SAVAS BASLADI!" << endl;
    int hasar2 = random(hasar2);

    if (can2 > 0) {
        can2 = can2 - hasar2;
        cout << "  Karakter 2 Can: " << can2 << endl;
    }

    if (can2 <= 0) {
        cout << "Karakter 2 yenildi!" << endl;
    }
}

bool yanYana() {
    if (x == x2 && (y == y2 + 1 || y == y2 - 1)) {
        return true;
    }
    if (y == y2 && (x == x2 + 1 || x == x2 - 1)) {
        return true;
    }
    return false;
}

void up() {
    yeni_y = y - 1;
    if (yeni_y >= 0) {
        if (!(yeni_y == y2 && x == x2)) {
            harita[y][x] = 0;
            if (y != yeni_y) {
                y = yeni_y;
            }
            harita[y][x] = 1;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void up2() {
    yeni_y2 = y2 - 1;
    if (yeni_y2 >= 0) {
        if (!(yeni_y2 == y && x2 == x)) {
            harita[y2][x2] = 0;
            if (y2 != yeni_y2) {
                y2 = yeni_y2;
            }
            harita[y2][x2] = 2;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void down() {
    yeni_y = y + 1;
    if (yeni_y < 6) {
        if (!(yeni_y == y2 && x == x2)) {
            harita[y][x] = 0;
            if (y != yeni_y) {
                y = yeni_y;
            }
            harita[y][x] = 1;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void down2() {
    yeni_y2 = y2 + 1;
    if (yeni_y2 < 6) {
        if (!(yeni_y2 == y && x2 == x)) {
            harita[y2][x2] = 0;
            if (y2 != yeni_y2) {
                y2 = yeni_y2;
            }
            harita[y2][x2] = 2;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void left() {
    yeni_x = x - 1;
    if (yeni_x >= 0) {
        if (!(y == y2 && yeni_x == x2)) {
            harita[y][x] = 0;
            if (x != yeni_x) {
                x = yeni_x;
            }
            harita[y][x] = 1;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void left2() {
    yeni_x2 = x2 - 1;
    if (yeni_x2 >= 0) {
        if (!(y2 == y && yeni_x2 == x)) {
            harita[y2][x2] = 0;
            if (x2 != yeni_x2) {
                x2 = yeni_x2;
            }
            harita[y2][x2] = 2;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void right() {
    yeni_x = x + 1;
    if (yeni_x < 7) {
        if (!(y == y2 && yeni_x == x2)) {
            harita[y][x] = 0;
            if (x != yeni_x) {
                x = yeni_x;
            }
            harita[y][x] = 1;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void right2() {
    yeni_x2 = x2 + 1;
    if (yeni_x2 < 7) {
        if (!(y2 == y && yeni_x2 == x)) {
            harita[y2][x2] = 0;
            if (x2 != yeni_x2) {
                x2 = yeni_x2;
            }
            harita[y2][x2] = 2;
        }
        else {
            cout << "Karakterler ayni konumda olamaz!" << endl;
        }
    }
}

void kontrol() {
    if (yon == 'U' || yon == 'u') {
        up();
    }
    if (yon == 'D' || yon == 'd') {
        down();
    }
    if (yon == 'L' || yon == 'l') {
        left();
    }
    if (yon == 'R' || yon == 'r') {
        right();
    }
    if (yon == 'F' || yon == 'f') {
        if (yanYana()) {
            fight2();
        }
        else {
            cout << "Savas yapilamaz, karakterler yan yana degil!" << endl;
        }
    }
}

void kontrol2() {
    if (yon2 == 'U' || yon2 == 'u') {
        up2();
    }
    if (yon2 == 'D' || yon2 == 'd') {
        down2();
    }
    if (yon2 == 'L' || yon2 == 'l') {
        left2();
    }
    if (yon2 == 'R' || yon2 == 'r') {
        right2();
    }
    if (yon2 == 'F' || yon2 == 'f') {
        if (yanYana()) {
            fight();
        }
        else {
            cout << "Savas yapilamaz, karakterler yan yana degil!" << endl;
        }
    }
}

void harita_yazdir() {
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 7; j++) {
            cout << harita[i][j] << " ";
        }
        cout << endl;
    }
    cout << "Karakter1 Can: " << can1 << "   Karakter2 Can: " << can2 << endl;
}

int main() {
    while (yon != 'E' && yon != 'e') {
        harita_yazdir();

        cout << "1. Karakter icin yon girin (U,D,L,R,F,E): ";
        cin >> yon;
        if (yon == 'E' || yon == 'e') {
            break;
        }

        cout << "2. Karakter icin yon girin (U,D,L,R,F,E): ";
        cin >> yon2;
        if (yon2 == 'E' || yon2 == 'e') {
            break;
        }

        kontrol();
        kontrol2();

        if (can1 <= 0 || can2 <= 0) {
            cout << "Oyun bitti!" << endl;
            break;
        }
    }

    cout << "Program bitti." << endl;
    sleep(1);
    cout << "Iyi Savastin Asker !" << endl;
    sleep(2);
    return 0;
}
