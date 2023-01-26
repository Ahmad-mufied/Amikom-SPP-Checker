#include <iostream>
#include <string>
#include <curl/curl.h>
#include <typeinfo>
#include <sstream>
#include <fstream>
#include <json/json.h>
#include "os_detection.h"

using namespace std;

string json_final;
std::string readBuffer;

// Declare URLs as global variables
const string addDataURL = "https://spp-bill.fly.dev/add_data";
const string allMhsURL = "https://spp-bill.fly.dev/all_mhs";

struct spp_type
{   
    string prodi_name;
    int spp_tetap;
    int spp_variable;

    int hitungSpp(int j_sks) {
        return spp_tetap + spp_variable * j_sks;
    }
};


struct mhs {
    string nama;
    int prodi_code, jumlah_sks;
    int total_spp;
};

string parsing_json(string name, string prodi, int jumlah_sks, int total_spp) {
    stringstream ss;
    ss << total_spp;
    string total_spp_string = ss.str();
    return "{\"Nama\":\"" + name + "\",\"Jurusan\":\"" + prodi + "\",\"SKS\": \"" + to_string(jumlah_sks) + "\",\"Biaya\":\"" + to_string(total_spp) + "\"}";
}

size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up)
{
    // This function is a callback function that is used by curl to write the response from the server
    // It takes in the response data as a char buffer and prints it out to the console
    for (int c = 0; c<size*nmemb; c++)
    {
        std::cout << buf[c];
    }
    return size*nmemb;
}

void initializeCurl(CURL*& curl) {
    // This function initializes the curl library and creates a new easy handle
    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
}

// string struct_to_string(struct mhs) {

// }
void setCurlOptions(CURL* curl) {
    // This function sets the options for the curl request
    // It sets the URL, the data to be sent in the request, the write function callback and the headers
    curl_easy_setopt(curl, CURLOPT_URL, addDataURL.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_final.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback);
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
}

void performCurlRequest(CURL* curl) {
    // This function performs the curl request and checks for any errors
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        std::cout << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
    }
}

void cleanupCurl(CURL* curl) {
    // This function cleans up after the curl request by freeing the easy handle and cleaning up the global curl environment
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void makeCurlRequest() {
    CURL* curl;
    initializeCurl(curl);
    if (curl) {
        setCurlOptions(curl);
        performCurlRequest(curl);
        cleanupCurl(curl);
    }
}

// These for retrieving from api server

size_t handleResponse(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

bool fetchDataFromURL(std::string& readBuffer) {
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, allMhsURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &handleResponse);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res != CURLE_OK) {
            std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
            return false;
        }
    }
    return true;
}

bool parseJson(std::string& readBuffer, Json::Value& root) {
    Json::Reader reader;

    if (!reader.parse(readBuffer, root)) {
        std::cerr << "Failed to parse JSON" << std::endl;
        return false;
    }
    return true;
}

bool writeToFile(Json::Value& root) {
    std::ofstream csvFile("students.csv");
    csvFile << "id,name,department,sks,fee" << std::endl;

    for (int i = 0; i < root.size(); i++) {
        csvFile << root[i]["id"].asInt() << ",";
        csvFile << root[i]["name"].asString() << ",";
        csvFile << root[i]["department"].asString() << ",";
        csvFile << root[i]["sks"].asInt() << ",";
        csvFile << root[i]["fee"].asInt() << std::endl;
    }

    csvFile.close();
    return true;
}

void processData() {
    std::string readBuffer;
    Json::Value root;

    if (fetchDataFromURL(readBuffer) && parseJson(readBuffer, root) && writeToFile(root)) {
        std::cout << "Data successfully written to students.csv" << std::endl << std::endl;
    } else {
        std::cerr << "Error: unable to process data" << std::endl;
    }
}


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

int input_jumlah_mahasiswa() {
    string n_mahasiswa;
    while (true)
   {
    cout << "Mau menghitung berapa mahasiswa nih ? ";
    cin>>n_mahasiswa;

    if (!isInteger(n_mahasiswa)) {
        clearTerminal(OS);
        cout << "!!! Masukan Angka bukan Huruf !!!" << endl;
        continue;
    }

    clearTerminal(OS);

    return stoi(n_mahasiswa);
   }
}

void HitungSPP() {
    const int JUMLAH_PRODI = 3;
    const int NUM_STUDENTS = input_jumlah_mahasiswa();
    spp_type list_spp_type[JUMLAH_PRODI] = {
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
            for (int j = 0; j < JUMLAH_PRODI; j++) {
                cout << j+1 << " = " << list_spp_type[j].prodi_name << endl;
            }
        
            cout << endl << "Prodi \t: ";
            cin >> pilihan;

            if (!isInteger(pilihan)) {
                cout << "!!! Masukan Angka bukan Huruf !!!" << endl;
                continue;
            }

            int code = stoi(pilihan) - 1;
            
            if (code >= 0 && code < JUMLAH_PRODI) {
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
        string name = list_mhs[i].nama;
        string prodi = list_spp_type[list_mhs[i].prodi_code].prodi_name;
        int jumlah_sks = list_mhs[i].jumlah_sks;
        float total_spp = list_mhs[i].total_spp;

        json_final = parsing_json(name, prodi, jumlah_sks, total_spp);

        cout << "Nama \t\t: " << name << endl;
        cout << "Prodi \t\t: " << prodi << endl;
        cout << "SKS \t\t: " << jumlah_sks << endl;
        cout << "Total SPP \t: " << total_spp << endl;
        cout << total_spp << endl; 
        cout << "=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=" << endl;

         // HTTP POST to server
        makeCurlRequest();

        cout << "-------------------------------------------" << endl;
    }
}

void export_to_csv() {
    processData();
}

void HandleMenuChoice(int choice) {
    if (choice == 1) {
        HitungSPP();
    } else if (choice == 2) {
        export_to_csv();
    } else if (choice == 3) {
        cin.ignore();
    }
}

int HandleMenu() {
    
    
    string pilihan;
    menu:
    cout<<"========================="<<endl;
    cout<<"1.Hitung SPP"<<endl;
    cout<<"2.Export"<<endl;
    cout<<"3.Exit/Keluar"<<endl;
    cout<<"========================="<<endl;
    cout<<"Masukan Angka Pilihan Anda = ";
    cin>>pilihan;
    system("clear");

    if (pilihan == "1") {
        return 1;
    } else if (pilihan == "2") {
        return 2;
    } else if (pilihan == "3") {
        return 3;
    } else {
        cin.ignore();
        cout<<"Pilihan salah, tekan enter untuk melanjutkan";
        cin.get();
        system("clear");
        goto menu;
    }
}


int main() {

    int menuChoice;
    while(1){
       menuChoice = HandleMenu();
       if(menuChoice == 3) break;
       HandleMenuChoice(menuChoice);
    }    

    return 0;
}