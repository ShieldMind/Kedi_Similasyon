#include <iostream>
#include <unistd.h>
#include <random>
#include <windows.h>

using namespace std ;

int Can = 100 ;
int Yas = 0;
int aclik = 100;
int y_aclik ;
int mama_sayisi = 0 ;
int Gun = 0;
int secim = -1;
int mutluluk = 100;
int Somon = 0;
int Sut = 0;
int havuc = 0;
int bilet = 10;

bool oyunsecim = 0 ;

void muzikCal() {
    // MP3 dosyasını aç (ANSI sürümünü kullanıyoruz)
    mciSendStringA("open \"Buraya Ses Dosyasının Uzantısını Ekleyin" type mpegvideo alias mymusic", NULL, 0, NULL);

    // Müziği çal
    mciSendStringA("play mymusic", NULL, 0, NULL);

    // Müziğin bitmesini bekle (örnek: 31.5 saniye)
    Sleep(31500);

    // Müzik dosyasını kapat
    mciSendStringA("close mymusic", NULL, 0, NULL);
}

void fight(){
    cout << "Arena'ya yonlendiriliyorsunuz (25-30 sn. surer)..." << endl ;
    muzikCal() ;
    cout << "Karsi rakip belirleniyor..." << endl;
    cout << "Oyuncu vs Oyuncu = 1 / AI vs Oyuncu = 0" << endl;
    cin >> oyunsecim ;
    if (oyunsecim == 1) {
        cout <<"Oyuncu vs Oyuncu moduna yonlendiriliyorsunuz..." << endl ;
        system("OvsO.exe");
    }
    if (oyunsecim == 0){
        cout <<"AI vs Oyuncu moduna yonlendiriliyorsunuz..." << endl ;
        system("AIvsO.exe");
    }
}
//Rastgele Sayi ureten kod
int random (int a) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(10, 25);
    return dist(gen);
}

void Ses () {
    if (bilet > 0) {
        cout << "Meow" << endl ;
        mutluluk += 10 ; 
        if (mutluluk > 100) {
            mutluluk = 100 ;
        }
        bilet-- ;
    }else {
        cout << "Yeterli Biletiniz Yok!" << endl ;
    }
}

//Random Stok Ureten Kod
int stok (int a) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist (1,2) ;
    return dist(gen);
}

int Somon_Stok;
int Sut_Stok;
int Havuc_Stok;

void magza () {
    if (bilet > 4) {
        cout << "Magzaya Hosgeldiniz" << endl ;
        cout << "Stoklarimiz" << endl ;
        sleep(1) ;
        Somon_Stok = stok(Somon_Stok);
        Sut_Stok = stok(Sut_Stok);
        Havuc_Stok = stok(Havuc_Stok);

        cout << Somon_Stok << " Kadar Somonumuz Var" << endl;
        cout << Sut_Stok << " Kadar Sutumuz Var" << endl;
        cout << Havuc_Stok << " Kadar Havucumuz Var" << endl;

        sleep(2) ;

        cout << "Hepsi Alindi" << endl ;
        sleep(1) ;
        cout << "Somon + " << Somon_Stok << endl ;
        Somon += Somon_Stok ;
        cout << "Sut + " << Sut_Stok << endl ;
        Sut += Sut_Stok ;
        cout << "Havuc + " << Havuc_Stok << endl ;
        havuc += Havuc_Stok ;
        bilet = bilet - 5 ;
    }else {
        cout << "Yeterli biletiniz yok!" << endl ;
    }

}

void mama_Yapma () {
    if (bilet > 2) {
        if (Somon > 0 && Sut > 0 && havuc > 0){
            cout << "Elinizdeki malzemelerle 'Kedi Mamasi' yapiliyor..." << endl ;

            sleep(2);

            mama_sayisi ++ ;
            cout << "Mama Yapildi +1 MAMA  Toplam mama: " << mama_sayisi << endl ;

            cout << "Somon - 1" << endl ;
            Somon -- ;
            cout << "Sut - 1" << endl ;
            Sut -- ;
            cout << "Havuc - 1" << endl ;
            havuc -- ;     
            bilet = bilet - 3 ;   
        }else {
            cout << "Yeterli malzeme yok! Magzadan alin." << endl ;
        }

    }else {
        cout << "Yeterli biletiniz yok!" << endl ;
    }

}

