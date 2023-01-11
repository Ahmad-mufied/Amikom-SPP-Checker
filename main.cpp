#include <iostream>
#include <string>
#include "os_detection.h"

using namespace std;

struct spp_type
{   
    string prodi_name;
    int spp_tetap;
    float spp_variable;

    float hitungSpp(int j_sks) {
        return spp_tetap + spp_variable * j_sks;
    }
};

struct mhs {
    string nama;
    int prodi_code, jumlah_sks;
    float total_spp;
};

bool isInteger(std::string str) { 
    // Check if string contains only digits 
    for (int i = 0; i < str.length(); i++) { 
        if (isdigit(str[i]) == false) 
            return false; 
    } 

    // If we reach here, all characters are digits 
    return true; 
}

void clearTerminal(string os) {
    if (os == "Linux") {
        system("clear");
    } else if (os == "Windows"){
        system("cls");
    }
}


int main() {
    const int NUM_PROGRAMS = 3;
    const int NUM_STUDENTS = 2;
    spp_type list_spp_type[NUM_PROGRAMS] = {
        {"Kedokteran", 2000, 1500},
        {"Olahraga", 500, 150},
        {"System Design", 6500, 2050},
    };
    mhs list_mhs[NUM_STUDENTS];
    string pilihan;
    
    cout << "######################################################" << endl<< endl;
    cout << "     Yuk Cari Tahu Biaya SPP Mahasiswa di Amikom" << endl;
    cout << "     Input Data dibawah ya untuk menghitungnya" <<endl<< endl;
    cout << "######################################################" << endl<<endl;
    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        cout << "------------------------------------------------------" << endl;
        cout << "Nama \t: ";
        cin.ignore();
        getline(cin, list_mhs[i].nama);

        while (true)
        {
            cout << "------------------------------------------------------" << endl;
            cout << "Silahkan masukan kode jurusannya:" << endl << endl;
            for (int j = 0; j < NUM_PROGRAMS; j++) {
                cout << j+1 << " = " << list_spp_type[j].prodi_name << endl;
            }
        
            cout << endl << "Prodi \t: ";
            cin >> pilihan;

            if (!isInteger(pilihan)) {
                cout << "!!! Masukan Angka bukan Huruf !!!" << endl;
                continue;
            }

            int code = stoi(pilihan) - 1;
            
            if (code >= 0 && code < NUM_PROGRAMS) {
              list_mhs[i].prodi_code = code;
              break;

            } else {
                cout << "!!! Masukan Kode yang ada diatas dong !!!" << endl;
            }
        }
         while (true)
        {
            cout << "------------------------------------------------------" << endl;
            cout << "Jumlah SKS: \t: ";
            cin >> pilihan;

            if (!isInteger(pilihan)) {
                cout << "!!! Masukan Angka bukan Huruf !!!" << endl;
                continue;
            } 

            list_mhs[i].jumlah_sks = stoi(pilihan);
            cout << "------------------------------------------------------" << endl;
            break;
        }

        list_mhs[i].total_spp = list_spp_type[list_mhs[i].prodi_code].hitungSpp(list_mhs[i].jumlah_sks);
        clearTerminal(OS);
    }

    cout << "\n";
    cout << "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=" << endl;
    for (int i = 0; i < NUM_STUDENTS; i++) {
        cout << "Nama \t\t: " << list_mhs[i].nama << endl;
        cout << "Prodi \t\t: " << list_spp_type[list_mhs[i].prodi_code].prodi_name << endl;
        cout << "SKS \t\t: " << list_mhs[i].jumlah_sks << endl;
        cout << "Total SPP \t: " << list_mhs[i].total_spp << endl;
        cout << "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=" << endl;
    }

    return 0;
}