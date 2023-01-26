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

// function makes a HTTP request to an API server using the cURL library.
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

// callback function used by the cURL library to handle the response from the API server.
// It appends the response to a string passed as a user pointer.
size_t handleResponse(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// uses the cURL library to fetch data from a specified URL and writes the response to a string passed as a reference.
// It also sets options for the cURL request such as the URL, write function, and use of SSL. It returns a boolean
// indicating whether the request was successful or not.
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

//  uses the JsonCPP library to parse the response from the API server, which is in JSON format,
// into a Json::Value object. It returns a boolean indicating whether the parsing was successful or not.
bool parseJson(std::string& readBuffer, Json::Value& root) {
    Json::Reader reader;

    if (!reader.parse(readBuffer, root)) {
        std::cerr << "Failed to parse JSON" << std::endl;
        return false;
    }
    return true;
}

// writes the data from the Json::Value object to a CSV file. It writes the data in a specified format,
// with each field separated by a comma and each record on a new line. It returns a boolean indicating 
// whether the file was successfully written or not.
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

// calls the above functions in order, first fetching data from the API server, then parsing the JSON response,
// and finally writing the data to a CSV file. If all steps are successful, it prints a message indicating success, otherwise, it prints an error message.
void processData() {
    std::string readBuffer;
    Json::Value root;

    if (fetchDataFromURL(readBuffer) && parseJson(readBuffer, root) && writeToFile(root)) {
        std::cout << "Data successfully written to students.csv" << std::endl << std::endl;
    } else {
        std::cerr << "Error: unable to process data" << std::endl;
    }
}

void export_to_csv() {
    processData();
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

// function is used to clear the terminal screen.
void clearTerminal(string os) {
    if (os == "Linux") {
        system("clear");
    } else if (os == "Windows"){
        system("cls");
    }
}
// function that prompts the user for input on how many students they want to calculate the SPP.
int input_jumlah_mahasiswa() {

    // variable to store number of students input
    string n_mahasiswa;

    // loop until a valid input is given
    while (true)
   {
    cout << "Mau menghitung berapa mahasiswa nih ? ";
    cin>>n_mahasiswa;

    // check if input is a valid number
    if (!isInteger(n_mahasiswa)) {
        clearTerminal(OS);
        cout << "!!! Masukan Angka bukan Huruf !!!" << endl;
        continue;
    }

     // clear the terminal
    clearTerminal(OS);

    // return the input as integer
    return stoi(n_mahasiswa);
   }
}


// This code defines a function called "HitungSPP" which calculates the tuition fees (SPP) for students at a university. It first sets up two constant variables,
// "JUMLAH_PRODI" and "NUM_STUDENTS", with the number of programs and students respectively. It then initializes an array of structs for the different program types
// and an array of structs for the students. The program then displays an introduction message and prompts the user to input student data. It uses a for loop
// to iterate through each student, asking for their name, department code (which is checked for validity), and number of credit hours (SKS). The program then
// calculates the total SPP for each student using the "hitungSpp" function from the struct for their specific program type and stores it in their student record.
// The code also includes a function called "isInteger" to check if user input is an integer, and a function "clearTerminal" to clear the terminal.

void HitungSPP() {
    // initialize constant variables for the number of programs and students
    const int JUMLAH_PRODI = 3;
    const int NUM_STUDENTS = input_jumlah_mahasiswa();

     // initialize an array of structs for the different program types
    spp_type list_spp_type[JUMLAH_PRODI] = {
        {"Kedokteran", 2000, 1500},
        {"Olahraga", 500, 150},
        {"System Design", 6500, 2050},
    };
    // initialize an array of structs for the students
    mhs list_mhs[NUM_STUDENTS];
    // initialize a string variable for user input
    string pilihan;

    // display introduction message
    cout << "######################################################" << endl<< endl;
    cout << "     Yuk Cari Tahu Biaya SPP Mahasiswa di Amikom" << endl;
    cout << "     Input Data dibawah ya untuk menghitungnya" <<endl<< endl;
    cout << "######################################################" << endl<<endl;

    // loop through each student
    for (int i = 0; i < NUM_STUDENTS; i++) {
        cout << "------------------------------------------------------" << endl;
        // get student name
        cout << "Nama \t: ";
        cin.ignore();
        getline(cin, list_mhs[i].nama);

        // This code is a loop that allows the user to input a valid department code. The loop will continue until the user inputs a valid number.
        // The program first displays a list of all available department codes along with their names, and then prompts the user to input a code.
        // The input is then checked to see if it is an integer using the isInteger() function. If the input is not an integer, the user is
        // prompted to input a valid number. If the input is an integer, the program checks if the input is a valid code by checking if it is within
        // the range of available codes. If the code is valid, the loop is broken and the selected department code is assigned to the student.
        // If the code is not valid, the user is prompted to input a valid code again.
        while (true)
        {
            cout << "------------------------------------------------------" << endl;
            cout << "Silahkan masukan kode jurusannya:" << endl << endl;
            for (int j = 0; j < JUMLAH_PRODI; j++) {
                cout << j+1 << " = " << list_spp_type[j].prodi_name << endl;
            }
        
            cout << endl << "Prodi \t: ";
            cin >> pilihan;

            // check if input is an integer
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

        // This section of code is asking the user to input the number of credit hours (SKS) for a student. It uses a while loop to continue asking
        // the user until a valid input is entered. The input is stored in the variable "pilihan". The code uses the "isInteger" function to check
        // if the input is a valid integer. If the input is not an integer, the code will display an error message and the loop will continue.
        // If the input is a valid integer, it will be stored in the "jumlah_sks" field of the student's record and the loop will break, moving
        // on to the next step in the program.
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
    
    // These lines of code are displaying the final data of students after processing it. The code is displaying the name,
    // program, number of credits and total tuition fee of each student. The code is also calling a function "parsing_json"
    // which is converting the data into json format. Then it is calling a function "makeCurlRequest" which is posting the data to the server. 
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


// This function handles the user's choice from the menu
void HandleMenuChoice(int choice) {
    if (choice == 1) {
        // function to calculate student's SPP
        HitungSPP();
    } else if (choice == 2) {
        // function to export data to csv
        export_to_csv();
    } else if (choice == 3) {
        // ignore user input
        cin.ignore();
    }
}


//  displays the menu options to the user, asks for the user's choice, and returns the choice as an integer.
// If the user enters an invalid choice, the function will prompt the user to enter a valid choice again.
int HandleMenu() {

    string pilihan;
    menu:
    cout<<"========================="<<endl;
    cout<<"1.Hitung SPP"<<endl;
    cout<<"2.Export To CSV"<<endl;
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

    // repeatedly calls the HandleMenu and HandleMenuChoice functions until the user chooses to exit the program (choice 3).
    // The selected choice from HandleMenu is passed as an argument to HandleMenuChoice to determine which action to take.
    int menuChoice;
    while(1){
       menuChoice = HandleMenu();
       if(menuChoice == 3) break;
       HandleMenuChoice(menuChoice);
    }    

    return 0;
}