//Karnı dourma kodu
void aclik_artma () {
    if (bilet > 1) {
        if (mama_sayisi > 0) {
            int eski_aclik = aclik ;
            aclik += random(Can) ;
        

        if (aclik > 100) {
            aclik = 100 ;
        }
            mama_sayisi -- ;

            cout << "Eski aclik: " << eski_aclik << endl ;
            cout << "Yeni aclik: " << aclik << endl ;
        } else {
            cout << "Mama yok! Magzaya gidin" << endl ;
        }
        bilet = bilet - 2 ;
    }else {
        cout << "Yeterli biletiniz yok!" << endl ;
    }

}

void Gun_son () {
    aclik -= random(y_aclik) ;
    Gun ++ ;

    if (mutluluk > 0) {
        mutluluk = mutluluk - 5 ;
    }else if (mutluluk <= 0) {
        mutluluk = 0 ;
        Can -= 10 ;
        cout << "Kedi cok mutsuz oldu ve cani azaldi :( Yeni can : " << Can << endl ;
    }
    if (aclik <= 0) {
        aclik = 0 ;
        Can -= 10 ;
        cout << "Kedi cok acikti ve cani azaldi :( Yeni can : " << Can << endl ;
    }
    if (aclik > 80) {
        Can += 15 ;
        if (Can > 100) {
            Can = 100 ;
        }
    }else if (mutluluk > 80) {
        Can += 10 ;
        if (Can > 100) {
            Can = 100 ;
        }
    }
    secim = -1 ;
    bilet += 10 ;
}

void Zaman_Gec () {
    cout << "Gun bitti!" << endl ;
    Gun_son() ;
    bilet = bilet - 1 ;
}

void secme () {
    cout << Gun << ". Gundesin" << endl ;
    cout << "Biletin: " << bilet << endl ;
    cout << "Can: " << Can << "  Aclik: " << aclik << "  Mutluluk: " << mutluluk << "  Mama: " << mama_sayisi << endl ;
    cout << "Somon: " << Somon << "  Sut: " << Sut << "  Havuc: " << havuc << endl ;
    cout << "Kediyi Sevmek icin 1" << " Bir bilet gider" << endl ;
    cout << "Magzayi Ziyaret Etmek icin 2" << " Bes bilet gider" << endl ;
    cout << "Kedi Mamasi Yapmak icin 3" << " Uc bilet gider" << endl ;
    cout << "Kediyi Beslemek icin 4" << " Iki bilet gider" << endl ;
    cout << "Zaman Gecirmek icin 5" << " Bir bilet gider" << endl ;
    cout << "Arena'ya gitmek icin 6" << " Iki bilet gider" << endl ;
    cin >> secim ;
    cout << "Seciminiz: " << secim << endl ;
}

void secim_secme() {
    if (secim == 5) {
        Zaman_Gec();
    } else if (secim == 4) {
        aclik_artma();
    } else if (secim == 3) {
        mama_Yapma();
    } else if (secim == 2) {
        magza();
    } else if (secim == 1) {
        Ses();
    }else if (secim == 6)
    {
        fight();
    } else {
        cout << "Lutfen gecerli bir sayi girin" << endl ;
        secme();
    }
}

int main(){
    cout << "Kedi simulasyonuna hosgeldiniz!" << endl ;
    while (Can > 0) {
        secme() ;
        secim_secme() ;
        sleep(1) ;
    }
    cout << "Kediniz Allah'in rahmetine kavustu. Oyun bitti..." << endl ;

    cin.get();
    cin.ignore();
    return 0 ;

}
